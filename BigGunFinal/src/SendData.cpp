#include "SendData.h"

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

void SendData::Send(double yaw, double pitch, double distance, int number)
{

	//printf("\t[Yaw=%f\tPitch=%f\tdist=%f\tarrorNum=%d]\n", yaw, pitch, distance, number);

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