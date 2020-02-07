#include "ArmorRecognition.h"

//#define SvmPredict
#define frameimg
//#define gray_th_img
//#define color_th_img
//#define rltimg
#define NumRoiimg



static pthread_mutex_t image = PTHREAD_MUTEX_INITIALIZER;
enum
{
	WIDTH_GREATER_THAN_HEIGHT,
	ANGLE_TO_UP
};



ArmorRecognition::ArmorRecognition()
{
	k = 0;
}

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
				// ���������߶Ȳ��
				if (fabs(r1.center.y - r2.center.y) <  h_max)
				{
					return true;
				}
			}
		}
	}
	return false;
}

int svmPredict( Mat & image, Ptr<SVM> svm)
{
	//Ptr<ml::SVM> svm = Algorithm::load<ml::SVM>(_Filename);
	//�������    
	Mat dstImage = image.clone();

	Mat src = image.clone();
	if (!src.data)
	{
		cout << "empty!" << endl;
	}
	Mat trainTempImg = Mat::zeros(Size(48, 48), CV_8UC3);

	resize(src, trainTempImg, trainTempImg.size(), 0, 0);

	HOGDescriptor *hog = new HOGDescriptor(Size(48, 48), Size(8, 8), Size(4, 4), Size(4, 4), 9);
	vector<float>descriptors;//��Ž��       
	hog->compute(trainTempImg, descriptors); //Hog��������  , Size(1, 1), Size(0, 0)    
	//cout << "HOG dims: " << descriptors.size() << endl;  //��ӡHog����ά��  ��������4356
	Mat SVMtrainMat;
	SVMtrainMat.create(1, descriptors.size(), CV_32FC1);

	int n = 0;
	for (vector<float>::iterator iter = descriptors.begin(); iter != descriptors.end(); iter++)
	{
		SVMtrainMat.at<float>(0, n) = *iter;
		n++;
	}

	int ret = svm->predict(SVMtrainMat);//�����

	return ret;
}

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

	line(img, pt[0], pt[1], CV_RGB(0, 255, 255), 1, 8, 0);
	line(img, pt[1], pt[2], CV_RGB(0, 255, 255), 1, 8, 0);
	line(img, pt[2], pt[3], CV_RGB(0, 255, 255), 1, 8, 0);
	line(img, pt[3], pt[0], CV_RGB(0, 255, 255), 1, 8, 0);
}

void  ArmorRecognition::drawBox(RotatedRect box, Mat img)
{
	Point2f pt[4];
	int i;
	for (i = 0; i < 4; i++)
	{
		pt[i].x = 0;
		pt[i].y = 0;
	}
	box.points(pt);
	line(img, pt[0], pt[1], CV_RGB(0, 255, 255), 3, 8, 0);
	line(img, pt[1], pt[2], CV_RGB(0, 255, 255), 3, 8, 0);
	line(img, pt[2], pt[3], CV_RGB(0, 255, 255), 3, 8, 0);
	line(img, pt[3], pt[0], CV_RGB(0, 255, 255), 3, 8, 0);


}

int daJinThreshold(Mat grayImg)
{
	if (grayImg.channels() != 1)
		cvtColor(grayImg, grayImg, COLOR_BGR2GRAY);

	int width = grayImg.cols;
	int height = grayImg.rows;
	int x = 0, y = 0;
	int pixelCount[256];
	float pixelPro[256];
	int pixelSum = width * height, threshold = 0;


	//��ʼ��  
	for (uint nI = 0; nI < 256; nI++)
	{
		pixelCount[nI] = 0;
		pixelPro[nI] = 0;
	}

	//ͳ�ƻҶȼ���ÿ������������ͼ���еĸ���  
	for (uint nI = y; nI < height; nI++)
	{
		uchar* pixData = grayImg.ptr<uchar>(nI);
		for (uint nJ = x; nJ < width; nJ++)
		{
			pixelCount[pixData[nJ]] = pixData[nJ];
		}
	}


	//����ÿ������������ͼ���еı���  
	for (uint nI = 0; nI < 256; nI++)
	{
		pixelPro[nI] = (float)(pixelCount[nI]) / (float)(pixelSum);
	}

	//����ostu�㷨,�õ�ǰ���ͱ����ķָ�  
	//�����Ҷȼ�[0,255],������������ĻҶ�ֵ,Ϊ�����ֵ  
	float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;
	for (uint nI = 0; nI < 256; nI++)//iΪ��ֵ
	{
		w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;

		for (uint nJ = 0; nJ < 256; nJ++)//������ֵ���鵱i=0��1��2��3...255
		{
			if (nJ <= nI) //��������  
			{
				//��iΪ��ֵ���࣬��һ���ܵĸ���  
				w0 += pixelPro[nJ];
				u0tmp += nJ * pixelPro[nJ];
			}
			else       //ǰ������  
			{
				//��iΪ��ֵ���࣬�ڶ����ܵĸ���  
				w1 += pixelPro[nJ];
				u1tmp += nJ * pixelPro[nJ];
			}
		}

		u0 = u0tmp / w0;        //��һ���ƽ���Ҷ�  
		u1 = u1tmp / w1;        //�ڶ����ƽ���Ҷ�  
		u = u0tmp + u1tmp;      //����ͼ���ƽ���Ҷ�  
		//������䷽��  
		deltaTmp = w0 * (u0 - u)*(u0 - u) + w1 * (u1 - u)*(u1 - u);
		//�ҳ������䷽���Լ���Ӧ����ֵ  
		if (deltaTmp > deltaMax)
		{
			deltaMax = deltaTmp;
			threshold = nI;
		}
	}
	return threshold;
}

