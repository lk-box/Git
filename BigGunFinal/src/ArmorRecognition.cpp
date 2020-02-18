#include "ArmorRecognition.h"


//开多线程时图像显示锁
pthread_mutex_t image_mutex;

ArmorRecognition::ArmorRecognition(FileStorage fsRead)
{
	ch = 1;
	fsRead["bgray_th"] >> gray_th;
	fsRead["color_th"] >> color_th;

	fsRead["diff_angle"] >> diff_angle;

	fsRead["min_area"] >> min_area;
	fsRead["max_area"] >> max_area;

    fsRead["min_length"] >> min_length;
	fsRead["max_length"] >> max_length;

    fsRead["min_armor_angle"] >> min_armor_angle;
	fsRead["max_armor_angle"] >> max_armor_angle;

    fsRead["min_led_Ratio"] >> min_led_Ratio;
	fsRead["max_led_Ratio"] >> max_led_Ratio;

    fsRead["min_armor_Ratio"] >> min_armor_Ratio;
	fsRead["max_armor_Ratio"] >> max_armor_Ratio;
}

enum
{
	WIDTH_GREATER_THAN_HEIGHT,
	ANGLE_TO_UP
};

//调整矩形角度函数
cv::RotatedRect& adjustRec(cv::RotatedRect& rec, const int mode)
{
	using std::swap;

	float& width = rec.size.width;
	float& height = rec.size.height;
	float& angle = rec.angle;

	if (mode == WIDTH_GREATER_THAN_HEIGHT)
	{
		if (width < height)
		{
			swap(width, height);
			angle += 90.0;
		}
	}

	while (angle >= 90.0) angle -= 180.0;
	while (angle < -90.0) angle += 180.0;

	if (mode == ANGLE_TO_UP)
	{
		if (angle >= 45.0)
		{
			swap(width, height);
			angle -= 90.0;
		}
		else if (angle < -45.0)
		{
			swap(width, height);
			angle += 90.0;
		}
	}
	return rec;
}


//装甲板判定条件
bool ArmorRecognition::suitble(RotatedRect &r1, RotatedRect &r2)
{
	double dAngle = abs(r1.angle - r2.angle);
	while (dAngle > 180)
	{
		dAngle -= 180;
	}
	if (dAngle < diff_angle || 180 - dAngle < diff_angle)
	{
		if ((r1.size.height*0.8f < r2.size.height
			&& r1.size.height*1.2f > r2.size.height)
			|| (r2.size.height*0.8f < r1.size.height
				&& r2.size.height*1.2f > r1.size.height)) // 0.7   1.3
		{
			float armor_width = fabs(r1.center.x - r2.center.x);
			if (armor_width > (r1.size.width + r2.size.width) )
			{
				float h_max = (r1.size.height + r2.size.height) / 2.0f;
				if (fabs(r1.center.y - r2.center.y) <  h_max)
				{
					return true;
				}
			}
		}
	}
	return false;
}


//svm预测函数
int svmPredict( Mat & image, Ptr<SVM> svm)
{

	Mat dstImage = image.clone();

	Mat src = image.clone();
	if (!src.data)
	{
		cout << "empty!" << endl;
	}
	Mat trainTempImg = Mat::zeros(Size(48, 48), CV_8UC3);

	resize(src, trainTempImg, trainTempImg.size(), 0, 0);

	HOGDescriptor *hog = new HOGDescriptor(Size(48, 48), Size(8, 8), Size(4, 4), Size(4, 4), 9);
	vector<float>descriptors;     
	hog->compute(trainTempImg, descriptors);  
	//cout << "HOG dims: " << descriptors.size() << endl;  //4356
	Mat SVMtrainMat;
	SVMtrainMat.create(1, descriptors.size(), CV_32FC1);

	int n = 0;
	for (vector<float>::iterator iter = descriptors.begin(); iter != descriptors.end(); iter++)
	{
		SVMtrainMat.at<float>(0, n) = *iter;
		n++;
	}

	int ret = svm->predict(SVMtrainMat);

	return ret;
}


