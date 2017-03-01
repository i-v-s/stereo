// stereo.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <rectifier.h>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>

int main()
{
	Rectifier rct("log2.txt");
	cv::VideoCapture vcl(1), vcr(0);
	int nod = 256;
	auto sm = cv::StereoBM::create(nod, 21);
	//auto sm = cv::StereoSGBM::create(5, nod, 15);
	do {
		cv::Mat left, right, left_g, right_g, left_r, right_r, disp, disp8;
		vcl >> left;
		vcr >> right;
		cv::cvtColor(left, left_g, CV_RGB2GRAY);
		rct.rectify(left_g, left_r, 0);
		cv::cvtColor(right, right_g, CV_RGB2GRAY);
		rct.rectify(right_g, right_r, 1);
		cv::imshow("Left", left_r);
		cv::imshow("Right", right_r);

		sm->compute(left_r, right_r, disp);
		disp.convertTo(disp8, CV_8U, 255 / (nod * 16.));
		//cv::
		cv::imshow("Disparity", disp8);
	} while (cv::waitKey(1) != 27);

    return 0;
}

