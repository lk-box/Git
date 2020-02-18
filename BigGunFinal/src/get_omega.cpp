#include "get_omega.h"
#include"serialport.h"

#define PI 3.1415927
#define DISTENCE 8000.0

#define DEBUG

void get_omega::energy_buff(float omega, int direction, int buff_type, int show_num, float angle,  float time1, struct logo final_center_R)
{
	if (buff_type != -1)
	{
		if (buff_type == 0)//small buff
		{
			point0 = forecast_point(direction,angle, omega, final_center_R);
		}
		else
		{
			//积分得到角度
			delta_theta = get_delta_x(time1);
			point0 = forecast_point(direction,angle + delta_theta, 0, final_center_R);
		}
	}
}

void get_omega::forecast(Mat& image, float& yaw,  float& pitch, struct logo final_center_R)
{

	//得到摄像头中心到达预测点所需要的角度
	float delta_x1 = point0.x - frame_center_x;//得到x方向上的象素距离
	float delta_y1 = -(point0.y - frame_center_y);
	float distance_x = delta_x1 / final_center_R.r * 700;//得到x方向的实际距离
	float distance_y = delta_y1 / final_center_R.r * 700;

	yaw = atan(distance_x / DISTENCE) / PI * 180;
	pitch = atan(distance_y / DISTENCE) / PI * 180;

	//cout << "yaw = " << yaw;
	//cout << "    pitch = " << pitch << endl;

#ifdef DEBUG

	circle(image, point0, 1, Scalar(0, 255, 0), 5);
	line(image, Point(0, 240), Point(894, 240), Scalar(124, 200, 150), 1);
	line(image, Point(427, 0), Point(427, 480), Scalar(124, 200, 150), 1);
	line(image, point0, Point(427, 240), Scalar(200, 150, 26), 1);

	//drawgraph(yaw, pitch, show_num);

	//imshow("graph_x", graph_x);
	//imshow("graph_y", graph_y);
#endif // DEBUG
}

Point get_omega::forecast_point(int direction, float angle, float omega, struct logo R)
{
	Point2f point;

	if (direction == 1)//顺时针情况
	{
		point.x = R.x + R.r * sin((angle + omega * T) / 180 * PI);
		point.y = R.y - R.r * cos((angle + omega * T) / 180 * PI);

	}
	else//逆时针情况
	{
		point.x = R.x + R.r * sin((angle - omega * T) / 180 * PI);
		point.y = R.y - R.r * cos((angle - omega * T) / 180 * PI);
	}
	return point;
}

void get_omega::drawgraph(float point_x, float point_y, int show_num)
{
	graph_x.create(Size(1400, 480), CV_8UC3);
	graph_y.create(Size(1400, 480), CV_8UC3);

	//Point point_1(show_num, 480 - point_x * 480 / (frame_center_x * 2));//point.x
	//Point point_2(show_num, 480 - point_y * 480 / (frame_center_y * 2));//point.y

	//Point point_1(show_num, 480 - point_x * 480 / 360);//angle

	Point point_1(show_num, 100 - point_x);//yaw
	Point point_2(show_num, 100 - point_y);//pitch

	point1.push_back(point_1);
	point2.push_back(point_2);

	if (point1.size() > 1)
	{
		line(graph_x, point1[point1.size() - 2], point1[point1.size() - 1], Scalar(0, 0, 255));
		line(graph_y, point2[point2.size() - 2], point2[point2.size() - 1], Scalar(0, 255, 0));
	}
}