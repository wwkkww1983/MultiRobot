/*
IPClocation
说  明：一个用RTSP地址IP摄像头来进行定位的库，提供了定位接口
并且提供了一个消息类IPCobj，可以作为消息输出，存储了在场地上有多少
物体并且给出了他的位置。
制作人：邹智强
版  本：beta 0.3
更  改：
	1、删除了缓存队列模块类，以及其变量。
	2、修复setWorld返回总是为false的问题
	3、添加了delayTime变量，存储定位系统的延时时间。并且加入xml
	4、添加画坐标点的函数功能（张猛作）。还需要改善，加大可视化。
	5、为IPCmsg类添加了一个互斥锁，主要保护视频流不同时被多个线程访问。
	6、公有化UpdateXMLfile等函数。
*/

#pragma once

#include <opencv2\aruco.hpp>
#include "opencv2/opencv.hpp"
#include <time.h>
#include <vector>
#include <stdio.h>
#include <string>
#include <io.h>
#include "zfun.h"
#include <windows.h>


using namespace cv;







//定义一个物体类型，记录存储物体信息
class IPCobj
{
public:
	IPCobj();

	enum objclass
	{
		Robot = 1,     //机器人
		object = 2	 //需要抓取的目标
	};

	//参数
	objclass cls;//物体是什么类

	Vec3d coordinate3D; //物体的3D位置
	Vec3d direction3D;  //物体的3D方向
	
	uint8_t ID; //若物体时robot则他的ID是。

	uint8_t dimension; //物体维度 3则是在3D世界坐标系下，2则是在相机2D坐标系下，4则是3D世界坐标系加上时间参数

	Point2i coordinate2D; //物体的2D位置

	double time;//4D维度下才用

private:

};

//定义一个IPC类型，用于存储IPC信息
class IPCmsg
{
public:
	IPCmsg();
	IPCmsg(String addr);
	//参数
	String rtsp;  //这个IPC的rtsp地址
	Mat cameraMatrix; //相机内参
	Mat distCoeffs;  //相机畸变参数
	Mat RwMatrix, TwVec, RwMatrixI; //外参旋转矩阵和位移矩阵、以及旋转矩阵的逆矩阵
	VideoCapture cap;//如果 Open()成功 ，就可以访问这个流。
	HANDLE hMutexcap;//用于保护 cap流。不被同时多个线程调用。
	//IPC状态类型
	enum StatusEnum
	{
		CANNOT_OPEN = -1, //不能打开摄像头
		NORMAL = 1,
		NOSTAR=0,
	};
	
private:
	
	StatusEnum Status;//存储相机目前的状态

public: //输出函数接口
	/*
	@brief:打开这个相机，这样就可以用VideoCapture了
	@returns:错误正确
	*/
	bool Open();

	/*
	@brief:检查相机状态
	@returns:返回Status
	*/
	StatusEnum checkStatus();
	/*
	@brief:更新旋转矩阵的逆矩阵
	*/
	void updateRwMatrixI();

};


/*--------------------------------------------------------------
@brief：定义一个IPC定位类，核心接口类,通过图片中的ARtag码来进行定位
---------------------------------------------------------------*/
class IPClocation
{
public:
	IPClocation();
	//参数
	std::vector<IPCmsg> IPC; //所有IPC

	//用于多线程 视觉处理定位线程
	HANDLE hThread;
	DWORD ThreadID;
	//用于多线程 互斥锁
	HANDLE hMutex;
	//定位算法
	int Algorithm=0;//0:AR姿态估计定位   1:多相机交点定位
	//定位延时
	double delayTime = 0;

	

private:
	
	//用于建立世界坐标系的标定AR码ID和它的大小
	uint8_t worldAR_ID=11; //11号作为世界坐标系标定ARtag
	float worldAR_size= 0.1784;	   // 米为单位 0.5

	//AR码类型
	aruco::PREDEFINED_DICTIONARY_NAME ARtag_dict = aruco::DICT_6X6_250;
	//AR码分配类型以及size
	//将AR_ID分为2个区间，每个区间有3个值：ID取值域2个 ，AR码的大小size一个(单位mm)
	uint16_t AR_ID_distribList[2][3] = {
		{20,60,178},	//大号机器人的尺寸
		{80,150,100}	//小号机器人的尺寸
	};

	//存储相机参数
	String xmlfileName;

	//自定义函数
	/*
	@brief:通过AR的四个角点，计算出中心点
	@param  corner：输入4个角点
	*/
	Point2i calculateCentre(std::vector<Point2f> corner);

	/*
	@brief:画定位坐标系用的函数
	@author:张猛
	*/
	bool cmp(Point2d &s1, Point2d &s2);
	void drawArrow(cv::Mat& img, cv::Point2d pLocation, cv::Point2d pDirection, Point2d oPoint, int len, int alpha,
		cv::Scalar color, int thickness = 2, int lineType = 8);
	void drawCoorArrow(cv::Mat& img, cv::Point2d pLocation, cv::Point2d pDirection, Point2d oPoint, int len, int alpha,
		cv::Scalar color, int thickness = 2, int lineType = 8);


public://输出函数接口

