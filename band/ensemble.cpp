// This is test 
//  ensemble.cpp
//  band
//
//  Created by Jaewook Woo on 18/02/2018.
//  Copyright © 2018 Jaewook Woo. All rights reserved.
//
//  Reference: http://dyndy.tistory.com/256 [DY N DY]
//

#include "ensemble.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include <opencv/cv.hpp>
#include <stdlib.h>
#include <Windows.h>

using namespace std;
using namespace cv;

int make_output() {
	const string source1 = "sample1.avi";
	const string source2 = "sample2.avi";

	vector<string> videoPath;
	videoPath.push_back(source1);
	videoPath.push_back(source2);

	vector<VideoCapture> inputVideo;

	for (int i = 0; i < videoPath.size(); i++) {
		inputVideo.push_back(VideoCapture(videoPath[i]));

		if (!inputVideo[i].isOpened()) {
			cout << "fail to read the video" << endl;
			return EXIT_FAILURE;
		}
	}

	double fps = inputVideo[0].get(CV_CAP_PROP_FPS);
	int width1 = (int)inputVideo[0].get(CV_CAP_PROP_FRAME_WIDTH);
	int height1 = (int)inputVideo[0].get(CV_CAP_PROP_FRAME_HEIGHT);
	int width2 = (int)inputVideo[1].get(CV_CAP_PROP_FRAME_WIDTH);
	int height2 = (int)inputVideo[1].get(CV_CAP_PROP_FRAME_HEIGHT);
	int width = 0;
	if (width1 > width2) width = width1; else width = width2;
	int W_size = width;
	int H_size = height1 + height2;
	int loop = 1;
	VideoWriter outputVideo("result.avi", CV_FOURCC('D', 'I', 'V', 'X'), fps / 2, Size(W_size, H_size));
	int set1, set2;
	set1 = set2 = 1;
	while (loop) {
		Mat image = Mat(Size(W_size, H_size), CV_8UC3);
		Mat frame;

		inputVideo[0] >> frame;
		if (!frame.empty()) frame.copyTo(image(Rect(0, 0, width1, height1)));
		else set1 = 0;
		inputVideo[1] >> frame;
		if (!frame.empty()) frame.copyTo(image(Rect(0, height1, width2, height2)));
		else set2 = 0;
		if (set1 == 0 && set2 == 0) { loop = 0; break; }
		if (!image.empty()) {
			outputVideo.write(image);
			//imshow("image", image);
			//waitKey(1);
		}
	}

	outputVideo.release();

	// Put the absolute path for the script file
	system("mergemp3.bat");

	return 0;
}
