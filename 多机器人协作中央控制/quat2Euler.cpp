#include "stdafx.h" //这个文件一定要放前面 坑死我了！！！！
#include "quat2Euler.h"



namespace zfun
{
	float numFormat(float num, int rz)
	{
		//非四舍五入
		float e = pow(10, rz);
		return floor(num*e) / e;
	}

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
}