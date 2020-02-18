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

#define MATWIDTH 728
#define MATHEIGHT 544

//调试
#define Debug

#ifdef Debug

#define AimTrackbar

#define frameimg
// #define gray_th_img   //灰度二值图
// #define color_th_img  //通道相减二值图
// #define rltimg        //融合图像

// #define NumRoiimg   //数字二值图
// #define SvmPredict  //开启svm判定装甲板

// #define getBox//框出目标装甲板
// #define getLight//框出灯条

#endif





typedef struct
{
	RotatedRect rect;
	float max_led_height;
	int num;
}my_rect;


class ArmorRecognition
{
public:

	ArmorRecognition(FileStorage fsRead);
	bool suitble(RotatedRect &r1, RotatedRect &r2);
	void track_armor(Mat frame, Ptr<SVM> svm, Rect RoiRect);
	void draw_light(RotatedRect box, Mat img);
	void drawBox(RotatedRect &box, Mat img, Rect RoiRect);
	int get_num(RotatedRect box, Ptr<SVM> svm, Mat frame);
	vector<my_rect> armorDetect(vector<RotatedRect> vEllipse, Mat frame, Ptr<SVM> svm);

	int ch;
	int color_th, gray_th;

	double yaw;
	double pitch;
	float distance;

	float height, width, x, y;

	int Isfind;

private:

	ofstream outfile;
	int min_area, max_area;
	int min_length, max_length;
	int min_armor_angle, max_armor_angle;
	int min_led_Ratio, max_led_Ratio;
	int min_armor_Ratio, max_armor_Ratio;
	int diff_angle;
};