void getArrorNumBinary(Mat &src)
{
	//@@@ֱ��ͼ���⻯
	equalizeHist(src, src);
	int thre = daJinThreshold(src);
#ifdef MLStep20
	printf("thre=%d\n", thre);
	imshow("equalizeHist", src);
#endif // MLStep20
	//@@@����ֵ��
	medianBlur(src, src, 5);
	//cout << "thre:" << thre << endl;
	threshold(src, src, thre, 255, THRESH_BINARY);
	//adaptiveThreshold(src, src, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 11, 2);
	//@@@ȥ���������
}

int GetMeanThreshold(Mat grayImg)//�Ҷ�ƽ��ֵֵ��
{
	int width = grayImg.cols;
	int height = grayImg.rows;
	int x = 0, y = 0;
	int pixelCount[256];
	float pixelPro[256];
	int pixelSum = width * height;

	//��ʼ��  
	for (uint nI = 0; nI < 256; nI++)
	{
		pixelCount[nI] = 0;
		pixelPro[nI] = 0;
	}

	//ͳ�ƻҶȼ���ÿ������������ͼ���еĸ���  
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
	int ht = box.size.height * 2;

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
	//cout << 11 << endl;
	imageROI = frame(Rect(x, y, wh, ht));
	//cout << 22 << endl;
	//k++;

	resize(imageROI, imageROI, cv::Size(48, 48), (0, 0), (0, 0), cv::INTER_LINEAR);  //��ͼƬ����Ϊ��ͬ�Ĵ�С
	//getArrorNumBinary(imageROI);
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
				nW = sqrt((vEllipse[i].center.x - vEllipse[j].center.x) * (vEllipse[i].center.x - vEllipse[j].center.x) + (vEllipse[i].center.y - vEllipse[j].center.y) * (vEllipse[i].center.y - vEllipse[j].center.y)); //װ�׵Ŀ�ȵ�������LED������ת������������ľ���

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

				if (armor.rect.angle < 90)
				{
					armor.rect.angle += 180;
				}

				if (armor.rect.size.height * 6 > armor.rect.size.width && armor.rect.size.height < armor.rect.size.width )//&& suitble(vEllipse[i], vEllipse[j])
				{

#ifdef SvmPredict
					int finalnum;
					finalnum = get_num(armor.rect, svm, frame);
					if (finalnum)
					{
						armor.num = finalnum;
						vRlt.push_back(armor);
					}
#else
					vRlt.push_back(armor);
#endif
				}
			}
		}
	}
	return vRlt;
}