//绘制灯条函数
void ArmorRecognition::draw_light(RotatedRect box, Mat img)
{
	Point2f pt[4];
	int i;
	for (i = 0; i < 4; i++)
	{
		pt[i].x = 0;
		pt[i].y = 0;
	}
	box.points(pt);

	line(img, pt[0], pt[1], CV_RGB(0, 225, 255), 2, 8, 0);
	line(img, pt[1], pt[2], CV_RGB(0, 225, 255), 2, 8, 0);
	line(img, pt[2], pt[3], CV_RGB(0, 225, 255), 2, 8, 0);
	line(img, pt[3], pt[0], CV_RGB(0, 225, 255), 2, 8, 0);
}


//绘制装甲板，并将装甲板坐标恢复为原图坐标
void  ArmorRecognition::drawBox(RotatedRect &box, Mat img, Rect RoiRect)
{
	Point2f pt[4];
	int i;
	box.points(pt);
	
#ifdef getBox	
	line(img, pt[0], pt[1], CV_RGB(0, 255, 255), 3, 8, 0);
	line(img, pt[1], pt[2], CV_RGB(0, 255, 255), 3, 8, 0);
	line(img, pt[2], pt[3], CV_RGB(0, 255, 255), 3, 8, 0);
	line(img, pt[3], pt[0], CV_RGB(0, 255, 255), 3, 8, 0);
#endif

	for (i = 0; i < 4; i++)
	{
		pt[i].x += RoiRect.x;
		pt[i].y += RoiRect.y;
	}
	box.center.x += RoiRect.x;
	box.center.y += RoiRect.y;
	if (box.center.x < 0 || box.center.x > MATWIDTH)
	{
		box.center.x = 0;
	}
	if (box.center.y < 0 || box.center.y > MATHEIGHT)
	{
		box.center.y = 0;
	} 
	x = pt[1].x - (box.size.width * 0.5);
	y = pt[1].y - (box.size.height * 0.5);
	width = box.size.width * 2;
	height = box.size.height * 2;

}


//灰度平均阈值函数
int GetMeanThreshold(Mat grayImg)
{
	int width = grayImg.cols;
	int height = grayImg.rows;
	int x = 0, y = 0;
	int pixelCount[256];
	float pixelPro[256];
	int pixelSum = width * height;
  
	for (uint nI = 0; nI < 256; nI++)
	{
		pixelCount[nI] = 0;
		pixelPro[nI] = 0;
	}
 
	for (uint nI = y; nI < height; nI++)
	{
		uchar* pixData = grayImg.ptr<uchar>(nI);
		for (uint nJ = x; nJ < width; nJ++)
		{
			pixelCount[pixData[nJ]] = pixData[nJ];
		}
	}
	int Sum = 0, Amount = 0;
	for (int Y = 0; Y < 256; Y++)
	{
		Amount += pixelCount[Y];
		Sum += Y * pixelCount[Y];
	}
	return Sum / Amount;
}


//获取装甲板数字函数
int ArmorRecognition::get_num(RotatedRect box, Ptr<SVM> svm, Mat frame)
{
	Point2f pt[4];
	int i;
	for (i = 0; i < 4; i++)
	{
		pt[i].x = 0;
		pt[i].y = 0;
	}
	box.points(pt);
	
	int x = pt[3].x + box.size.width / 5;
	int y = pt[3].y - box.size.height / 2;
	int wh = box.size.width * 3 / 5;
	int ht = box.size.height * 1.5;

	if (x <= 0)
	{
		x = 0;
	}

	if (y <= 0)
	{
		y = 0;
	}

	if (x + wh > frame.cols)
	{
		wh = frame.cols - x;
	}
	if (y + ht > frame.rows)
	{
		ht = frame.rows - y;
	}

	//cout << "x:" << x << "y:" << y << "width:" << wh << "hight:" << ht << endl;	
	Mat imageROI;
	//cout << "开始构建装甲板数字roi..." << endl;
	imageROI = frame(Rect(x, y, wh, ht));
	//cout << "构建装甲板数字roi成功..." << endl;
	//cout << 22 << endl;


	resize(imageROI, imageROI, cv::Size(48, 48), (0, 0), (0, 0), cv::INTER_LINEAR);  

	cvtColor(imageROI, imageROI, COLOR_BGR2GRAY);
	equalizeHist(imageROI, imageROI);
	int tth = GetMeanThreshold(imageROI);
	
	threshold(imageROI, imageROI, tth, 255, THRESH_BINARY);
	
#ifdef NumRoiimg
	imshow("NumRoi", imageROI);
#endif

	//string Img_Name = "D:\\RM\\Opencv\\lk\\data\\h0\\car" + to_string(k) + ".png";
	//imwrite(Img_Name, imageROI);
	int a = 0;
	a = svmPredict(imageROI, svm);

	return a;

}



