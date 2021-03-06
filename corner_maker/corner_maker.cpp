#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <queue>

#define PI 3.14159265358979

typedef struct Point { int x;  int y; } _point;


int main(int argc, char* argv[])
{
	std::string inputfilename, configfilename;

	
	if (argc != 3) {
		std::cerr << "Usage: corner_maker [filename] [config]" << std::endl;
		return -1;
	}

	inputfilename = argv[1];
	configfilename = argv[2];

	std::ifstream ifs(configfilename);
	
	if (!ifs.is_open()) {
		std::cerr << "Error: Couldn't read config: " << argv[2] << std::endl;
		return -1;
	}
	
	double f = 0.0;
	unsigned int side = 0;
	std::string pattern;
	int drawable[5][5] = {0};

	ifs >> f >> side >> pattern;

	if (f <= 0.0) {
		std::cerr << "Error: Invalid focal length: " << f  << "mm > 0.0mm." << std::endl;
		return -1;
	}
	else if (side <= 0) {
		std::cerr << "Error: Invalid side pixels: " << side << " > 0." << std::endl;
		return -1;
	}
	else if (pattern != "full-size" && pattern != "apsc") {
		std::cerr << "Error: Invalid angle pattern " << pattern  << " = full-size or apsc."<< std::endl;
		return -1;
	}

	if (pattern == "full-size") {
		for (int i = 0; i < 5; ++i) {
			for (int j = 0; j < 5; ++j) {
				ifs >> drawable[i][j];
				if (ifs.eof() && i != 4 && j != 4) {
					std::cerr << "Error: Lack of drawing squares." << std::endl;
					std::cerr << "	 if use full-size pattern, describe in your config as follows" << std::endl;
					std::cerr << "	1 1 1 1 1" << std::endl;
					std::cerr << "	1 1 1 1 1" << std::endl;
					std::cerr << "	1 1 1 1 1" << std::endl;
					std::cerr << "	1 1 1 1 1" << std::endl;
					std::cerr << "	1 1 1 1 1" << std::endl;
					return -1;
				}
			}
		}
	}
	else {
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				ifs >> drawable[i][j];
				if (ifs.eof() && i != 2 && j != 2) {
					std::cerr << "Error: Lack of drawing squares." << std::endl;
					std::cerr << "	 if use apsc pattern, describe in your config as follows" << std::endl;
					std::cerr << "	1 1 1" << std::endl;
					std::cerr << "	1 1 1" << std::endl;
					std::cerr << "	1 1 1" << std::endl;
					return -1;
				}
			}
		}
	}
	
	cv::Mat image = cv::imread(inputfilename, 2 | 4);
	if (image.data == NULL) {
		std::cerr << "Error: Couldn't read image file " << inputfilename << std::endl;
		return -1;
	}
	
	int xsize = image.cols, ysize = image.rows;
	int newxsize, newysize;

	if (xsize < ysize) {
		newxsize = ysize;
		newysize = xsize;
	}
	else {
		newxsize = xsize;
		newysize = ysize;
	}


	cv::Mat data = cv::Mat::zeros(newysize, newxsize, CV_16UC3);
	
	if (xsize < ysize) {
		for (int j = 0; j < ysize; ++j) {
			for (int i = 0; i < xsize; ++i) {
				data.at<cv::Vec3w>(xsize - i - 1, j) = image.at<cv::Vec3w>(j, i);
			}
		}
	}
	else {
		for (int j = 0; j < ysize; ++j) {
			for (int i = 0; i < xsize; ++i) {
				data.at<cv::Vec3w>(j, i) = image.at<cv::Vec3w>(j, i);
			}
		}
	}

	image.release();
	xsize = newxsize;
	ysize = newysize;

	double horizontal = (pattern == "full-size") ? 2.0 * (atan(36.0  / (2.0 * f)) * 180 / PI)
		: 2.0 * (atan(23.4 / (2.0 * f)) * 180 / PI);
	std::cout << horizontal * 3600.0 / (double)xsize << " arcsec / pixel" << std::endl;

	cv::Mat output = cv::Mat::zeros(side * ((pattern == "full-size") ? 5 : 3), side * ((pattern == "full-size") ? 5 : 3), CV_16UC3);

	_point center = { (xsize / 2), (ysize / 2) };

	std::queue<_point> start_points;

	if (pattern == "full-size") {
		start_points.push(Point{ 0, 0 });
		start_points.push(Point{ (int)(xsize * 6.3 / 36), 0 });
		start_points.push(Point{ (int)(center.x - side / 2.0), 0 });
		start_points.push(Point{ (int)(xsize * 29.7 / 36.0 - side), 0 });
		start_points.push(Point{ xsize - (int)side, 0 });

		start_points.push(Point{ 0, (int)(ysize * 3.65 / 24.0)});
		start_points.push(Point{ (int)(xsize * 6.3 / 36), (int)(ysize * 3.65 / 24.0) });
		start_points.push(Point{ (int)(center.x - side / 2.0), (int)(ysize * 3.65 / 24.0) });
		start_points.push(Point{ (int)(xsize * 29.7 / 36.0 - side), (int)(ysize * 3.65 / 24.0) });
		start_points.push(Point{ xsize - (int)side, (int)(ysize * 3.65 / 24.0) });

		start_points.push(Point{ 0,(int)(center.y - side / 2.0) });
		start_points.push(Point{ (int)(xsize * 6.3 / 36), (int)(center.y - side / 2.0) });
		start_points.push(Point{ (int)(center.x - side / 2.0), (int)(center.y - side / 2.0) });
		start_points.push(Point{ (int)(xsize * 29.7 / 36.0 - side), (int)(center.y - side / 2.0) });
		start_points.push(Point{ xsize - (int)side, (int)(center.y - side / 2.0) });

		start_points.push(Point{ 0, (int)(ysize * 20.35 / 24.0 - side) });
		start_points.push(Point{ (int)(xsize * 6.3 / 36), (int)(ysize * 20.35 / 24.0 - side) });
		start_points.push(Point{ (int)(center.x - side / 2.0), (int)(ysize * 20.35 / 24.0 - side) });
		start_points.push(Point{ (int)(xsize * 29.7 / 36.0 - side), (int)(ysize * 20.35 / 24.0 - side) });
		start_points.push(Point{ xsize - (int)side, (int)(ysize * 20.35 / 24.0 - side) });

		start_points.push(Point{ 0, ysize - (int)side });
		start_points.push(Point{ (int)(xsize * 6.3 / 36), ysize - (int)side });
		start_points.push(Point{ (int)(center.x - side / 2.0), ysize - (int)side });
		start_points.push(Point{ (int)(xsize * 29.7 / 36.0 - side), ysize - (int)side });
		start_points.push(Point{ xsize - (int)side, ysize - (int)side });
	}
	else {
		start_points.push(Point{ 0, 0 });
		start_points.push(Point{ (int)(center.x - side / 2.0), 0});
		start_points.push(Point{ xsize - (int)side, 0});

		start_points.push(Point{ 0, (int)(center.y - side / 2.0)});
		start_points.push(Point{ (int)(center.x - side / 2.0), (int)(center.y - side / 2.0) });
		start_points.push(Point{ xsize - (int)side, (int)(center.y - side / 2.0) });

		start_points.push(Point{0, ysize - (int)side });
		start_points.push(Point{ (int)(center.x - side / 2.0), ysize - (int)side });
		start_points.push(Point{ xsize - (int)side, ysize - (int)side });
	}

	for (int c = 0; c < (pattern == "full-size" ? 5 : 3) ; ++c) {
		for (int r = 0; r < (pattern == "full-size" ? 5 : 3); ++r) {
			_point start = start_points.front(); start_points.pop();
			if (drawable[c][r] == 1) {
				for (int j = 0; j < (int)side; ++j) {
					for (int i = 0; i < (int)side; ++i) {
						output.at<cv::Vec3w>(c * side + j, r * side + i) = data.at<cv::Vec3w>(start.y + j, start.x + i);
					}
				}
			}
			else {
				for (int j = 0; j < (int)side; ++j) {
					for (int i = 0; i < (int)side; ++i) {
						output.at<cv::Vec3w>(c * side + j, r * side + i) = { 0xffff, 0xffff, 0xffff };
					}
				}
			}
		}
	}

	cv::imwrite("output.tif", output);
    return 0;
}

