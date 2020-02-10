#include "ArmorRecognition.h" //装甲识别
#include "SendData.h"
#include <opencv2/opencv.hpp>
#include <opencv2/ml/ml.hpp>
#include <pthread.h>
#include <iostream>
#include <stdlib.h>

//#include "KSJCamera.h"
//#include "serialport.h"	
using namespace std;
using namespace cv;
using namespace cv::ml;


//#define GY
//#define AimTrackbar

//全局变量
String videoPath0 = "video/q.avi";
FileStorage fsRead;
Ptr<SVM> SvmLoad;
Rect RoiRect= Rect(0, 0, MATWIDTH, MATHEIGHT); //感兴趣区域初始化
uchar ReceiveBuffer[1] = { '0' };
vector<float> savedis;
int unfind = 0;

//相机
#ifdef GY
	KSJCamera cap(728, 544, ExposureValue);//728 544
#else
	VideoCapture cap(videoPath0);
#endif

//线程锁
pthread_mutex_t get_mutex;
pthread_mutex_t receive_mutex;
pthread_mutex_t data_mutex;
pthread_mutex_t dis_mutex;
pthread_mutex_t roi_mutex;
pthread_mutex_t aim_mutex;


//处理图像
void* run1(void* arg);
void* run2(void* arg);

//接收数据
void* receive(void* arg);

//读取自瞄参数
void Aim_param(ArmorRecognition &Target);

//距离补偿
float dis_compensation1(float dis);





void* run1(void* arg)
{
    ArmorRecognition Target;//自瞄对象创建
	SendData uart;//串口发送对象创建

	pthread_mutex_lock(&aim_mutex);
    Aim_param(Target);
	pthread_mutex_unlock(&aim_mutex);

    Mat srcimage;//原始图像
	Mat Roi_image;//roi图像

#ifdef AimTrackbar
	namedWindow("Debug", CV_WINDOW_NORMAL);
#endif

    while(1)
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

#ifdef AimTrackbar
		createTrackbar("gray_th", "Debugg", &Target.gray_th, 255, 0);
		createTrackbar("color_th", "Debug", &Target.color_th, 255, 0);
#endif


        pthread_mutex_lock(&get_mutex);
#ifdef GY
		cap.getSrc(srcimage);
#else
		cap.read(srcimage);
#endif
		pthread_mutex_unlock(&get_mutex);

        if (!srcimage.data)
        {
            cout << "Camera reading error!";
            break;
        }
        //得到图像需要处理的部分
        Roi_image = srcimage(RoiRect).clone();

		// clock_t start, end;
		// start = clock();

		//图像处理
        Target.track_armor(Roi_image, SvmLoad, RoiRect);
		cout << "x :" << Target.x << ", y :" << Target.y << ", width :" << Target.width << ", height :" << Target.height << endl;
		// end = clock();
		// double fps = 1 / ((double)(end - start) / CLOCKS_PER_SEC);
		// cout << "fps = " << fps << endl;


	    if (Target.Isfind)
        {
            Target.distance = dis_compensation1(Target.distance);

			//更新roi
			pthread_mutex_lock(&roi_mutex);
            RoiRect = Rect(Target.x, Target.y, Target.width, Target.height);
			pthread_mutex_unlock(&roi_mutex);

            unfind = 0;
			//cout << "yaw :" << Target.yaw << ", pitch :" << Target.pitch << ", distance :" << Target.distance << endl;

			//数据发送
            pthread_mutex_lock(&data_mutex);
            uart.Send(Target.yaw, -1 * Target.pitch, Target.distance / 100.0, 0);
            pthread_mutex_unlock(&data_mutex);

        }
        else
        {
            unfind++;
            if (unfind >= 10)
            {
				//更新roi
				pthread_mutex_lock(&roi_mutex);
               	RoiRect = Rect(0, 0, MATWIDTH, MATHEIGHT);
				pthread_mutex_unlock(&roi_mutex);
            } 
			//cout << "yaw :" << 0 << ", pitch :" << 0 << ", distance :" << 0 << endl;

			//数据发送
            pthread_mutex_lock(&data_mutex);
            uart.Send(0, 0, 0, 9);
            pthread_mutex_unlock(&data_mutex);
        } 
        
    }

}





void* run2(void* arg)
{
    ArmorRecognition Target;
	SendData uart;

	pthread_mutex_lock(&aim_mutex);
    Aim_param(Target);
	pthread_mutex_unlock(&aim_mutex);

    Mat srcimage;
	Mat Roi_image;
    while(1)
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

        pthread_mutex_lock(&get_mutex);
#ifdef GY
		cap.getSrc(srcimage);
#else
		cap.read(srcimage);
#endif
		pthread_mutex_unlock(&get_mutex);

        if (!srcimage.data)
        {
            cout << "Camera reading error!";
            break;
        }
        
        Roi_image = srcimage(RoiRect).clone();

        Target.track_armor(Roi_image, SvmLoad, RoiRect);
		cout << "x :" << Target.x << ", y :" << Target.y << ", width :" << Target.width << ", height :" << Target.height << endl;
	    
		if (Target.Isfind)
        {
            Target.distance = dis_compensation1(Target.distance);

            pthread_mutex_lock(&roi_mutex);
            RoiRect = Rect(Target.x, Target.y, Target.width, Target.height);
			pthread_mutex_unlock(&roi_mutex);

            unfind = 0;
			//cout << "yaw :" << Target.yaw << ", pitch :" << Target.pitch << ", distance :" << Target.distance << endl;
            pthread_mutex_lock(&data_mutex);
            uart.Send(Target.yaw, -1 * Target.pitch, Target.distance / 100.0, 0);
            pthread_mutex_unlock(&data_mutex);

        }
        else
        {
            unfind++;
            if (unfind >= 10)
            {
              	pthread_mutex_lock(&roi_mutex);
               	RoiRect = Rect(0, 0, MATWIDTH, MATHEIGHT);
				pthread_mutex_unlock(&roi_mutex);
            } 
			//cout << "yaw :" << 0 << ", pitch :" << 0 << ", distance :" << 0 << endl;
            pthread_mutex_lock(&data_mutex);
            uart.Send(0, 0, 0, 9);
            pthread_mutex_unlock(&data_mutex);
        } 
        
    }

}




void* receive(void* arg)
{
	while (1)
	{
		pthread_mutex_lock(&receive_mutex);
		//SerialPort_Recv(ReceiveBuffer, 1);
		pthread_mutex_unlock(&receive_mutex);
	}
}


void Aim_param(ArmorRecognition &Target)
{    
    Target.ch = 0;
	fsRead["bgray_th"] >> Target.gray_th;

	fsRead["color_th"] >> Target.color_th;
	fsRead["diff_angle"] >> Target.diff_angle;
	fsRead["min_area"] >> Target.min_area;
	fsRead["max_area"] >> Target.max_area;

    fsRead["min_length"] >> Target.min_length;
	fsRead["max_length"] >> Target.max_length;
    fsRead["min_armor_angle"] >> Target.min_armor_angle;
	fsRead["max_armor_angle"] >> Target.max_armor_angle;
    fsRead["min_led_Ratio"] >> Target.min_led_Ratio;
	fsRead["max_led_Ratio"] >> Target.max_led_Ratio;
    fsRead["min_armor_Ratio"] >> Target.min_armor_Ratio;
	fsRead["max_armor_Ratio"] >> Target.max_armor_Ratio;
    
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

