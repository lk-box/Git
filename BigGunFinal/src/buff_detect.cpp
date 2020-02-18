#include "buff_detect.h"

#define PI 3.1415927
#define MLR  //ʹ�û���ѧϰ��R

//��������ʶ��������
int buff_detect::BuffDetectTask(Mat& image, Mat& dst, float& angle, struct logo& center_R)//dst��ɫͨ�����   imageԭͼ��  angle_num�õ��ǶȵĴ���
{
	vector<RotatedRect> target_armor;//�洢Ŀ��
	vector<RotatedRect> target_R;//�м��R
	vector<RotatedRect> final_R;

	//�Զ�ֵ��ͼ����ʶ�����������õ�������������R
	
	buff_detection(image, dst, target_R, target_armor);

	if (target_armor.empty())
	{
		return 0;
	}

	if (target_R.empty())
	{
		return 0;
	}
#ifdef MLR	//ʹ�û���ѧϰ������ѡĿ�꼯���������ٽ��л���ѧϰ��ʶ��

	machineLearning(image, target_R, final_R, svm1);

#endif // !MACHINELEARNING


#ifndef MLR	//��ʹ�û���ѧϰ��ֱ�Ӹ��������ҳ�R

	final_R.push_back(target_R[0]);

#endif // !MACHINELEARNING
	if (final_R.empty())
	{
		return 0;
	}
	

	center_R.r = sqrt(powf((target_armor[0].center.x - final_R[0].center.x), 2) + powf((target_armor[0].center.y - final_R[0].center.y), 2));

	center_R.x = final_R[0].center.x;
	center_R.y = final_R[0].center.y;

	//ȷ��Ŀ����Ҷ�ĽǶ�
	float delta_x = abs(target_armor[0].center.x - final_R[0].center.x);
	float delta_y = abs(target_armor[0].center.y - final_R[0].center.y);
	float theta = atan(delta_x / delta_y) / PI * 180;

	if (delta_x != 0 || delta_y != 0)
	{
		if (target_armor[0].center.x > final_R[0].center.x && target_armor[0].center.y < final_R[0].center.y)//��һ����
		{
			angle = theta;
		}
		else if (target_armor[0].center.x > final_R[0].center.x && target_armor[0].center.y > final_R[0].center.y)//�ڶ�����
		{
			angle = 180.0 - theta;
		}
		else if (target_armor[0].center.x < final_R[0].center.x && target_armor[0].center.y > final_R[0].center.y)//��������
		{
			angle = 180.0 + theta;
		}
		else if (target_armor[0].center.x < final_R[0].center.x && target_armor[0].center.y < final_R[0].center.y)//��������
		{
			angle = 360.0 - theta;
		}
	}
	else
	{
		if (delta_x == 0 && target_armor[0].center.y < final_R[0].center.y)//y�ϰ���
		{
			angle = 0.0;
		}
		else if (delta_x == 0 && target_armor[0].center.y > final_R[0].center.y)//y�°���
		{
			angle = 180.0;
		}
		else if (delta_y == 0 && target_armor[0].center.x < final_R[0].center.x)//x�����
		{
			angle = 270.0;
		}
		else if (delta_y == 0 && target_armor[0].center.x > final_R[0].center.x)//x�Ұ���
		{
			angle = 90.0;
		}
	}

	drawtarget(target_armor, image);//����Ƶ�����Ͽ��Ŀ��
		
	box.drawBoxYellow(final_R[0], image);
	

	return target_armor.size();
}


void buff_detect::color_buff(Mat& image, Mat& dst, int threshValue, int color)
{
	for (int i = 0; i < image.rows; i++)
	{
		uchar* data_b = image.ptr<uchar>(i);
		uchar* data_g = image.ptr<uchar>(i) + 1;
		uchar* data_r = image.ptr<uchar>(i) + 2;

		for (int j = 0; j < image.cols; j++)
		{
			if (color == 0)
			{
				dst.ptr<uchar>(i)[j] = saturate_cast<uchar>(*data_b);
				if (dst.ptr<uchar>(i)[j] < threshValue || (*data_b > 150 && *data_g > 150 && *data_r > 150))
				{
					dst.ptr<uchar>(i)[j] = 0;
				}
				else
				{
					dst.ptr<uchar>(i)[j] = 255;
				}

				data_g = data_g + 3;
			}
			else
			{
				dst.ptr<uchar>(i)[j] = saturate_cast<uchar>(*data_r - *data_b);

				if ((*data_r < 120) || dst.ptr<uchar>(i)[j] < threshValue)
				{
					dst.ptr<uchar>(i)[j] = 0;
				}
				else
				{
					dst.ptr<uchar>(i)[j] = 255;
				}
			}

			data_b = data_b + 3;
			data_r = data_r + 3;
		}
	}
}