void ArmorRecognition::b_track_armor(Mat srcimage, Mat frame, Result &r, FileStorage fs, Ptr<SVM> svm)
{
	ofstream outfile;//�����ļ�
	outfile.open("log.txt");

	if (srcimage.data)
	{
		outfile << "��ʼ����..." << endl;
		Mat mask;
		Size imgSize;
		imgSize = frame.size();

		Mat rimg = Mat(imgSize, CV_8UC1);
		Mat gimg = Mat(imgSize, CV_8UC1);
		Mat bimg = Mat(imgSize, CV_8UC1);

		Mat diff = Mat(imgSize, CV_8UC1);
		Mat th = Mat(imgSize, CV_8UC1);
		Mat th1 = Mat(imgSize, CV_8UC1);
		Mat rlt = Mat(imgSize, CV_8UC1);

		vector<vector<Point>> contour;
		RotatedRect s;


		vector<RotatedRect> vArmor;
		vector<my_rect> vRlt;
		vector<RotatedRect> vRlt_svm;
		vector<RotatedRect> vEllipse;
		my_rect finalarmor;

		Mat gray = Mat(imgSize, CV_8UC1);
		cvtColor(frame, gray, COLOR_BGR2GRAY);//225
		outfile << "��ȡgray�ɹ�..." << endl;

		Mat channels[3];
		split(frame, channels);

		bimg = channels[0];
		gimg = channels[1];
		rimg = channels[2];

		if (ch == 0)
		{
			diff = bimg - rimg;
		}
		else if (ch == 1)
		{
			diff = rimg - bimg;
		}
		outfile << "��ȡdiff�ɹ�..." << endl;


		dilate(diff, diff, Mat(), Point(-1, -1), 3);
		threshold(diff, th1, color_th, 255, THRESH_BINARY);
		outfile << "��ȡcolor_th�ɹ�..." << endl;
		threshold(gray, th, gray_th, 255, THRESH_BINARY);
		outfile << "��ȡgray_th�ɹ�..." << endl;

#ifdef gray_th_img
		imshow("gray_th", th);
#endif

#ifdef color_th_img
		imshow("color_th", th1);
#endif

		bitwise_and(th, th1, rlt);
		outfile << "��ȡrlt�ɹ�..." << endl;

#ifdef rltimg
		imshow("rlt", rlt);
#endif


		findContours(rlt, contour, RETR_EXTERNAL, CHAIN_APPROX_NONE);
		outfile << "�ҵ�����ʼ..." << endl;
		for (int k = 0; k < contour.size(); k++)
		{
			double area = contourArea(contour[k]);
			if (area < min_area || area > max_area)//
			{
				continue;
			}
			double length = arcLength(contour[k], true); // �����ܳ�
			if (length > 3 && length < 2000)
			{
				if (contour[k].size() > 6)
				{
					bool Flag = true;
					s = fitEllipse(contour[k]);
					adjustRec(s, ANGLE_TO_UP);
					if (s.size.height > s.size.width)//isValidLightBlob(contour[k], s)
					{
						draw_light(s, frame);
						vEllipse.push_back(s);
					}
				}
			}
		}
		outfile << "�ҵ�������..." << endl;


		outfile << "��װ�װ忪ʼ..." << endl;
		vRlt = armorDetect(vEllipse, frame, svm);
		outfile << "��װ�װ����..." << endl;

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

			drawBox(finalarmor.rect, frame);
#ifdef SvmPredict
			putText(frame, to_string(finalarmor.num), Point(finalarmor.rect.center.x - 5, finalarmor.rect.center.y + 5), 5, 1.2, Scalar(0, 255, 255), 2);
#endif			
			//cout << finalnum << endl;

			ArrorAttitudeAlgorithm angle;
			double yaw = 0, pitch = 0;
			outfile << "�������ݿ�ʼ..." << endl;
			r.d_distance = angle.angleSover(srcimage, finalarmor.rect, yaw, pitch);
			r.d_pitch = pitch;
			r.d_yaw = yaw;
			outfile << "�������ݽ���..." << endl;
			
			r.r_x = finalarmor.rect.center.x - finalarmor.rect.size.width;
			r.r_y = finalarmor.rect.center.y - finalarmor.rect.size.height;
			r.r_width = finalarmor.rect.size.width * 2;
			r.r_height = finalarmor.rect.size.height * 2;

			if (r.r_x < 0)
			{
				r.r_x = 0;
			}
			if (r.r_y < 0)
			{
				r.r_y = 0;
			}
			if (r.r_x + r.r_width > MATWIDTH)
			{
				r.r_width = MATWIDTH - r.r_x;
			}
			if (r.r_y + r.r_height > MATHEIGHT)
			{
				r.r_height = MATHEIGHT - r.r_y;
			}
			r.r_flag == 1;
		}
		else
		{
			r.r_flag == 0;
		}
		
/* #ifdef frameimg
			//pthread_mutex_lock(&image);
			imshow("frame", frame);
			waitKey(1);
			//pthread_mutex_unlock(&image);
#endif */			

		vEllipse.clear();
		vRlt.clear();
		vArmor.clear();
	}
	
	outfile << "�������..." << endl;
	outfile.close();
}



