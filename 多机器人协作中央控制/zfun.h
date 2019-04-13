//主要用来实现一些小型的 自定义函数
#pragma once
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
	

}
