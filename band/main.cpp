//
//  main.cpp
//  band
//
//  Created by Jaewook Woo on 13/02/2018.
//  Copyright © 2018 Jaewook Woo. All rights reserved.
//
//  Reference: http://dyndy.tistory.com/256 [DY N DY]
//

#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include <opencv/cv.hpp>

using namespace std;
using namespace cv;

int main(int argc, const char * argv[]) {
    const string source = "/Users/jaewook/band/sample_video/sample.avi";

    vector<string> videoPath;
    videoPath.push_back(source);
    videoPath.push_back(source);
    videoPath.push_back(source);
    videoPath.push_back(source);
    videoPath.push_back(source);
    videoPath.push_back(source);
    
    int H = (int)(sqrt(videoPath.size()));
    int W = ceil((videoPath.size()) / (double)H);

    vector<VideoCapture> inputVideo;

    for (int i = 0; i < videoPath.size(); i++) {
        inputVideo.push_back(VideoCapture(videoPath[i]));
        
        if (!inputVideo[i].isOpened()) {
            cout << "fail to read the video" << endl;
            return EXIT_FAILURE;
        }
    }
    
    // 모든 비디오의 width와 height가 같다고 가정
    double fps = inputVideo[0].get(CV_CAP_PROP_FPS);
    int width = (int)inputVideo[0].get(CV_CAP_PROP_FRAME_WIDTH);
    int height = (int)inputVideo[0].get(CV_CAP_PROP_FRAME_HEIGHT);
    int W_size = width * W;
    int H_size = height * H;
    int loop = 1;
    VideoWriter outputVideo("result.avi", CV_FOURCC('D', 'I', 'V', 'X'), fps, Size(W_size, H_size));
    
    while (loop) {
        Mat image = Mat(Size(W_size, H_size), CV_8UC3);
        
        int i = 0;
        for (int c = 0; c < H; ++c) {
            for (int r = 0; r < W; ++r) {
                Mat frame;
                if (i > videoPath.size() - 1)
                    continue;
                
                inputVideo[i] >> frame;
                i++;
                
                if (frame.empty()) {
                    loop = 0;
                    break;
                }
                frame.copyTo(image(Rect((r*width), (c*height), width, height)));
            }
        }
        if (!image.empty()) {
            outputVideo.write(image);
            imshow("image", image);
            waitKey(1);
        }
    }
    
    outputVideo.release();
    return 0;
}
