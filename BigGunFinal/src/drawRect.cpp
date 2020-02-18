#include "drawRect.h"

void drawbox::drawBoxBlue(RotatedRect box, Mat& img)
{
	Point2f pt[4];
	int i;
	for (i = 0; i < 4; i++)
	{
		pt[i].x = 0;
		pt[i].y = 0;
	}
	box.points(pt); //计算二维盒子顶点 
	line(img, pt[0], pt[1], CV_RGB(0, 0, 255), 2, 8, 0);
	line(img, pt[1], pt[2], CV_RGB(0, 0, 255), 2, 8, 0);
	line(img, pt[2], pt[3], CV_RGB(0, 0, 255), 2, 8, 0);
	line(img, pt[3], pt[0], CV_RGB(0, 0, 255), 2, 8, 0);
}

void drawbox::drawBoxGreen(RotatedRect box, Mat& img)
{
	Point2f pt[4];
	int i;
	for (i = 0; i < 4; i++)
	{
		pt[i].x = 0;
		pt[i].y = 0;
	}
	box.points(pt); //计算二维盒子顶点 
	line(img, pt[0], pt[1], CV_RGB(0, 255, 0), 2, 8, 0);
	line(img, pt[1], pt[2], CV_RGB(0, 255, 0), 2, 8, 0);
	line(img, pt[2], pt[3], CV_RGB(0, 255, 0), 2, 8, 0);
	line(img, pt[3], pt[0], CV_RGB(0, 255, 0), 2, 8, 0);
}

void drawbox::drawBoxRed(RotatedRect box, Mat& img)
{
	Point2f pt[4];
	int i;
	for (i = 0; i < 4; i++)
	{
		pt[i].x = 0;
		pt[i].y = 0;
	}
	box.points(pt); //计算二维盒子顶点 
	line(img, pt[0], pt[1], CV_RGB(255, 0, 0), 2, 8, 0);
	line(img, pt[1], pt[2], CV_RGB(255, 0, 0), 2, 8, 0);
	line(img, pt[2], pt[3], CV_RGB(255, 0, 0), 2, 8, 0);
	line(img, pt[3], pt[0], CV_RGB(255, 0, 0), 2, 8, 0);
}

void drawbox::drawBoxYellow(RotatedRect box, Mat& img)
{
	Point2f pt[4];
	int i;
	for (i = 0; i < 4; i++)
	{
		pt[i].x = 0;
		pt[i].y = 0;
	}
	box.points(pt); //计算二维盒子顶点 
	line(img, pt[0], pt[1], CV_RGB(255, 255, 0), 2, 8, 0);
	line(img, pt[1], pt[2], CV_RGB(255, 255, 0), 2, 8, 0);
	line(img, pt[2], pt[3], CV_RGB(255, 255, 0), 2, 8, 0);
	line(img, pt[3], pt[0], CV_RGB(255, 255, 0), 2, 8, 0);
}
