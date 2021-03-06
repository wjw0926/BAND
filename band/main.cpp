//
//  main.cpp
//  band
//
//  Created by Jaewook Woo on 13/02/2018.
//  Copyright © 2018 Jaewook Woo. All rights reserved.
//
//  Reference: 2014 Spring KAIST CS230 System Programming Project Assignment 3 FTP
//

#include <iostream>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#include "ensemble.hpp"

#define PORTNUM 3800
#define MAXLINE 1024

#define Q_LIST      1
#define Q_UPLOAD    2
#define Q_DOWNLOAD  3
#define Q_MERGE     4

int processRequest(int sockfd);
int serv_file_list(int sockfd);
int serv_file_upload(int sockfd, char *filename);
int serv_file_download(int sockfd, char *filename);
int serv_file_merge(int sockfd, char *filename);

struct Cquery
{
    int command;
    char f_name[64];
};

int main(int argc, const char * argv[]) {
    struct sockaddr_in addr = {0};
    struct sockaddr cli_addr = {0};
    int sockfd, cli_sockfd;
    socklen_t clilen = sizeof(cli_addr);
    int pid;
    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        return 1;
    }
    
    printf("Server Start\n");
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORTNUM);
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //inet_addr("127.0.0.1")
    
    //Bind
    if(bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        printf("bind error\n");
        printf("Maybe you can change your port number\n");
        return 1;
    }
    
    //Listen
    if(listen(sockfd, 5) == -1) {
       printf("listen error\n");
       return 1;
    }
    
    while(1) {
       //Accept
       cli_sockfd = accept(sockfd, &cli_addr, &clilen);
       if(cli_sockfd < 0) exit(0);
       pid = fork();
       
       if(pid == 0) { //Child
           processRequest(cli_sockfd);
           close(cli_sockfd);
       }
       else{ //Parent
           close(cli_sockfd);
       }
    }
    return 0;
}

int processRequest(int sockfd) {
    char buff[MAXLINE];
    struct Cquery query;
    int ret = 1;

    query.command = 0;

    printf("OK Server\n");
 
    while(1)
    {
        //Receive command: LIST, UPLOAD, DOWNLOAD
        if(recv(sockfd, buff, MAXLINE, 0) == -1) {
               return -1;
        }
        query.command = atoi(buff);
 
       //Receive filename
        if(recv(sockfd, buff, MAXLINE, 0) == -1) {
            return -1;
        }
        strncpy(query.f_name, buff, 64);
 
       printf("Received Query: %d, %s\n", query.command, query.f_name);
 
       switch(query.command)
       {
           case Q_LIST:
               serv_file_list(sockfd);
               break;
           case Q_UPLOAD:
               serv_file_upload(sockfd, query.f_name);
               break;
           case Q_DOWNLOAD:
               serv_file_download(sockfd, query.f_name);
               break;
           case Q_MERGE:
               serv_file_merge(sockfd, query.f_name);
               break;
           default:
               ret = -1;
               break;
       }
       break;
    }
    if (ret == -1) {
        printf("Client request failed!\n");
        exit(0);
    }
    return ret;
}

int serv_file_list(int sockfd) {
    DIR *dirp;
    struct dirent *dentry;
    char buf[MAXLINE];
 
    //Read each entry in directory and send the filename to a client
    if((dirp = opendir("video")) != NULL) {
        memset(buf, 0, MAXLINE);
        while((dentry = readdir(dirp)) != NULL) {
            strncpy(buf, dentry->d_name, MAXLINE);
            if(send(sockfd, buf, MAXLINE, 0) == -1)
                printf("Send file list failed\n");
            memset(buf, 0, MAXLINE);
        }
        closedir(dirp);
    } else {
        printf("Can't open directory\n");
        closedir(dirp);
    }
    return 1;
}

