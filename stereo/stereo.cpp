// stereo.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <rectifier.h>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <Windows.h>
#include <ctime>

int main()
{
	Rectifier rct("log2s.txt");
	cv::VideoCapture vcl(0), vcr(1);
	int nod = 128;
	int idx = 0;
	std::string wdir = "";
	auto sm = cv::StereoBM::create(nod, 21);
	//auto sm = cv::StereoSGBM::create(5, nod, 15);
	int k;
	do {
		cv::Mat left, right, left_g, right_g, left_r, right_r, disp, disp8;
		vcl >> left;
		vcr >> right;
		cv::cvtColor(left, left_g, CV_RGB2GRAY);
		rct.rectify(left_g, left_r, 0);
		//cv::equalizeHist(left_r, left_r);

		cv::cvtColor(right, right_g, CV_RGB2GRAY);
		cv::flip(right_g, right_g, -1);
		rct.rectify(right_g, right_r, 1);
		//cv::equalizeHist(right_r, right_r);

		if (wdir != "") {
			char buf[32];
			sprintf_s(buf, "%04d", idx++);
			//cv::imwrite()

		}
		cv::imshow("Left", left_r);
		cv::imshow("Right", right_r);

		sm->compute(left_r, right_r, disp);
		disp.convertTo(disp8, CV_8U, 255 / (nod * 16.));
		//cv::
		cv::imshow("Disparity", disp8);
		k = cv::waitKey(1);
		switch (k) {
		case 'w': if (wdir == "") {
			idx = 0;
			time_t rawtime;
			struct tm timeinfo;
			char buffer[128];

			time(&rawtime);
			localtime_s(&timeinfo, &rawtime);

			strftime(buffer, 128, "d:/wires/%Y-%m-%d %I:%M:%S/", &timeinfo);
			if (CreateDirectoryA(buffer, NULL) || ERROR_ALREADY_EXISTS == GetLastError())
				wdir = buffer;
			break;
		}
		case 's': 
			wdir = "";
			break;
		}
	} while (k != 27);

    return 0;
}