void buff_detect::buff_detection(Mat& img, Mat& dst, vector<RotatedRect>& target_R, vector<RotatedRect>& target_armor)
{

#ifndef MLF

	RotatedRect father;
	RotatedRect son;

#endif // !MLF


#ifdef MLF

	vector<RotatedRect> father;
	vector<RotatedRect> blade;
	vector<RotatedRect> son;

#endif //MLF
	
	vector<vector<Point>> lightContours;
	vector<Vec4i> hierarchy;
	vector<RotatedRect> rect;

	Mat elementDilate = getStructuringElement(MORPH_ELLIPSE, Size(7, 7));
	Mat elementErode = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));

	medianBlur(dst, dst, 3);

	dilate(dst, dst, elementDilate);
	erode(dst, dst, elementErode);

	//Ѱ��������������
	findContours(dst, lightContours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);

	for (int i = 0; i < lightContours.size(); i++)
	{
		if (lightContours[i].size() > 6)
		{
			rect.push_back(fitEllipse(lightContours[i]));
		}
	}

	if (rect.size() == 0)
	{
		return;
	}

	for (int i = 0; i < rect.size(); i++)
	{
		RotatedRect light;   //������ת����

		light = rect[i];//����Բ��������õ���ת����

		//��Ե����
		int x = 0, y = 0;
		x = abs(light.center.x);
		y = abs(light.center.y);
		if (x + light.size.width / 2 > img.cols || y + light.size.height / 2 > img.rows ||
			x - light.size.width / 2 < 0 || y - light.size.height / 2 < 0)
		{
			continue;
		}

#ifdef MLF
		//��Ҷû�и���������������
		if (hierarchy[i][3] == -1 && hierarchy[i][2] != -1)
		{
			float vilue = 0;

			vilue = (light.size.width > light.size.height) ? light.size.width / light.size.height : light.size.height / light.size.width;

			if (vilue > 1.5 && vilue < 3.5)
			{
				machineLearningF(img, light, blade, svm);
			}

			//�õ���Ҷ��ֱ�ӻ�ȡ������
			if (!blade.empty())
			{
				target_armor.push_back(rect[hierarchy[i][2]]);
				get_R = true;
				break;
			}
		}

#endif // MLF

#ifndef MLF

		//��Ҷû�и���������������
		if (hierarchy[i][3] == -1 && hierarchy[i][2] != -1)
		{
			float vilue = 0;

			vilue = (light.size.width > light.size.height) ? light.size.width / light.size.height : light.size.height / light.size.width;

			int son_num = hierarchy[i][2];//�������ı��

			if (vilue > 1.5 && vilue < 3.5)
			{
				//����������Ŀ��װ�װ� ������������
				if (hierarchy[son_num][0] == -1 && hierarchy[son_num][1] == -1)//Ŀ��û����������û��ͬ������
				{
					son = rect[hierarchy[i][2]];
					father = light;

					if (father.size.area() / son.size.area() > 7 && father.size.area() / son.size.area() < 15)
					{
						target_armor.push_back(son);//�õ�Ŀ��
						box.drawBoxBlue(light, img);//������Ҷ
						get_R = true;
						break;
					}
				}
			}
		}

#endif // !MLF
		
	}

#ifdef MLF
	if (get_R)
	{
		get_energy_center(img, blade[0], target_armor[0], rect, target_R);

		get_R = false;
	}
#endif // MLF

#ifndef MLF

	if (get_R)
	{
		get_energy_center(img, father, son, rect, target_R);

		get_R = false;
	}

#endif // !MLF
	
}

void buff_detect::drawtarget(vector<RotatedRect>& target, Mat& image)
{
	if (!target.empty() && target.size() < 2)//Ŀ��ĸ���Ҫ����1
	{
		for (int i = 0; i < target.size(); i++)
		{
			box.drawBoxGreen(target[i], image);
		}
	}
}

