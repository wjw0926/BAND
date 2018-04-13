//
//  client.c
//  band
//
//  Created by Jaewook Woo on 16/03/2018.
//  Copyright © 2018 Jaewook Woo. All rights reserved.
//
//  Reference: 2014 Spring KAIST CS230 System Programming Project Assignment 3 FTP
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORTNUM 3800
#define MAXLINE 1024

#define Q_LIST      1
#define Q_UPLOAD    2
#define Q_DOWNLOAD  3

int get_list(int sockfd);
int upload(int sockfd, char *file);
int download(int sockfd, char *file);

void help(char *progname)
{
    printf("Usage : %s -h -i [ip] -u [upload filename] -d [download filename] -l\n", progname);
}

int main(int argc, char * argv[]) {
    struct sockaddr_in addr={0};
    int sockfd;
    socklen_t servlen;
    
    int command_type=0;
    int opt;
    int optflag=0;
    char ipaddr[36]={0x00,};
    char fname[64]={0x00,};
    
    //process command line argument
    while((opt = getopt(argc, argv, "hli:u:d:")) != -1) {
        switch(opt) {
            case 'h':
                help(argv[0]);
                return 1;
                
            case 'i':
                sprintf(ipaddr, "%s", optarg);
                break;
                
            case 'u':
                command_type = Q_UPLOAD;
                sprintf(fname, "%s", optarg);
                optflag = 1;
                break;
                
            case 'd':
                command_type = Q_DOWNLOAD;
                sprintf(fname, "%s", optarg);
                optflag = 1;
                break;
                
            case 'l':
                command_type = Q_LIST;
                break;
                
            default:
                help(argv[0]);
                return 1;
        }
    }
    
    if(ipaddr[0] == '\0') {
        printf ("ip address not setting\n");
        return 0;
    }
    
    if((fname[0] == '\0') && (optflag == 1)) {
        printf ("fname error\n");
        return 0;
    }

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
         printf("Socket error\n");
         return 1;
    }
    
    addr.sin_family     = AF_INET;
    addr.sin_port       = htons(PORTNUM);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    //Connect
    servlen = sizeof(addr);
    if(connect(sockfd, (struct sockaddr *)&addr, servlen) == -1) {
        printf("Connect error\n");
        return 0;
    }
    
    while(1)
    {
        switch(command_type)
        {
            case (Q_LIST):
                get_list(sockfd);
                break;

            case (Q_DOWNLOAD):
                download(sockfd, fname);
                break;
                
            case (Q_UPLOAD):
                upload(sockfd, fname);
                break;
                
            default:
                printf("Unknown command\n");
                break;
        }
        break;
    }
    close(sockfd);
}

int get_list(int sockfd)
{
    char buf[MAXLINE];
    int command=Q_LIST;
    char filename[64]="no file";
 
    printf("This is the list function\n");
 
    //Send a command  (LIST)
    sprintf(buf, "%d", command);
    if(send(sockfd, buf, MAXLINE, 0) == -1) {
        return -1;
    }
    //Send a filename
    sprintf(buf, "%s", filename);
    if(send(sockfd, buf, MAXLINE, 0) == -1) {
        return -1;
    }
    //Receive a file name from a server and printf it to the stdout
    while(recv(sockfd, buf, MAXLINE, 0) > 0)
        printf("%s\n", buf);
 
    printf("End List!\n");
    return 1;
}

int upload(int sockfd, char *file)
{
    int sendn;
    char buf[MAXLINE];
    int command=Q_UPLOAD;
 
    printf("This is the upload function\n");
 
    //Send a command
    sprintf(buf, "%d", command);
    if(send(sockfd, buf, MAXLINE, 0) == -1) {
        return -1;
    }
    //Send a filename
    sprintf(buf, "%s", file);
    if(send(sockfd, buf, MAXLINE, 0) == -1) {
        return -1;
    }
    //Open a file to read (Upload)
    FILE *f = fopen(file, "rb");
    if(f == NULL)
    {
        printf("ERROR: File %s not found.\n", file);
        fclose(f);
        close(sockfd);
        return -1;
    }
    //Send a file content to a server
    fseek(f, 0, SEEK_END);
    int filesize = ftell(f);
    fseek(f, 0, SEEK_SET);
    printf("Uploading file size is %d\n", filesize);
    
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
        int bytestosend = filesize;
        while((sendn = send(sockfd, buffer+offset, MAXLINE, 0)) > 0 && bytestosend > 0)
        {
            offset += sendn;
            bytestosend -= sendn;
        }
    } else {
        fclose(f);
        free(buffer);
        printf("ERROR: Failed to read file %s\n", file);
        close(sockfd);
        return 0;
    }

    fclose(f);
    free(buffer);
    close(sockfd);

    printf("End Upload!\n");
    return 1;
}

int download(int sockfd, char *file)
{
    int readn, writen;
    char buf[MAXLINE];
    int command=Q_DOWNLOAD;
    int receive = 0;
 
    printf("This is the download function\n");
 
    //Send a command
    sprintf(buf, "%d", command);
    if(send(sockfd, buf, MAXLINE, 0) == -1) {
        return -1;
    }
    //Send a filename
    sprintf(buf, "%s", file);
    if(send(sockfd, buf, MAXLINE, 0) == -1) {
        return -1;
    }
    //Open a file to write (Download)
    FILE *f = fopen(file, "wb");
    
    //Receive a file content from a server and write it to the file
    printf("Receiving file %s...\n", file);
    memset(buf, 0, MAXLINE);
    while((readn = recv(sockfd, buf, MAXLINE, 0)) > 0)
    {
        receive = 1;
        if(fwrite(buf, readn, 1, f) < 1)
            printf("ERROR: Failed to write file %s\n", file);
        memset(buf, 0, MAXLINE);
    }
    fclose(f);

    if(receive) {
        printf("Receiving file %s is done!\n", file);
    } else {
        printf("ERROR: Failed to receive file %s\n", file);
    }
    close(sockfd);

    printf("End Download!\n");
    return 1;
}