int serv_file_upload(int sockfd, char *filename) {
    size_t readn;
    char buf[MAXLINE];
    bool receive = false;
    
    //Tokenizing the filename
    char *token;
    char *rest = filename;
    char ret[64];
    while ((token = strtok_r(rest, "/", &rest)))
        strncpy(ret, token, 64);

    //Upload a file from a client
    char path[64] = {0x00,};
    sprintf(path, "video/%s", ret);
    FILE *f = fopen(path, "wb");

    printf("Receiving file %s...\n", ret);
    memset(buf, 0, MAXLINE);
    while((readn = recv(sockfd, buf, MAXLINE, 0)) > 0)
    {
        receive = true;
        if(fwrite(buf, readn, 1, f) < 1)
            printf("ERROR: Failed to write file %s\n", filename);
        memset(buf, 0, MAXLINE);
    }
    fclose(f);

    if(receive) {
        printf("Receiving file %s is done!\n", ret);
    } else {
        printf("ERROR: Failed to receive file %s\n", ret);
    }
    close(sockfd);

    return 1;
}

int serv_file_download(int sockfd, char *filename) {
    size_t sendn;
    
    //Tokenizing the filename
    char *token;
    char *rest = filename;
    char ret[64];
    while ((token = strtok_r(rest, "/", &rest)))
        strncpy(ret, token, 64);
    
    //Open a file to read
    char path[64] = {0x00,};
    sprintf(path, "video/%s", ret);
    FILE *f = fopen(path, "rb");
    if(f == NULL)
    {
        printf("ERROR: File %s not found.\n", ret);
        fclose(f);
        close(sockfd);
        return -1;
    }
    
    //Send a file content to a client
    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    fseek(f, 0, SEEK_SET);
    printf("Downloading file size is %ld\n", filesize);
    
    char *buffer = (char *)malloc(filesize + 1);
    if (!buffer)
    {
        printf("ERROR: Failed to allocate memory\n");
        fclose(f);
        free(buffer);
        close(sockfd);
        
        return 0;
    }
    
    memset(buffer, 0, filesize+1);
    if(fread(buffer, filesize, 1, f) > 0)
    {
        int offset = 0;
        long bytestosend = filesize;
        while((sendn = send(sockfd, buffer+offset, MAXLINE, 0)) > 0 && bytestosend > 0)
        {
            offset += sendn;
            bytestosend -= sendn;
        }
    } else {
        fclose(f);
        free(buffer);
        printf("ERROR: Failed to read file %s\n", ret);
        close(sockfd);
        return 0;
    }
    
    fclose(f);
    free(buffer);
    printf("Sending file %s is done!\n", ret);
    close(sockfd);
 
    return 1;
}

int serv_file_merge(int sockfd, char *filename) {
    size_t sendn;

    //Tokenizing the filename
    char *token;
    char *rest = filename;
    token = strtok_r(rest, "-", &rest);

    //Merge two videos
    std::string first(token);
    std::string second(rest);
    printf("Merging two files %s, %s...\n", token, rest);
    mergeVideo(std::string(token), std::string(rest));

    //Open a merged file to read
    char path[64] = {0x00,};
    sprintf(path, "video/final.avi");
    FILE *f = fopen(path, "rb");
    if(f == NULL)
    {
        printf("ERROR: File final.avi not found.\n");
        fclose(f);
        close(sockfd);
        return -1;
    }

    //Send a merged file content to a client
    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    fseek(f, 0, SEEK_SET);
    printf("Merged file size is %ld\n", filesize);

    char *buffer = (char *)malloc(filesize + 1);
    if (!buffer)
    {
        printf("ERROR: Failed to allocate memory\n");
        fclose(f);
        free(buffer);
        close(sockfd);

        return 0;
    }

    memset(buffer, 0, filesize+1);
    if(fread(buffer, filesize, 1, f) > 0)
    {
        int offset = 0;
        long bytestosend = filesize;
        while((sendn = send(sockfd, buffer+offset, MAXLINE, 0)) > 0 && bytestosend > 0)
        {
            offset += sendn;
            bytestosend -= sendn;
        }
    } else {
        fclose(f);
        free(buffer);
        printf("ERROR: Failed to read file final.avi\n");
        close(sockfd);
        return 0;
    }

    fclose(f);
    free(buffer);
    printf("Sending merged file final.avi is done!\n");
    close(sockfd);

    return 1;
}
