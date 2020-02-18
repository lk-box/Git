#pragma once
#include <opencv2/opencv.hpp>
#include "buff_detect.h"
#define PI 3.1415927

using namespace cv;
using namespace std;

class get_omega
{
public:
	
	int frame_center_y = 0;
	int frame_center_x = 0;

	Point forecast_point(int direction, float angle, float omega, struct logo R);
	void energy_buff(float omega, int direction, int buff, int show_num, float angle,  float time1, struct logo final_center_R);
	void drawgraph(float point_x, float point_y, int show_num);
	void forecast(Mat& image, float& yaw, float& pitch, struct logo final_center_R);
private:

	float delta_theta = 0;
	
	
	float T = 0.5;//发射延时

	Mat graph_x, graph_y;
	vector<Point> point1;
	vector<Point> point2;
	Point2f point0;

	float get_delta_x(float time)
	{
		float x = 0;
		x = -0.785 / 1.884 * cosf(1.884 * (time + T)) + 1.305 * (time + T)+
			0.785 / 1.884 * cosf(1.884 * time) - 1.306 * time;//对速度进行定积分

		return x * 360 / (2 * PI); //x的单位为rad 1rad == 360 / (2 * PI)
	}
};

