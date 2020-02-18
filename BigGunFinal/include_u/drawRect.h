#pragma once
#include<opencv2/opencv.hpp>

using namespace cv;

class drawbox
{
public:

	void drawBoxBlue(RotatedRect box, Mat& img);
	void drawBoxGreen(RotatedRect box, Mat& img);
	void drawBoxRed(RotatedRect box, Mat& img);
	void drawBoxYellow(RotatedRect box, Mat& img);
};


