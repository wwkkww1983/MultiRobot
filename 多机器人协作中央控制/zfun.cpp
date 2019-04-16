#include "stdafx.h"
#include "zfun.h"



namespace zfun
{
	float numFormat(float num, int rz)
	{
		//非四舍五入
		float e = pow(10, rz);
		return floor(num*e) / e;
	}
	//四元数转欧拉角
	Eigen::Vector3d Quaterniond2Euler(const double x, const double y, const double z, const double w)
	{
		Eigen::Quaterniond q;
		q.x() = x;
		q.y() = y;
		q.z() = z;
		q.w() = w;
		Eigen::Vector3d euler = q.toRotationMatrix().eulerAngles(2, 1, 0);
		return euler;
	}
	//查找vector元素
	int findVecterElm(std::vector<int> vec, int elm)
	{
		int ret;
		std::vector<int>::iterator iter;
		iter = find(vec.begin(), vec.end(), elm);
		if (iter != vec.end())
		{
			ret = &*iter - &vec[0];
		}
		else
		{
			ret = -1;
		}
		return ret;
	}
	int findVecterElm(std::vector<std::string> vec, std::string elm)
	{
		int ret;
		std::vector<std::string>::iterator iter;
		iter = find(vec.begin(), vec.end(), elm);
		if (iter != vec.end())
		{
			ret = &*iter - &vec[0];
		}
		else
		{
			ret = -1;
		}
		return ret;
	}
	int findVecterElm(std::vector<uint8_t> vec, uint8_t elm)
	{
		int ret;
		std::vector<std::uint8_t>::iterator iter;
		iter = find(vec.begin(), vec.end(), elm);
		if (iter != vec.end())
		{
			ret = &*iter - &vec[0];
		}
		else
		{
			ret = -1;
		}
		return ret;
	}
	


	double distancePoint(cv::Vec3d p1, cv::Vec3d p2)
	{
		double dx = p1[0] - p2[0];
		double dy = p1[1] - p2[1];
		double dz = p1[2] - p2[2];

		return abs(dx)+ abs(dy)+ abs(dz);
	}

}