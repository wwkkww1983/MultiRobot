//主要用来实现一些小型的 自定义函数
#pragma once

#include "opencv2/opencv.hpp"
#include "Eigen\Dense"
#include <math.h>
#include <iostream>
#include <vector>
#include <string>


namespace zfun
{
	float numFormat(float num, int rz);//保留小数点后多少位
	//四元数转欧拉角
	Eigen::Vector3d Quaterniond2Euler(const double x, const double y, const double z, const double w);

	//查找vector元素
	int findVecterElm(std::vector<int> vec, int elm);
	int findVecterElm(std::vector<std::string> vec, std::string elm);
	int findVecterElm(std::vector<uint8_t> vec, uint8_t elm);
	

	//两点之间的距离
	double distancePoint(cv::Vec3d p1, cv::Vec3d p2);
	

}