vector<my_rect> ArmorRecognition::armorDetect(vector<RotatedRect> vEllipse, Mat frame, Ptr<SVM> svm)
{
	vector<my_rect> vRlt;
	my_rect armor;
	int nL, nW;
	double dAngle;
	vRlt.clear();
	if (vEllipse.size() < 2)
	{
		return vRlt;
	}
	for (unsigned int i = 0; i < vEllipse.size() - 1; i++)
	{
		for (unsigned int j = i + 1; j < vEllipse.size(); j++)
		{
			if (suitble(vEllipse[i], vEllipse[j]))
			{
				armor.rect.center.x = (vEllipse[i].center.x + vEllipse[j].center.x) / 2;
				armor.rect.center.y = (vEllipse[i].center.y + vEllipse[j].center.y) / 2;
				armor.rect.angle = (vEllipse[i].angle + vEllipse[j].angle) / 2;
				nL = (vEllipse[i].size.height + vEllipse[j].size.height) / 2;
				nW = sqrt((vEllipse[i].center.x - vEllipse[j].center.x) * (vEllipse[i].center.x - vEllipse[j].center.x) + (vEllipse[i].center.y - vEllipse[j].center.y) * (vEllipse[i].center.y - vEllipse[j].center.y)); //װ�׵Ŀ��ȵ�������LED������ת������������ľ���

				if (nL < nW)
				{
					armor.rect.size.height = nL;
					armor.rect.size.width = nW;
				}
				else
				{
					armor.rect.size.height = nW;
					armor.rect.size.width = nL;
				}
				adjustRec(armor.rect, WIDTH_GREATER_THAN_HEIGHT);

				//cout << "angle :"<< armor.rect.angle << endl;
				//cout << "Ratio:" << armor.rect.size.width / armor.rect.size.height << endl;
				if (armor.rect.size.width / armor.rect.size.height > min_armor_Ratio 
				&& armor.rect.size.width / armor.rect.size.height < max_armor_Ratio
				&& armor.rect.angle > min_armor_angle
				&& armor.rect.angle < max_armor_angle 
				)
				{
#ifdef SvmPredict
					int finalnum;
					//cout << "开始预测装甲板数字..." << endl;
					finalnum = get_num(armor.rect, svm, frame);
					//cout << "预测装甲板数字结束..." << endl;
					if (finalnum)
					{
						armor.num = finalnum;
						vRlt.push_back(armor);
					}
#else
					//cout << "找到候选装甲板..." << endl;
					vRlt.push_back(armor);
#endif
				}
			}
		}
	}
	return vRlt;
}


