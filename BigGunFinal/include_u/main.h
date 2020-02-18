#include <opencv2/opencv.hpp>
#include <opencv2/ml/ml.hpp>
#include <pthread.h>
#include <iostream>
#include <stdlib.h>
#include "ArmorRecognition.h" //装甲识别
#include "SendData.h"
#include "get_omega.h"
//#include "KSJCamera.h"
//#include "serialport.h"	

using namespace std;
using namespace cv;
using namespace cv::ml;




//全局变量q
String videoPath0 = "video/8.avi";
FileStorage fsRead;
Ptr<SVM> SvmLoad;
Rect RoiRect= Rect(0, 0, MATWIDTH, MATHEIGHT); //感兴趣区域初始化
uchar ReceiveBuffer[1] = { '0' };
vector<float> savedis;
int unfind = 0;
char c = 0;




//相机
#ifdef GY
	KSJCamera cap(728, 544, ExposureValue);//728 544
#else
	VideoCapture cap(videoPath0);
#endif

//线程锁
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;/*定义互斥锁*/
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;/*定义互斥锁*/
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;/*定义互斥锁*/
pthread_mutex_t mutex4 = PTHREAD_MUTEX_INITIALIZER;/*定义互斥锁*/
pthread_mutex_t roi_mutex;
pthread_mutex_t data_mutex;
pthread_mutex_t ReceiveMutex;



//处理图像
void* run1(void* arg);
void* run2(void* arg);

//接收数据
void* receive(void* arg);
float dis_compensation1(float dis);

void* receive(void* arg)
{
	while (1)
	{
		pthread_mutex_lock(&ReceiveMutex);
		SerialPort_Recv(ReceiveBuffer, 1);
		pthread_mutex_unlock(&ReceiveMutex);
	}
}

void* run1(void* arg)
{
    ArmorRecognition Target(fsRead);
	SendData uart;

    Mat srcimage;
	Mat Roi_image;

#ifdef AimTrackbar
	namedWindow("Debug", CV_WINDOW_NORMAL);
	createTrackbar("gray_th", "Debug", &Target.gray_th, 255, 0);
	createTrackbar("color_th", "Debug", &Target.color_th, 255, 0);
	createTrackbar("ch", "Debug", &Target.ch, 1, 0);
#endif


	while(1)
	{

		clock_t start, end;
		start = clock();
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

		pthread_mutex_lock(&mutex1);
		cap >> srcimage;
		pthread_mutex_unlock(&mutex1); 


		if (!srcimage.data)
		{
			cout << "Camera reading error!";
			break;
		}
		
		Roi_image = srcimage(RoiRect).clone();

		Target.track_armor(Roi_image, SvmLoad, RoiRect);
		//cout << "x :" << Target.x << ", y :" << Target.y << ", width :" << Target.width << ", height :" << Target.height << endl;
		
		if (Target.Isfind)
		{
			Target.distance = dis_compensation1(Target.distance);

			pthread_mutex_lock(&roi_mutex);
			RoiRect = Rect(Target.x, Target.y, Target.width, Target.height);
			pthread_mutex_unlock(&roi_mutex);

			unfind = 0;
			pthread_mutex_lock(&data_mutex);
			cout << "yaw :" << Target.yaw << ", pitch :" << Target.pitch << ", distance :" << Target.distance << endl;
			//uart.Send(Target.yaw, -1 * Target.pitch, Target.distance / 100.0, 0);
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
			pthread_mutex_lock(&data_mutex);
			cout << "yaw :" << 0 << ", pitch :" << 0 << ", distance :" << 0 << endl;
			//uart.Send(0, 0, 0, 9);
			pthread_mutex_unlock(&data_mutex);
		} 
		end = clock();
		double fps = 1 / ((double)(end - start) / CLOCKS_PER_SEC);
		//cout << "fps = " << fps << endl;

	}
	
}

void* run2(void* arg)
{
    ArmorRecognition Target(fsRead);
	SendData uart;

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

        pthread_mutex_lock(&mutex1);
		cap >> srcimage;
		pthread_mutex_unlock(&mutex1);

        if (!srcimage.data)
        {
            cout << "Camera reading error!";
            break;
        }
        
        Roi_image = srcimage(RoiRect).clone();

        Target.track_armor(Roi_image, SvmLoad, RoiRect);
		//cout << "x :" << Target.x << ", y :" << Target.y << ", width :" << Target.width << ", height :" << Target.height << endl;
	    
		if (Target.Isfind)
        {
            Target.distance = dis_compensation1(Target.distance);

            pthread_mutex_lock(&roi_mutex);
            RoiRect = Rect(Target.x, Target.y, Target.width, Target.height);
			pthread_mutex_unlock(&roi_mutex);

            unfind = 0;
            pthread_mutex_lock(&data_mutex);
			cout << "yaw :" << Target.yaw << ", pitch :" << Target.pitch << ", distance :" << Target.distance << endl;
            //uart.Send(Target.yaw, -1 * Target.pitch, Target.distance / 100.0, 0);
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
            pthread_mutex_lock(&data_mutex);
			cout << "yaw :" << 0 << ", pitch :" << 0 << ", distance :" << 0 << endl;
            //uart.Send(0, 0, 0, 9);
            pthread_mutex_unlock(&data_mutex);
        } 
        
    }

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