void buff_detect::get_energy_center(Mat image, RotatedRect father, RotatedRect son, vector<RotatedRect> rect, vector<RotatedRect>& target_R)
{
	float delta_x = son.center.x - father.center.x;
	float delta_y = son.center.y - father.center.y;
	float distance = father.size.width + father.size.height;
	float father_center_to_son_center = sqrt(powf(delta_x, 2) + powf(delta_y, 2));
	float Y = abs(delta_y * distance / father_center_to_son_center);
	float X = abs(delta_x * distance / father_center_to_son_center);
	float R_x = 0, R_y = 0;

	//ȷ��ROI��ת���ε�����
	if (delta_x != 0 && delta_y != 0)
	{
		if (son.center.x < father.center.x && son.center.y < father.center.y)//��������
		{
			R_x = son.center.x + X;
			R_y = son.center.y + Y;
		}
		else if (son.center.x < father.center.x && son.center.y > father.center.y)//��������
		{
			R_x = son.center.x + X;
			R_y = son.center.y - Y;
		}
		else if (son.center.x > father.center.x && son.center.y > father.center.y)//�ڶ�����
		{
			R_x = son.center.x - X;
			R_y = son.center.y - Y;
		}
		else if (son.center.x > father.center.x && son.center.y < father.center.y)//��һ����
		{
			R_x = son.center.x - X;
			R_y = son.center.y + Y;
		}
	}
	else
	{
		if (delta_x == 0 && delta_y > 0)//y������
		{
			R_x = son.center.x;
			R_y = son.center.y + distance;
		}
		else if (delta_x == 0 && delta_y < 0)//y������
		{
			R_x = son.center.x;
			R_y = son.center.y - distance;
		}
		else if (delta_x > 0 && delta_y == 0)//x������
		{
			R_x = son.center.x - distance;
			R_y = son.center.y;
		}
		else if (delta_x < 0 && delta_y == 0)//x������
		{
			R_x = son.center.x + distance;
			R_y = son.center.y;
		}
	}

	Point2f p[4];
	RotatedRect energy_ROI(Point2f(R_x, R_y), Size2f(distance * 2, distance * 2), 0);
	energy_ROI.points(p);

	//Rect energy_roi_rect = energy_ROI.boundingRect();

	//if (energy_roi_rect.tl().x > 0 && energy_roi_rect.tl().y > 0 && energy_roi_rect.br().x < image.cols && energy_roi_rect.br().y < image.rows)
	//{
	//	imshow("ROI", image(Rect(energy_roi_rect)));
	//}

	for (int i = 0; i < rect.size(); i++)
	{
		//R�ڸ�roi��
		if (rect[i].center.y > p[1].y && rect[i].center.x > p[1].x && rect[i].center.y < p[3].y &&
			rect[i].center.x < p[3].x)
		{
			float a = (rect[i].size.height > rect[i].size.width) ? rect[i].size.height / rect[i].size.width :
					rect[i].size.width / rect[i].size.height;

			if (a < 1.2 && a > 0.8)
			{

#ifdef MLR
			
				target_R.push_back(rect[i]);

#endif // MLR


#ifndef MLR

				if (son.size.area() / rect[i].size.area() > 1 && son.size.area() / rect[i].size.area() < 3)
				{
					target_R.push_back(rect[i]);
					break;
				}
			
#endif // !MLR
			}
		}
	}

	get_R = false;
}

void buff_detect::machineLearning(Mat image, vector<RotatedRect> target_R, vector<RotatedRect>& final_R, Ptr<SVM> svm1)
{
	Rect roi;
	Mat target;
	Mat target_t;
	int response = 0;
	for (int i = 0; i < target_R.size(); i++)
	{
		roi = target_R[i].boundingRect();

		//��Ե����
		if (roi.tl().x < 0 || roi.tl().y < 0 || roi.br().x > image.cols || roi.br().y > image.rows)
		{
			continue;
		}

		target = image(Rect(roi));

		cvtColor(target, target_t, COLOR_BGR2GRAY);
		resize(target_t, target_t, Size(svmWidth, svmHeight));

		//��ʼSVMԤ��
		Mat p = target_t.reshape(1, 1);
		p.convertTo(p, CV_32FC1);
		response = (int)svm1->predict(p);//��ʼ��ÿ��ͼƬ����Ԥ�⣬ͬʱ���з�����Ӧ�������ĳһ��ͼƬԤ���õ�����Ӧֻ��һ��

		if (response == 1)
		{
			final_R.push_back(target_R[i]);
			//char imagename[100];
			//sprintf_s(imagename, "%s%d.jpg", "./R/right/R_r_v", num);
			//resize(target_t, target_t, Size(48, 48));
			//imwrite(imagename, target_t);
			//num++;

			break;
		}
		//else
		//{
		//	char imagename[100];
		//	sprintf_s(imagename, "%s%d.jpg", "./R/false/R_f_x", num);
		//	resize(target_t, target_t, Size(48, 48));
		//	imwrite(imagename, target_t);
		//	num++;
		//}
	}
}