//主处理图像函数
void ArmorRecognition::track_armor(Mat frame, Ptr<SVM> svm, Rect RoiRect)
{
	
	outfile.open("log.txt");
	outfile << "开始处理..." << endl;
	Mat mask;
	Size imgSize;
	imgSize = frame.size();

	Mat rimg = Mat(imgSize, CV_8UC1);
	Mat bimg = Mat(imgSize, CV_8UC1);

	Mat diff = Mat(imgSize, CV_8UC1);
	Mat th = Mat(imgSize, CV_8UC1);
	Mat th1 = Mat(imgSize, CV_8UC1);
	Mat rlt = Mat(imgSize, CV_8UC1);

	vector<vector<Point>> contour;
	RotatedRect light;


	vector<my_rect> vRlt;
	vector<RotatedRect> vEllipse;
	my_rect finalarmor;

	Mat gray = Mat(imgSize, CV_8UC1);
	cvtColor(frame, gray, COLOR_BGR2GRAY);//225

	Mat channels[3];
	split(frame, channels);

	bimg = channels[0];
	rimg = channels[2];

	if (ch == 0)
	{
		diff = bimg - rimg;
	}
	else if (ch == 1)
	{
		diff = rimg - bimg;
	}


	threshold(diff, th1, color_th, 255, THRESH_BINARY);
	threshold(gray, th, gray_th, 255, THRESH_BINARY);
	dilate(th, th, Mat(), Point(-1, -1), 2);
	dilate(th1, th1, Mat(), Point(-1, -1), 2);
	
#ifdef gray_th_img
	imshow("gray_th", th);
#endif

#ifdef color_th_img
	imshow("color_th", th1);
#endif

	bitwise_and(th, th1, rlt);
	outfile << "得到混合图像..." << endl;
#ifdef rltimg
	imshow("rlt", rlt);
#endif

	outfile << "寻找灯条开始..." << endl;
	findContours(rlt, contour, RETR_EXTERNAL, CHAIN_APPROX_NONE);

	for (int k = 0; k < contour.size(); k++)
	{
		double area = contourArea(contour[k]);
		double length = arcLength(contour[k], true); 
		if (area < min_area || area > max_area || length < min_length || length > max_length)
		{
			continue;
		}
		if (contour[k].size() > 6)
		{
			light = fitEllipse(contour[k]);
			adjustRec(light, ANGLE_TO_UP);
			if (light.angle > 50 || light.angle < -50)
			{
				continue;
			}
			if ( light.size.height / light.size.width > min_led_Ratio && light.size.height / light.size.width < max_led_Ratio)
			{
				outfile << "得到灯条..." << endl;
#ifdef getLight
				draw_light(light, frame);
#endif
				vEllipse.push_back(light);
			}
		}
		
	}

	contour.clear();
	outfile << "寻找装甲板开始..." << endl;
	vRlt = armorDetect(vEllipse, frame, svm);

	//cout << "count :" << vRlt.size() << endl;
	if (vRlt.size() != 0)
	{
		finalarmor = vRlt[0];
		if (vRlt.size() > 1)
		{
			for (int i = 1; i < vRlt.size(); i++)
			{
				if (finalarmor.rect.size.area() < vRlt[i].rect.size.area())
				{
					finalarmor = vRlt[i];
				}
			}
		}
		outfile << "得到最终装甲板..." << endl;
		drawBox(finalarmor.rect, frame, RoiRect);

#ifdef SvmPredict
		putText(frame, to_string(finalarmor.num), Point(finalarmor.rect.center.x - 5, finalarmor.rect.center.y + 5), 5, 1.2, Scalar(0, 255, 255), 2);
#endif			
		//cout << finalnum << endl;


		ArrorAttitudeAlgorithm Get_data;
		double temp_yaw = 0, temp_pitch = 0;

		outfile << "获取数据..." << endl;
		distance = Get_data.angleSover(finalarmor.rect, temp_yaw, temp_pitch);
		pitch = temp_pitch;
		yaw = temp_yaw;


		if (x < 0 || x > MATWIDTH)
		{
			x = 0;
		}
		if (y < 0 || y > MATHEIGHT)
		{
			y = 0;
		}
		if (width < 0)
		{
			width = 0;
		}
		if (height < 0)
		{
			height = 0;
		}
		if (x + width > MATWIDTH)
		{
			width = MATWIDTH - x;
		}
		if (y + height > MATHEIGHT)
		{
			height = MATHEIGHT - y;
		}
		

		Isfind = 1;

	}
	else 
	{
		outfile << "未找到最终装甲板..." << endl;
		Isfind = 0;
	}

 #ifdef frameimg
	pthread_mutex_lock(&image_mutex);
	imshow("framee", frame);
	waitKey(1);
	pthread_mutex_unlock(&image_mutex);
#endif 	

	vEllipse.clear();
	vRlt.clear();
	outfile << "处理结束..." << endl;
	outfile.close();
}



