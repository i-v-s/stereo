// stereo.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <rectifier.h>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <Windows.h>
#include <ctime>

int kkk = 0;

float calcLine(const cv::Mat &src, int min, int max, float *pa, float *pb)
{
	float sx = 0, sx2 = 0, sy = 0, sxy = 0;
	int n = 0;
	const uint8_t *p = src.data;
	for (int x = 0; x < src.rows; x++) 
		for (int y = 0; y < src.cols; y++, p++) {
			uint8_t v = *p;
			if (v >= min && v <= max) {
				sx += x;
				sx2 += x * x;
				sy += y;
				sxy += x * y;
				n++;
			}
		}
	float d = sx2 * n - sx * sx;
	if (fabs(d) < 1E-10) return NAN;
	float b = (sx2 * sy - sx * sxy) / d;
	float a = (n * sxy - sx * sy) / d;
	float sse = 0;
	p = src.data;
	for (int x = 0; x < src.rows; x++)
		for (int y = 0; y < src.cols; y++, p++) {
			uint8_t v = *p;
			if (v >= min && v <= max) {
				float e = a * x + b - y;
				sse += e * e;
			}
		}

	*pa = a;
	*pb = b;
	return sqrt(sse / n);
}

void drawHist(const cv::Mat &disp)
{
	cv::Mat hist;
	int histSize = 256;
	float range[] = { 0, 256 };
	const float* histRange = { range };
	cv::calcHist(&disp, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);
	assert(hist.type() == CV_32F);
	const float *ph = (const float *) hist.data;

	int hist_w = 512; int hist_h = 600;
	int bin_w = cvRound((double)hist_w / histSize);
	cv::Mat histImage(hist_h, hist_w + 10, CV_8UC3, cv::Scalar(0, 0, 0));
	for (int i = 1; i < histSize; i++)
	{
		line(histImage,
			cv::Point(bin_w * (i - 1), hist_h - cvRound(hist.at<float>(i - 1))),
			cv::Point(bin_w * (i), hist_h - cvRound(hist.at<float>(i))),
			cv::Scalar(255, 0, 0), 2, 8, 0);
	}


	int k = 0;
	float sum = 0;
	int on = 0, max, min;
	bool o = false;
	for (const float *p = ph + 255; p >= ph; p--)
	{
		float v = *p;
		if (v > 100 && on < 5) {
			on += int(v / 100);
			if (on >= 5 && !o) {
				o = true;
				int t = p - ph + 5;
				cv::line(histImage, cv::Point(bin_w * t, 0), cv::Point(bin_w * t, hist_h), cv::Scalar(0, 0, 255));
				max = t;
			}
		}
		if (on > 0 && v < 30) {
			on--;
			if (on == 0 && o) {
				o = false;
				int t = p - ph;
				cv::line(histImage, cv::Point(bin_w * t, 0), cv::Point(bin_w * t, hist_h), cv::Scalar(0, 255, 0));
				min = t;
				break;
			}
		}
	}

	//if (kkk) k = kkk / 2;

	cv::imshow("Histogram", histImage);
	cv::Mat fdisp;
	cv::cvtColor(disp, fdisp, CV_GRAY2RGB);
	for (uint8_t *p = fdisp.data; p < fdisp.dataend; p += 3) {
		if (p[0] < min || p[0] > max) {
			p[1] = 0;
			p[2] = 0;
		}
	}

	float a, b;
	float sse = calcLine(disp, min, max, &a, &b);
	if (!isnan(sse)) {
		int h = disp.rows;
		cv::line(fdisp, cv::Point(int(b - sse), 0), cv::Point(int(b + h * a - sse), h), cv::Scalar(0, 0, 255));
		cv::line(fdisp, cv::Point(int(b + sse), 0), cv::Point(int(b + h * a + sse), h), cv::Scalar(0, 0, 255));
	}

	cv::imshow("FDisp", fdisp);
}

void cbf(int event, int x, int y, int flags, void* userdata)
{
	if (event == cv::EVENT_LBUTTONDOWN)
	{
		kkk = x;
	}

}

void toGray(const cv::Mat &src, cv::Mat &out)
{
	cv::Size size = src.size();
	cv::Mat dst(size, CV_8U);
	const uint8_t *s = src.data;
	uint8_t *d = dst.data;
	for (int y = size.height, k = 0; --y >= 0; ) {
		if (++k >= 3) k = 0;
		for (int x = size.width; --x >= 0; s += 3, d++) *d = s[k];
	}
	out = dst;
}

int main()
{
	Rectifier rct("5m2.txt");
	cv::VideoCapture vcl(0), vcr(1);
	int nod = 80;
	int idx = 0;
	bool pause = false;
	cv::namedWindow("Histogram");
	cv::setMouseCallback("Histogram", cbf, NULL);

	std::string wdir = "";
	std::string rdir = "d:/wires/2017-03-02 09.12.49/";
	//std::string rdir = "d:/wires/2017-03-02 09.14.25/";
	auto sm = cv::StereoBM::create(nod, 15);
	//auto sm = cv::StereoSGBM::create(5, nod, 15);
	//sm->setSmallerBlockSize(10);
	int k;
	cv::Mat left_r, right_r;
	do {
		cv::Mat left, right, left_g, right_g, disp, disp8;
		if (!pause) {
			if (rdir == "") {
				vcl >> left;
				vcr >> right;
				//cv::cvtColor(left, left_g, CV_RGB2GRAY);
				toGray(left, left_g);
				rct.rectify(left_g, left_r, 0);
				//cv::equalizeHist(left_r, left_r);

				//cv::cvtColor(right, right_g, CV_RGB2GRAY);
				toGray(right, right_g);
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
				toGray(left_r, left_r);
				toGray(right_r, right_r);

				//cv::cvtColor(left_r, left_r, CV_RGB2GRAY);
				//cv::cvtColor(right_r, right_r, CV_RGB2GRAY);
			}
		}

		cv::imshow("Left", left_r);
		cv::imshow("Right", right_r);

		sm->compute(left_r, right_r, disp);
		cv::filterSpeckles(disp, 0, 80, 64);

		disp.convertTo(disp8, CV_8U, 255 / (nod * 16.));

		drawHist(disp8);

		//cv::imshow("Disparity", disp8);
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
		case 'p':
			pause = !pause;
			break;
		}
	} while (k != 27);

    return 0;
}

