#pragma once
#include <iostream>
#include <opencv2/core/utility.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <pthread.h>
#include "opencv2/videoio.hpp"
#include <cmath>
#include <opencv2/ml/ml.hpp> 
#include "Result.h"
#include <fstream>
#include <stdio.h>
#include "ArrorAttitudeAlgorithm.h"
#include "Result.h"

using namespace cv;
using namespace std;
using namespace cv::ml;
#define MATWIDTH 800
#define MATHEIGHT 600

typedef struct
{
	RotatedRect rect;
	float max_led_height;
	int num;
}my_rect;


class ArmorRecognition
{
public:
	int color_th, gray_th;
	int min_area, max_area;
	int ch;
	int diff_angle;

	ArmorRecognition();
	bool suitble(RotatedRect &r1, RotatedRect &r2);
	void b_track_armor(Mat srcimage, Mat frame, Result &r, FileStorage fs, Ptr<SVM> svm);
	void draw_light(RotatedRect box, Mat img);
	void drawBox(RotatedRect box, Mat img);
	int get_num(RotatedRect box, Ptr<SVM> svm, Mat frame);
	vector<my_rect> armorDetect(vector<RotatedRect> vEllipse, Mat frame, Ptr<SVM> svm);

private:
	Mat dstimg;
	int tsize1;
	int tangle, bright;
	int k;
	int thresh, thresh1;
	
	
};