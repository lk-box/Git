#pragma once
#include <iostream>
#include <opencv2/core/utility.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <pthread.h>
#include "opencv2/videoio.hpp"
#include <cmath>
#include <opencv2/ml/ml.hpp> 
#include <fstream>
#include <stdio.h>
#include "ArrorAttitudeAlgorithm.h"

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
	int ch;
	int color_th, gray_th;
	int min_area, max_area;
	int min_length, max_length;
	int min_armor_angle, max_armor_angle;
	int min_led_Ratio, max_led_Ratio;
	int min_armor_Ratio, max_armor_Ratio;
	int diff_angle;

	double yaw;
	double pitch;
	float distance;

	float height;
	float width;
	float x;
	float y;

	int Isfind;
	
	bool suitble(RotatedRect &r1, RotatedRect &r2);
	void track_armor(Mat frame, Ptr<SVM> svm, Rect RoiRect);
	void draw_light(RotatedRect box, Mat img);
	void drawBox(RotatedRect &box, Mat img, Rect RoiRect);
	int get_num(RotatedRect box, Ptr<SVM> svm, Mat frame);
	vector<my_rect> armorDetect(vector<RotatedRect> vEllipse, Mat frame, Ptr<SVM> svm);

private:
ofstream outfile;
};