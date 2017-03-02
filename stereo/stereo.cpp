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
	Rectifier rct("5m2.txt");
	cv::VideoCapture vcl(0), vcr(1);
	int nod = 96;
	int idx = 0;
	std::string wdir = "";
	//std::string rdir = "d:/wires/2017-03-02 09.12.49/";
	std::string rdir = "d:/wires/2017-03-02 09.14.25/";
	auto sm = cv::StereoBM::create(nod, 15);
	//auto sm = cv::StereoSGBM::create(5, nod, 15);
	int k;
	do {
		cv::Mat left, right, left_g, right_g, left_r, right_r, disp, disp8;
		if (rdir == "") {
			vcl >> left;
			vcr >> right;
			cv::cvtColor(left, left_g, CV_RGB2GRAY);
			rct.rectify(left_g, left_r, 0);
			//cv::equalizeHist(left_r, left_r);

			cv::cvtColor(right, right_g, CV_RGB2GRAY);
			//cv::flip(right_g, right_g, -1);
			rct.rectify(right_g, right_r, 1);
			//cv::equalizeHist(right_r, right_r);

			if (wdir != "") {
				char buf[32];
				sprintf_s(buf, "%04d", idx++);
				cv::imwrite(wdir + buf + "l.png", left_r);
				cv::imwrite(wdir + buf + "r.png", right_r);
			}
		}
		else {
			char buf[32];
			sprintf_s(buf, "%04d", idx++);
			left_r = cv::imread(rdir + buf + "l.png");
			right_r = cv::imread(rdir + buf + "r.png");
			if (left_r.empty() || right_r.empty()) { idx = 0; continue; }
			cv::cvtColor(left_r, left_r, CV_RGB2GRAY);
			cv::cvtColor(right_r, right_r, CV_RGB2GRAY);
		}

		cv::imshow("Left", left_r);
		cv::imshow("Right", right_r);

		sm->compute(left_r, right_r, disp);
		disp.convertTo(disp8, CV_8U, 255 / (nod * 16.));

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

			strftime(buffer, 128, "d:/wires/%Y-%m-%d %I.%M.%S/", &timeinfo);
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

