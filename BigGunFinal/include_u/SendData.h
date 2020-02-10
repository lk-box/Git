#pragma once
#include "serialport.h"
#include <opencv2/opencv.hpp>
using namespace cv;
class SendData
{
public:

    void Send(double yaw, double pitch, double distance, int number);

};



