//#pragma comment(lib,"pthreadVC2.lib")
#include <opencv2/opencv.hpp>
#include <opencv2/ml/ml.hpp> 
#include <pthread.h>
#include "ArmorRecognition.h"
//#include "KSJCamera.h"
#include "Result.h"
//#include "serialport.h"		
#include <iostream>
#include <stdlib.h>
using namespace std;
using namespace cv;
using namespace cv::ml;

#define ShowTrackbar
//#define GY


pthread_mutex_t counter_mutex;
pthread_mutex_t receive_mutex;
pthread_mutex_t data_mutex;
pthread_mutex_t dis_mutex;

uchar ReceiveBuffer[1] = { '0' };
vector<float> savedis;
String videoPath0 = "video/q.avi";
FileStorage fsRead;
Ptr<SVM> SvmLoad;
int ExposureValue = 1;
Result Roitarget;
Rect RoiRect;
int Unfind = 0;


void* run1(void* arg);
//void* run2(void* arg);
void* receive(void* arg);
float dis_compensation1(float dis);
void SendData(double yaw, double pitch, double distance, int number);



#ifdef GY

	KSJCamera cap(728, 544, ExposureValue);//728 544

#else

	VideoCapture cap(videoPath0);

#endif





void* receive(void* arg)
{
	while (1)
	{
		pthread_mutex_lock(&receive_mutex);
		//SerialPort_Recv(ReceiveBuffer, 1);
		pthread_mutex_unlock(&receive_mutex);
	}
}


void* run1(void* arg)
{
	Result r;
	Mat srcimage;
	ArmorRecognition Target;
	RoiRect = Rect(0, 0, MATWIDTH, MATHEIGHT);
   // Mat trackbar;
	//trackbar = Mat::zeros(Size(1, 1), CV_8UC1);

	fsRead["bgray_th"] >> Target.gray_th;
	fsRead["color_th"] >> Target.color_th;
	fsRead["ch"] >> Target.ch;
	fsRead["diff_angle"] >> Target.diff_angle;
	fsRead["min_area"] >> Target.min_area;
	fsRead["max_area"] >> Target.max_area;

#ifdef ShowTrackbar

	namedWindow("data", 0);
	createTrackbar("bgray_th", "data", &Target.gray_th, 255, 0);
	createTrackbar("color_th", "data", &Target.color_th, 255, 0);
	createTrackbar("ch", "data", &Target.ch, 1, 0);
	createTrackbar("diff_angle", "data", &Target.diff_angle, 30, 0);

#endif

	while (1)
	{
		if (ReceiveBuffer[0] == 'r')
		{
			Target.ch = 1;
			fsRead["rgray_th"] >> Target.gray_th;
		}
		else if (ReceiveBuffer[0] == 'b')
		{
			Target.ch = 0;
			fsRead["bgray_th"] >> Target.gray_th;
		}


		pthread_mutex_lock(&counter_mutex);
#ifdef GY
		cap.getSrc(srcimage);
#else
		cap.read(srcimage);
#endif
		pthread_mutex_unlock(&counter_mutex);

		

		if (srcimage.data)
		{
			clock_t start, end;
			start = clock();

			r.r_x = RoiRect.x;
			r.r_y = RoiRect.y;
			r.r_height = RoiRect.height;
			r.r_width = RoiRect.width;
			Mat frame = srcimage(RoiRect).clone();

			Target.b_track_armor(srcimage, frame, SvmLoad);

			end = clock();
			double fps = 1 / ((double)(end - start) / CLOCKS_PER_SEC);
			cout << "fps = " << fps << endl;

			pthread_mutex_lock(&dis_mutex);
			r.d_distance = dis_compensation1(r.d_distance);
			pthread_mutex_unlock(&dis_mutex);

			if (r.r_flag == 1)
			{
				pthread_mutex_lock(&data_mutex);
				RoiRect = Rect(r.r_x, r.r_y, r.r_width, r.r_height);
				pthread_mutex_unlock(&data_mutex);
                Unfind = 0;
			}
			else
			{
				Unfind++;
				if (Unfind >= 13)
				{
					pthread_mutex_lock(&data_mutex);
					RoiRect = Rect(0, 0, MATWIDTH, MATHEIGHT);
					pthread_mutex_unlock(&data_mutex);
				}
                else
                {
                    pthread_mutex_lock(&data_mutex);
                    RoiRect = Rect(r.r_x, r.r_y, r.r_width, r.r_height);
                    pthread_mutex_unlock(&data_mutex);
                }
			}
			pthread_mutex_unlock(&data_mutex);
		}
		else
		{
			break;
		}

/* #ifdef ShowTrackbar
		pthread_mutex_lock(&counter_mutex);
		imshow("data", trackbar);
		waitKey(1);
		pthread_mutex_unlock(&counter_mutex);
#endif */

	}
	return NULL;
}









void UartSend(int date1, int date2, int date3, int date4, int date5, int date6, int
	date7, char flag1, char flag2)
{
	uchar data1[18] = { '0' };

	data1[0] = '$';

	data1[1] = (uchar)(date1 >> 8);
	data1[2] = (uchar)(date1);
	data1[3] = (uchar)(date2 >> 8);
	data1[4] = (uchar)(date2);

	data1[5] = (uchar)(date3 >> 8);
	data1[6] = (uchar)(date3);
	data1[7] = (uchar)(date4 >> 8);
	data1[8] = (uchar)(date4);

	data1[9] = (uchar)(date5 >> 8);
	data1[10] = (uchar)(date5);
	data1[11] = (uchar)(date6 >> 8);
	data1[12] = (uchar)(date6);

	data1[13] = (uchar)(date7 >> 8);
	data1[14] = (uchar)(date7);

	data1[15] = flag1;
	data1[16] = flag2;

	data1[17] = '#';
	//SerialPort_Send(data1, 18);

}

void SendData(double yaw, double pitch, double distance, int number)
{

	// printf("\t[Yaw=%f\tPitch=%f\tdist=%f\tarrorNum=%d]\n" 
	//       yaw, pitch, distance, number);

	char flag1, flag2;

	int yaw_integr;
	int yaw_decimals;

	int pitch_integr;
	int pitch_decimals;

	int distance_integr;
	int distance_decimals;


	if (yaw >= 0)    flag1 = 1;
	else           flag1 = 0;

	if (pitch >= 0) flag2 = 1;
	else           flag2 = 0;



	yaw_decimals = int(abs(yaw * 100) - int(abs(yaw)) * 100);
	yaw_integr = int(abs(yaw));

	pitch_decimals = int(abs(pitch * 100) - int(abs(pitch)) * 100);
	pitch_integr = int(abs(pitch));

	distance_decimals = int(distance * 100 - int(distance) * 100);
	distance_integr = int(distance);

	UartSend(yaw_integr, yaw_decimals, pitch_integr, pitch_decimals, distance_integr, distance_decimals, number, flag1, flag2);
}

float dis_compensation1(float dis)
{

	if (savedis.size() != 10)//20
	{
		savedis.push_back(dis);
		return dis;
	}
	else
	{
		savedis.erase(savedis.begin());
		savedis.push_back(dis);
		float add = 0;
		for (unsigned int i = 1; i < 8; i++)
		{
			add = add + savedis[i];
		}
		float aveadd = add / 7.0;
		return (aveadd * 0.7 + savedis[8] * 0.3);
	}
}