	/*
	@brief:以一张来自IPC的图片来定位图中的AR码，返回世界3D坐标
	@param  img：输入图片用于检测
	@param  IPCindex：以哪个编号的IPC参数为准。
	@returns：物体序列，应该全是是robot类别，且为3D世界坐标
	@time：i7-6700k 单核 13.6ms
	*/
	std::vector<IPCobj> location(Mat img, int IPCindex);

	/*
	@brief:以一张来自IPC的图片来定位图中的AR码，返回世界3D坐标,同时返回一个绘制图片
	@param  img：输入图片用于检测
	@param  IPCindex：以哪个编号的IPC参数为准。
	@param  outimg：输出返回一个绘制图片
	@returns：物体序列，应该全是是robot类别，且为3D世界坐标
	@time：i7-6700k 单核 14.7ms
	*/
	std::vector<IPCobj> location(Mat img, int IPCindex,Mat &outimg);

	/*
	@brief:以IPCindex号IP相机拍摄到的画面来定位AR码，返回世界3D坐标
	@param  IPCindex：以哪个编号的IPC参数为准。
	@returns：物体序列，应该全是是robot类别，且为3D世界坐标
	*/
	std::vector<IPCobj> location(int IPCindex);

	/*
	@brief:以一张来自IPC的图片来定位图中的AR码，返回相机坐标
	@time：i7-6700k 单核 12.8ms
	*/
	std::vector<IPCobj> locationMat(Mat img, int IPCindex);

	/*
	*@brief：以图片，来进行添加IP相机，会对img中的图片进行标定。err为误差标准值
	@param  img:图片序列
	@param	board_size:棋盘的格子数
	@param	square_size:棋盘每一格子的宽度
	@param	rtsp:添加相机的RTSP地址
	@param	err_th:标定重映射误差的阈值，允许的最低值
	@returns：错误还是正确
	*/
	bool AddIPC(std::vector<Mat> img, cv::Size board_size, cv::Size square_size, cv::String rtsp, double err_th = 1);

	/*
	*@brief：以图片文件夹，来进行添加IP相机，会对文件夹中的JPG图片进行标定。err为误差标准值
	@param  filedir:图片文件夹路径
	@param	board_size:棋盘的格子数
	@param	square_size:棋盘每一格子的宽度
	@param	rtsp:添加相机的RTSP地址
	@param	err_th:标定重映射误差的阈值，允许的最低值
	@returns：错误还是正确
	*/
	bool AddIPC(std::string filedir, cv::Size board_size, cv::Size square_size, cv::String rtsp, double err_th=1 );

	/*
	*@brief：设立世界坐标系，即对所有的IPC重设外参。这个函数当所有IPC在线时才成立
	运行此函数时，应该保证所有IPC分布在四周，且场中心有一个标定ARtag，且所有IPC都
	能看到此标定ARtag。一定要保证标定ARtag不能动，直到完成标定。
	@returns：错误还是正确
	*/
	bool setWorld();

	/*
	*@brief：按引索删除IPC
	@param  index:应该删除的IPC的序号
	@returns：错误还是正确
	*/
	bool DeleteIPC(int index);

	/*
	*@brief：按RTSP删除IPC
	@param  rtsp:应该删除的IPC的RTSP
	@returns：错误还是正确
	*/
	bool DeleteIPC(String rtsp);

	/*
	*@brief：得到所有IPC的rtsp
	@returns：所有IPC的rtsp
	*/
	std::vector<std::string> getIPCrtsp();

	/*
	*@brief：得到IPC个数
	@returns：IPC个数
	*/
	int getIPCNum();

	/*
	*@brief：与一个xml文件同步，实现了flash功能,,若xmlfile存在，则从xml文件中
	更新到IPC中，若xmlfile不存在，则从IPC中更新到xml中
	@param xmlfile：xmlfile xml文件名 xmlfilename.xml
	*/
	void bindxml(std::string xmlfile);

	/*
	*@brief：得到指定IPC编号的图像
	@param index：IPC编号
	@returns：图像
	*/
	Mat getIPCimage(int index);

	/*
	*@brief：得到指定IPC编号的经过AR处理的图片
	@param index：IPC编号
	@returns：图像
	*/
	Mat getIPCARPimage(int index);

	/*
	*@brief：通过对各个IPC的location到的vector<IPCobj>物体集合，进行整合和数据处理，得到
	真实的obj数据。该算法为多摄像头定位的核心算法。
	@param eobj：每一个IPC经过相机定位或者AR定位得到的物体集合，eobj[IPCindex][物体序号]
	@returns：整合好的真实的物体集合。
	*/
	std::vector<IPCobj> calculateAllObjection(std::vector<std::vector<IPCobj>> eobj);


	void UpdateXMLfile();//从IPC中重新更新到xmlFILE中，私有函数。只有bind了才能 Update
	void UpdateIPC();	 //从xmlFILE中重新更新到IPC中，私有函数。只有bind了才能 Update

	/*
	*@brief：画出obj
	@@author:张猛
	*/
	Mat paintObject(std::vector<IPCobj> input, Point2d center = Point2d(400, 300), float scale = 50);
	/*
	*@brief：找元素
	*/
	int findVecterElm(std::vector<IPCobj> vec, uint8_t robotidid);

};
