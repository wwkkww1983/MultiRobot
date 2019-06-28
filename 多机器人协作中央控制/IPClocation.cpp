/*
IPClocation
说  明：一个用RTSP地址IP摄像头来进行定位的库，提供了定位接口
并且提供了一个消息类IPCobj，可以作为消息输出，存储了在场地上有多少
物体并且给出了他的位置。
制作人：邹智强
版  本：beta 1.1
更  改：
	1、添加颜色识别功能，识别颜色并且定位出物体的位置。
*/

#include "stdafx.h"
#include "Eigen\Eigen"
#include "IPClocation.h"
#include "zfun.h"

#include <opencv2/core/eigen.hpp>

using namespace std;





IPCobj::IPCobj()
{
}

/*-----------------------------------------------------------------------------
						IPCmsg函数接口定义
							class
------------------------------------------------------------------------------*/

//初始化，主要初始化矩阵大小
IPCmsg::IPCmsg()
{
	cameraMatrix= Mat(3, 3, CV_32FC1, Scalar::all(0));
	distCoeffs = Mat(1, 5, CV_32FC1, Scalar::all(0));
	RwMatrix = Mat(3, 3, CV_32FC1, Scalar::all(0));
	TwVec = Mat(3, 1, CV_32FC1, Scalar::all(0));
	Status = NOSTAR;
}

IPCmsg::IPCmsg(String addr)
{
	cameraMatrix = Mat(3, 3, CV_32FC1, Scalar::all(0));
	distCoeffs = Mat(1, 5, CV_32FC1, Scalar::all(0));
	RwMatrix = Mat(3, 3, CV_32FC1, Scalar::all(0));
	TwVec = Mat(3, 1, CV_32FC1, Scalar::all(0));
	Status = NOSTAR;
	rtsp = addr;
}
bool IPCmsg::Open()
{
	bool ret=cap.open(rtsp);
	if (ret == false)
	{
		Status = CANNOT_OPEN;
	}
	else
	{
		Status = NORMAL;
	}
	return ret;
}
IPCmsg::StatusEnum IPCmsg::checkStatus()
{
	return Status;
}
void IPCmsg::updateRwMatrixI()
{
	invert(RwMatrix, RwMatrixI);
}
void IPCmsg::updatecameraMatrixI()
{
	invert(cameraMatrix, cameraMatrixI);
}


/*-----------------------------------------------------------------------------
					IPClocation函数接口定义
							class
------------------------------------------------------------------------------*/
IPClocation::IPClocation()
{

}


bool IPClocation::AddIPC(string filedir, cv::Size board_size, cv::Size square_size, cv::String rtsp,double err_th)
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);

	//查看是否有rtsp重复，有则返回错误
	vector<string> allipcrtsp=getIPCrtsp();
	if (zfun::findVecterElm(allipcrtsp, rtsp) != -1)
	{
		printf("error:IPC中存在相同rtsp地址的IP相机");
		return false;
	}

	//标定结果
	Mat cameraMatrix;
	Mat distCoeffs; 
	vector<Mat> rvecsMat; 
	vector<Mat> tvecsMat;
	
	int image_count = 0;                                            // 图像数量 
	Size image_size;                                                // 图像的尺寸 

	std::vector<Point2f> image_points;                              // 缓存每幅图像上检测到的角点
	std::vector<std::vector<Point2f>> image_points_seq;             // 保存检测到的所有角点

	//检测文件夹下的所有jpg图片
	std::vector<std::string> FilesName;//存储所有图片文件
	string FileType = ".jpg";
	std::string buffer = filedir + "\\*" + FileType;
	_finddata_t c_file;   // 存放文件名的结构体
	long long hFile;
	hFile = _findfirst(buffer.c_str(), &c_file);   //找第一个文件命
	if (hFile == -1L)   // 检查文件夹目录下存在需要查找的文件
		printf("No %s files in current directory!\n", FileType);
	else
	{
		string fullFilePath;
		do
		{
			fullFilePath.clear();
			//名字
			fullFilePath = filedir + "\\" + c_file.name;
			FilesName.push_back(fullFilePath);
		} while (_findnext(hFile, &c_file) == 0);  //如果找到下个文件的名字成功的话就返回0,否则返回-1  
		_findclose(hFile);
	}

	//检测角点
	for (int i = 0; i < FilesName.size(); i++)
	{
		image_count++;

		// 用于观察检验输出
		cout << "image_count = " << image_count << endl;
		Mat imageInput = imread(FilesName[i]);
		if (image_count == 1)  //读入第一张图片时获取图像宽高信息
		{
			image_size.width = imageInput.cols;
			image_size.height = imageInput.rows;
			cout << "image_size.width = " << image_size.width << endl;
			cout << "image_size.height = " << image_size.height << endl;
		}

		/* 提取角点 */
		bool ok = findChessboardCorners(imageInput, board_size, image_points, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);
		if (0 == ok)
		{
			cout << "第" << image_count << "张照片提取角点失败，请删除后，重新标定！" << endl; //找不到角点
			
			return false;
		}
		else
		{
			Mat view_gray;
			cout << "imageInput.channels()=" << imageInput.channels() << endl;
			cvtColor(imageInput, view_gray, CV_RGB2GRAY);

			/* 亚像素精确化 */
			//find4QuadCornerSubpix(view_gray, image_points, Size(5, 5)); //对粗提取的角点进行精确化
			cv::cornerSubPix(view_gray, image_points, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 20, 0.01));

			image_points_seq.push_back(image_points);  //保存亚像素角点

													   /* 在图像上显示角点位置 */
			drawChessboardCorners(view_gray, board_size, image_points, true);

			//imshow("Camera Calibration", view_gray);//显示图片
			//waitKey(100);//暂停0.1S     
		}
	}
	cout << "角点提取完成！！！" << endl;


	/*棋盘三维信息*/
	vector<vector<Point3f>> object_points_seq;                     // 保存标定板上角点的三维坐标
	for (int t = 0; t < image_count; t++)
	{
		vector<Point3f> object_points;
		for (int i = 0; i < board_size.height; i++)
		{
			for (int j = 0; j < board_size.width; j++)
			{
				Point3f realPoint;
				/* 假设标定板放在世界坐标系中z=0的平面上 */
				realPoint.x = i*square_size.width;
				realPoint.y = j*square_size.height;
				realPoint.z = 0;
				object_points.push_back(realPoint);
			}
		}
		object_points_seq.push_back(object_points);
	}

	/* 运行标定函数 */

	double err_first = calibrateCamera(object_points_seq, image_points_seq, image_size, cameraMatrix, distCoeffs, rvecsMat, tvecsMat, CV_CALIB_FIX_K3);
	cout << "重投影误差1：" << err_first << "像素" << endl << endl;
	cout << "标定完成！！！" << endl;

	//如果误差太大，则返回错误
	if (err_first > err_th)
	{
		printf("error:误差太大");
		return false;
	}
	
	
	//添加IPC
	IPCmsg newIPC(rtsp);
	newIPC.cameraMatrix = cameraMatrix;
	newIPC.distCoeffs = distCoeffs;
	IPC.push_back(newIPC);

	//保存定标结果    
	UpdateXMLfile();
	//关闭控制台
	FreeConsole();
	return true;
}

//从IPC中重新更新到xmlFILE中，私有函数。
void IPClocation::UpdateXMLfile()
{
	if (xmlfileName.empty() == true) return;
	FileStorage xml(xmlfileName, cv::FileStorage::WRITE);
	//rdxml << "IPC" << IPC;
	xml << "delayTime" << delayTime;
	xml << "IPCNum" << (int)IPC.size();
	for (size_t i = 0; i < IPC.size(); i++)
	{
		std::string indexc=std::to_string(i);
		xml << "r" + indexc << IPC[i].rtsp;
		xml << "M" + indexc << IPC[i].cameraMatrix;
		xml << "D" + indexc << IPC[i].distCoeffs;
		xml << "R" + indexc << IPC[i].RwMatrix;
		xml << "T" + indexc << IPC[i].TwVec;
	}
	xml.release();
}

void IPClocation::UpdateIPC()
{
	if (xmlfileName.empty() == true) return;
	FileStorage xml(xmlfileName, cv::FileStorage::READ);
	//清空IPC，关闭摄像头
	for (size_t i = 0; i < IPC.size(); i++)
	{
		IPC[i].cap.release();
	}
	IPC.clear();
	//读取IPCxml
	int IPCNum;
	xml["IPCNum"] >> IPCNum;
	xml["delayTime"] >> delayTime;
	for (size_t i = 0; i < IPCNum; i++)
	{
		IPCmsg newipc;
		std::string indexc = std::to_string(i);
		xml["r" + indexc] >> newipc.rtsp;
		xml["M" + indexc] >> newipc.cameraMatrix;
		xml["D" + indexc] >> newipc.distCoeffs;
		xml["R" + indexc] >> newipc.RwMatrix;
		xml["T" + indexc] >> newipc.TwVec;
		newipc.updateRwMatrixI();
		newipc.updatecameraMatrixI();
		IPC.push_back(newipc);
	}

	xml.release();

}
  
//得到所有IPC的rtsp
std::vector<std::string> IPClocation::getIPCrtsp()
{
	vector<string> outrtsp;
	for (int i = 0; i < IPC.size(); i++)
	{
		outrtsp.push_back(IPC[i].rtsp);
	}
	return outrtsp;
}

/*
*@brief：与一个xml文件同步，实现了flash功能,只有没有xmlfileName时，才生效。
*/
void IPClocation::bindxml(std::string xmlfile)
{
	xmlfileName = xmlfile;
	//查看是否存在xml
	FileStorage tsxml(xmlfileName, cv::FileStorage::READ);
	if (tsxml.isOpened() == true)
	{
		tsxml.release();
		UpdateIPC();
	}
	else
	{
		tsxml.release();
		UpdateXMLfile();
	}
}

/*
*@brief：设立世界坐标系，即对所有的IPC重设外参。这个函数当所有IPC在线时才成立
运行此函数时，应该保证所有IPC分布在四周，且场中心有一个标定ARtag，且所有IPC都
能看到此标定ARtag。
*/
bool IPClocation::setWorld()
{
	//检查所有IP相机是否能打开。
	for (size_t ipci = 0; ipci < IPC.size(); ipci++)
	{
		if (IPC[ipci].Open() == false) return false;
	}
	//对所有IPC分别进行采样标定
	for (size_t ipci = 0; ipci < IPC.size(); ipci++)
	{
		//相机坐标系的位姿信息
		vector< Vec3d > rvecs, tvecs;
		vector<int> ids;
		//字典
		Ptr<aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(ARtag_dict);
		//图片
		Mat image, imageCopy;
		//从相机采样
		IPC[ipci].cap >> image;
		resize(image, imageCopy, proimgsize);

		//开始计算
		vector<vector<Point2f> > corners;
		cv::aruco::detectMarkers(imageCopy, dictionary, corners, ids);
		// 如果有AR码，则进行姿态估计
		if (ids.size() > 0)
		{
			cv::aruco::estimatePoseSingleMarkers(corners, worldAR_size, IPC[ipci].cameraMatrix, IPC[ipci].distCoeffs, rvecs, tvecs);
		}
		else
		{
			return false;
		}
		//查找是否有标定AR码
		int orignum = zfun::findVecterElm(ids, worldAR_ID);//标定tag码所在的引索
		//如果有标定AR码
		if (orignum >= 0)
		{
			//得到（世界坐标系的RT）
			Rodrigues(rvecs[orignum], IPC[ipci].RwMatrix);
			IPC[ipci].TwVec = Mat(tvecs[orignum], true);
			IPC[ipci].updateRwMatrixI();
		}
		else
		{
			return false;
		}

	}
	UpdateXMLfile();
	return true;
}

/*
*@brief：得到指定IPC编号的经过AR处理的图片
*/
Mat IPClocation::getIPCARPimage(int index)
{
	Mat img;
	IPC[index].cap >> img;
	resize(img, img, proimgsize);
	//相机坐标系的位姿信息
	vector<int> ids;
	double starttime = cv::getTickCount();//计时

	vector<vector<Point2f> > corners;
	Ptr<aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(aruco::DICT_6X6_250);
	cv::aruco::detectMarkers(img, dictionary, corners, ids);
	// if at least one marker detected 
	if (ids.size() > 0)
	{
		
		vector< Vec3d > rvecsx, tvecsx;		//存储小号机器人的姿态
		vector<int> idsx;					//储存小号机器人的ID
		vector<vector<Point2f> > cornersx;	//储存小号机器人的角点

		//选取小号机器人的id和角点位置
		for (size_t i = 0; i < ids.size(); i++)
		{
			if (ids[i] >= AR_ID_distribList[1][0] && ids[i] <= AR_ID_distribList[1][1])
			{
				//找到了小号机器人id
				idsx.push_back(ids[i]);
				cornersx.push_back(corners[i]);
			}
		}
		if (idsx.size() > 0)
		{
			//对这些小号机器人进行姿态估计
			cv::aruco::estimatePoseSingleMarkers(cornersx, (float)AR_ID_distribList[1][2] / 1000.0, IPC[index].cameraMatrix, IPC[index].distCoeffs, rvecsx, tvecsx);
			//画画
			cv::aruco::drawDetectedMarkers(img, cornersx, idsx);

			for (int i = 0; i < idsx.size(); i++)
				cv::aruco::drawAxis(img, IPC[index].cameraMatrix, IPC[index].distCoeffs, rvecsx[i], tvecsx[i], 0.1);
		}

		rvecsx.clear(); tvecsx.clear(); idsx.clear(); cornersx.clear();

		//选取大号机器人的id和角点位置
		for (size_t i = 0; i < ids.size(); i++)
		{
			if (ids[i] >= AR_ID_distribList[0][0] && ids[i] <= AR_ID_distribList[0][1])
			{
				//找到了小号机器人id
				idsx.push_back(ids[i]);
				cornersx.push_back(corners[i]);
			}
		}
		if (idsx.size() > 0)
		{
			//对这些小号机器人进行姿态估计
			cv::aruco::estimatePoseSingleMarkers(cornersx, (float)AR_ID_distribList[0][2] / 1000.0, IPC[index].cameraMatrix, IPC[index].distCoeffs, rvecsx, tvecsx);
			//画画
			cv::aruco::drawDetectedMarkers(img, cornersx, idsx);

			for (int i = 0; i < idsx.size(); i++)
				cv::aruco::drawAxis(img, IPC[index].cameraMatrix, IPC[index].distCoeffs, rvecsx[i], tvecsx[i], 0.1);
		}

		rvecsx.clear(); tvecsx.clear(); idsx.clear(); cornersx.clear();
		//标定ARtag
		for (size_t i = 0; i < ids.size(); i++)
		{
			if (ids[i]== worldAR_ID)
			{
				//找到了小号机器人id
				idsx.push_back(ids[i]);
				cornersx.push_back(corners[i]);
			}
		}
		if (idsx.size() > 0)
		{
			//对姿态估计
			cv::aruco::estimatePoseSingleMarkers(cornersx, worldAR_size, IPC[index].cameraMatrix, IPC[index].distCoeffs, rvecsx, tvecsx);
			//画画
			cv::aruco::drawDetectedMarkers(img, cornersx, idsx);

			for (int i = 0; i < idsx.size(); i++)
				cv::aruco::drawAxis(img, IPC[index].cameraMatrix, IPC[index].distCoeffs, rvecsx[i], tvecsx[i], 0.5);
		}


	}
	return img;
}

/*
*@brief：得到IPC个数
*/
int IPClocation::getIPCNum()
{
	return IPC.size();
}

/*
*@brief：按引索删除IPC
*/
bool IPClocation::DeleteIPC(int index)
{
	if (index >= IPC.size() || index < 0)
	{
		return false;
	}

	std::vector<IPCmsg>::iterator iter=IPC.begin();
	IPC.erase(iter + index);
	return true;
}

/*
*@brief：按RTSP删除IPC
*/
bool IPClocation::DeleteIPC(String rtsp)
{
	std::vector<IPCmsg>::iterator iter;

	for ( iter = IPC.begin(); iter != IPC.end();)
	{
		if (iter->rtsp == rtsp)
		{
			iter=IPC.erase(iter);
			return true;
		}
		else
		{
			iter++;
		}
	}
	return false;
}

/*
@brief:以一张来自IPC的图片来定位图中的AR码，返回世界3D坐标
*/
std::vector<IPCobj> IPClocation::location(Mat img, int IPCindex)
{
	std::vector<IPCobj> retobj;

	//开始定位
	//存储所有id号
	vector<int> ids;
	//存储所有角点
	vector<vector<Point2f> > corners;

	Mat imageCopy;
	Ptr<aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(ARtag_dict);

	resize(img, imageCopy, proimgsize);

	
	cv::aruco::detectMarkers(imageCopy, dictionary, corners, ids);
	// if at least one marker detected 
	if (ids.size() > 0)
	{
		
		vector< Vec3d > rvecsx, tvecsx;		//存储小号机器人的姿态
		vector<int> idsx;					//储存小号机器人的ID
		vector<vector<Point2f> > cornersx;	//储存小号机器人的角点

		//选取小号机器人的id和角点位置
		for (size_t i = 0; i < ids.size(); i++)
		{
			if (ids[i] >= AR_ID_distribList[1][0] && ids[i] <= AR_ID_distribList[1][1])
			{
				//找到了小号机器人id
				idsx.push_back(ids[i]);
				cornersx.push_back(corners[i]);
			}
		}
		if (idsx.size() > 0)
		{
			//对这些小号机器人进行姿态估计
			cv::aruco::estimatePoseSingleMarkers(cornersx, (float)AR_ID_distribList[1][2] / 1000.0, IPC[IPCindex].cameraMatrix, IPC[IPCindex].distCoeffs, rvecsx, tvecsx);
		}
		//得到这些小号机器人世界坐标系坐标
		for (size_t i = 0; i < idsx.size(); i++)
		{
			//定义一个IPCobj 用于记录物体信息
			IPCobj newobj;
			newobj.cls = IPCobj::Robot;
			newobj.ID = idsx[i];
			newobj.dimension = 3;
			//开始进行坐标转化
			Mat Pw; //存储物体世界坐标
			Mat TsVec = Mat(tvecsx[i], true);
			Pw = IPC[IPCindex].RwMatrixI*(TsVec - IPC[IPCindex].TwVec);//公式
			if (Pw.rows==3&&Pw.cols==1)//如果正确就转化到coordinate3D
			{
				newobj.coordinate3D = Pw;//?????
			}
			Mat rw; //存储物体方向
			Mat Rs;//存储物体旋转矩阵
			Mat re = Mat(Vec3d(1, 0, 0), true);;//
			Rodrigues(rvecsx[i], Rs);
			rw = IPC[IPCindex].RwMatrixI*Rs*re;//公式
			if (rw.rows == 3 && rw.cols == 1)//如果正确就转化到direction3D
			{
				newobj.direction3D = rw;//?????
			}
			retobj.push_back(newobj);
		}

		rvecsx.clear(); tvecsx.clear(); idsx.clear(); cornersx.clear();
		//选取大号机器人的id和角点位置
		for (size_t i = 0; i < ids.size(); i++)
		{
			if (ids[i] >= AR_ID_distribList[0][0] && ids[i] <= AR_ID_distribList[0][1])
			{
				//找到了大号机器人id
				idsx.push_back(ids[i]);
				cornersx.push_back(corners[i]);
			}
		}
		if (idsx.size() > 0)
		{
			//对这些机器人进行姿态估计
			cv::aruco::estimatePoseSingleMarkers(cornersx, (float)AR_ID_distribList[0][2] / 1000.0, IPC[IPCindex].cameraMatrix, IPC[IPCindex].distCoeffs, rvecsx, tvecsx);

			//得到这些机器人世界坐标系坐标
			for (size_t i = 0; i < idsx.size(); i++)
			{
				//定义一个IPCobj 用于记录物体信息
				IPCobj newobj;
				newobj.cls = IPCobj::Robot;
				newobj.ID = idsx[i];
				newobj.dimension = 3;
				//开始进行坐标转化
				Mat Pw; //存储物体世界坐标
				Mat TsVec = Mat(tvecsx[i], true);
				Pw = IPC[IPCindex].RwMatrixI*(TsVec - IPC[IPCindex].TwVec);//公式
				if (Pw.rows == 3 && Pw.cols == 1)//如果正确就转化到coordinate3D
				{
					newobj.coordinate3D = Pw;//?????
				}
				Mat rw; //存储物体方向
				Mat Rs;//存储物体旋转矩阵
				Mat re = Mat(Vec3d(1, 0, 0), true);;//
				Rodrigues(rvecsx[i], Rs);
				rw = IPC[IPCindex].RwMatrixI*Rs*re;//公式
				if (rw.rows == 3 && rw.cols == 1)//如果正确就转化到direction3D
				{
					newobj.direction3D = rw;//?????
				}
				retobj.push_back(newobj);
			}
		}
	}

	return retobj;

}


/*
@brief:以一张来自IPC的图片来定位图中的AR码，返回世界3D坐标,同时返回一个绘制图片
*/
std::vector<IPCobj> IPClocation::location(Mat img, int IPCindex, Mat &outimg)
{
	std::vector<IPCobj> retobj;

	//开始定位
	//存储所有id号
	vector<int> ids;
	//存储所有角点
	vector<vector<Point2f> > corners;

	Mat imageCopy;
	Ptr<aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(ARtag_dict);
	resize(img, imageCopy, proimgsize);
	outimg = imageCopy.clone();

	cv::aruco::detectMarkers(imageCopy, dictionary, corners, ids);
	// if at least one marker detected 
	if (ids.size() > 0)
	{

		vector< Vec3d > rvecsx, tvecsx;		//存储小号机器人的姿态
		vector<int> idsx;					//储存小号机器人的ID
		vector<vector<Point2f> > cornersx;	//储存小号机器人的角点

		//**选取小号机器人的id和角点位置
		for (size_t i = 0; i < ids.size(); i++)
		{
			if (ids[i] >= AR_ID_distribList[1][0] && ids[i] <= AR_ID_distribList[1][1])
			{
				//找到了小号机器人id
				idsx.push_back(ids[i]);
				cornersx.push_back(corners[i]);
			}
		}
		if (idsx.size() > 0)
		{
			//对这些小号机器人进行姿态估计
			cv::aruco::estimatePoseSingleMarkers(cornersx, (float)AR_ID_distribList[1][2] / 1000.0, IPC[IPCindex].cameraMatrix, IPC[IPCindex].distCoeffs, rvecsx, tvecsx);
		
			//画画
			cv::aruco::drawDetectedMarkers(outimg, cornersx, idsx);
			for (int j = 0; j < idsx.size(); j++)
				cv::aruco::drawAxis(outimg, IPC[IPCindex].cameraMatrix, IPC[IPCindex].distCoeffs, rvecsx[j], tvecsx[j], 0.1);
			//得到这些小号机器人世界坐标系坐标
			for (size_t i = 0; i < idsx.size(); i++)
			{
				//定义一个IPCobj 用于记录物体信息
				IPCobj newobj;
				newobj.cls = IPCobj::Robot;
				newobj.ID = idsx[i];
				newobj.dimension = 3;
				//开始进行坐标转化
				Mat Pw; //存储物体世界坐标
				Mat TsVec = Mat(tvecsx[i], true);
				Pw = IPC[IPCindex].RwMatrixI*(TsVec - IPC[IPCindex].TwVec);//公式
				if (Pw.rows == 3 && Pw.cols == 1)//如果正确就转化到coordinate3D
				{
					newobj.coordinate3D = Pw;//?????
				}
				Mat rw; //存储物体方向
				Mat Rs;//存储物体旋转矩阵
				Mat re = Mat(Vec3d(1, 0, 0), true);;//
				Rodrigues(rvecsx[i], Rs);
				rw = IPC[IPCindex].RwMatrixI*Rs*re;//公式
				if (rw.rows == 3 && rw.cols == 1)//如果正确就转化到direction3D
				{
					newobj.direction3D = rw;//?????
				}
				retobj.push_back(newobj);
			}
		}

		//**选取大号机器人的id和角点位置
		rvecsx.clear(); tvecsx.clear(); idsx.clear(); cornersx.clear();
		for (size_t i = 0; i < ids.size(); i++)
		{
			if (ids[i] >= AR_ID_distribList[0][0] && ids[i] <= AR_ID_distribList[0][1])
			{
				//找到了大号机器人id
				idsx.push_back(ids[i]);
				cornersx.push_back(corners[i]);
			}
		}
		if (idsx.size() > 0)
		{
			//对这些机器人进行姿态估计
			cv::aruco::estimatePoseSingleMarkers(cornersx, (float)AR_ID_distribList[0][2] / 1000.0, IPC[IPCindex].cameraMatrix, IPC[IPCindex].distCoeffs, rvecsx, tvecsx);

			//画画
			cv::aruco::drawDetectedMarkers(outimg, cornersx, idsx);
			for (int i = 0; i < idsx.size(); i++)
				cv::aruco::drawAxis(outimg, IPC[IPCindex].cameraMatrix, IPC[IPCindex].distCoeffs, rvecsx[i], tvecsx[i], 0.1);
			//得到这些小号机器人世界坐标系坐标
			for (size_t i = 0; i < idsx.size(); i++)
			{
				//定义一个IPCobj 用于记录物体信息
				IPCobj newobj;
				newobj.cls = IPCobj::Robot;
				newobj.ID = idsx[i];
				newobj.dimension = 3;
				//开始进行坐标转化
				Mat Pw; //存储物体世界坐标
				Mat TsVec = Mat(tvecsx[i], true);
				Pw = IPC[IPCindex].RwMatrixI*(TsVec - IPC[IPCindex].TwVec);//公式
				if (Pw.rows == 3 && Pw.cols == 1)//如果正确就转化到coordinate3D
				{
					newobj.coordinate3D = Pw;//?????
				}
				Mat rw; //存储物体方向
				Mat Rs;//存储物体旋转矩阵
				Mat re = Mat(Vec3d(1, 0, 0), true);;//
				Rodrigues(rvecsx[i], Rs);
				rw = IPC[IPCindex].RwMatrixI*Rs*re;//公式
				if (rw.rows == 3 && rw.cols == 1)//如果正确就转化到direction3D
				{
					newobj.direction3D = rw;//?????
				}
				retobj.push_back(newobj);
			}
		}

		//**选取ARTAG
		rvecsx.clear(); tvecsx.clear(); idsx.clear(); cornersx.clear();
		for (size_t i = 0; i < ids.size(); i++)
		{
			if (ids[i]== worldAR_ID)
			{
				//找到了大号机器人id
				idsx.push_back(ids[i]);
				cornersx.push_back(corners[i]);
			}
		}
		if (idsx.size() > 0)
		{
			//对这些机器人进行姿态估计
			cv::aruco::estimatePoseSingleMarkers(cornersx, worldAR_size, IPC[IPCindex].cameraMatrix, IPC[IPCindex].distCoeffs, rvecsx, tvecsx);

			//画画
			cv::aruco::drawDetectedMarkers(outimg, cornersx, idsx);
			for (int i = 0; i < idsx.size(); i++)
				cv::aruco::drawAxis(outimg, IPC[IPCindex].cameraMatrix, IPC[IPCindex].distCoeffs, rvecsx[i], tvecsx[i], 0.5);
		}
		
	}

	return retobj;
}

/*
@brief:以IPCindex号IP相机拍摄到的画面来定位AR码，返回世界3D坐标
*/
std::vector<IPCobj> IPClocation::location(int IPCindex)
{
	Mat img;
	IPC[IPCindex].cap >> img;
	resize(img, img, proimgsize);
	vector<IPCobj> retobj = location(img, IPCindex);
	return retobj;
}

/*
*@brief：得到指定IPC编号的图像
*/
Mat IPClocation::getIPCimage(int index)
{
	Mat img;
	IPC[index].cap >> img;
	return img;
}

/*
@brief:以一张来自IPC的图片来定位图中的AR码，返回相机坐标
*/
std::vector<IPCobj> IPClocation::locationMat(Mat img, int IPCindex)
{
	std::vector<IPCobj> retobj;

	//开始定位
	vector<int> ids;//存储所有id号
	vector<vector<Point2f> > corners;//存储所有角点


	Mat imageCopy;
	Ptr<aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(ARtag_dict);

	resize(img, imageCopy, proimgsize);


	cv::aruco::detectMarkers(imageCopy, dictionary, corners, ids);
	// if at least one marker detected 
	if (ids.size() > 0)
	{

		vector< Vec3d > rvecsx, tvecsx;		//存储小号机器人的姿态
		vector<int> idsx;					//储存小号机器人的ID
		vector<vector<Point2f> > cornersx;	//储存小号机器人的角点

		//**选取小号机器人的id和角点位置
		for (size_t i = 0; i < ids.size(); i++)
		{
			if (ids[i] >= AR_ID_distribList[1][0] && ids[i] <= AR_ID_distribList[1][1])
			{
				//找到了小号机器人id
				idsx.push_back(ids[i]);
				cornersx.push_back(corners[i]);
			}
		}
		if (idsx.size() > 0)
		{

			//对这些小号机器人进行姿态估计
			if(estimation_Algorithm==0)
				cv::aruco::estimatePoseSingleMarkers(cornersx, (float)AR_ID_distribList[1][2] / 1000.0, IPC[IPCindex].cameraMatrix, IPC[IPCindex].distCoeffs, rvecsx, tvecsx);

			//得到这些小号机器人相机坐标
			for (size_t i = 0; i < idsx.size(); i++)
			{
				//定义一个IPCobj 用于记录物体信息
				IPCobj newobj;
				newobj.cls = IPCobj::Robot;
				newobj.ID = idsx[i];
				newobj.dimension = 2;
				//开始进行坐标转化
				newobj.coordinate2D = calculateCentre(cornersx[i]);
				if (estimation_Algorithm == 0)
				{
					Mat rw; //存储物体方向
					Mat Rs;//存储物体旋转矩阵
					Mat re = Mat(Vec3d(1, 0, 0), true);;//
					Rodrigues(rvecsx[i], Rs);
					rw = IPC[IPCindex].RwMatrixI*Rs*re;//公式
					if (rw.rows == 3 && rw.cols == 1)//如果正确就转化到direction3D
					{
						newobj.direction3D = rw;//?????
					}
				}
				retobj.push_back(newobj);
			}
		}

		

		//**选取大号机器人的id和角点位置
		rvecsx.clear(); tvecsx.clear(); idsx.clear(); cornersx.clear();
		for (size_t i = 0; i < ids.size(); i++)
		{
			if (ids[i] >= AR_ID_distribList[0][0] && ids[i] <= AR_ID_distribList[0][1])
			{
				//找到了大号机器人id
				idsx.push_back(ids[i]);
				cornersx.push_back(corners[i]);
			}
		}
		if (idsx.size() > 0)
		{

			//对这些大号机器人进行姿态估计
			if (estimation_Algorithm == 0)
				cv::aruco::estimatePoseSingleMarkers(cornersx, (float)AR_ID_distribList[0][2] / 1000.0, IPC[IPCindex].cameraMatrix, IPC[IPCindex].distCoeffs, rvecsx, tvecsx);

			//得到这些大号机器人相机坐标
			for (size_t i = 0; i < idsx.size(); i++)
			{
				//定义一个IPCobj 用于记录物体信息
				IPCobj newobj;
				newobj.cls = IPCobj::Robot;
				newobj.ID = idsx[i];
				newobj.dimension = 2;
				//开始进行坐标转化
				newobj.coordinate2D = calculateCentre(cornersx[i]);
				if (estimation_Algorithm == 0)
				{
					Mat rw; //存储物体方向
					Mat Rs;//存储物体旋转矩阵
					Mat re = Mat(Vec3d(1, 0, 0), true);;//
					Rodrigues(rvecsx[i], Rs);
					rw = IPC[IPCindex].RwMatrixI*Rs*re;//公式
					if (rw.rows == 3 && rw.cols == 1)//如果正确就转化到direction3D
					{
						newobj.direction3D = rw;//?????
					}
				}
				retobj.push_back(newobj);
			}
		}


	}

	return retobj;

}

Point2i IPClocation::calculateCentre(vector<Point2f> corner)
{
	float dy13 = corner[0].y - corner[2].y;
	float dx13 = corner[0].x - corner[2].x;
	float dy24 = corner[1].y - corner[3].y;
	float dx24 = corner[1].x - corner[3].x;
	
	Eigen::Matrix2f K;
	K << dy13, -dx13,
		 dy24, -dx24;
	Eigen::Vector2f B;
	B << dy13*corner[0].x - dx13*corner[0].y, 
		 dy24*corner[1].x - dx24*corner[1].y;
	Eigen::Vector2f retxy;
	retxy = K.inverse()*B;
	Point2f ret;
	ret.x = retxy[0]; ret.y = retxy[1];
	return ret;
}

bool IPClocation::AddIPC(std::vector<Mat> img, cv::Size board_size, cv::Size square_size, cv::String rtsp, double err_th )
{
	//查看是否有rtsp重复，有则返回错误
	vector<string> allipcrtsp = getIPCrtsp();
	if (zfun::findVecterElm(allipcrtsp, rtsp) != -1)
	{
		printf("error:IPC中存在相同rtsp地址的IP相机");
		return false;
	}

	//标定结果
	Mat cameraMatrix;
	Mat distCoeffs;
	vector<Mat> rvecsMat;
	vector<Mat> tvecsMat;

	int image_count = 0;                                            // 图像数量 
	Size image_size;                                                // 图像的尺寸 

	std::vector<Point2f> image_points;                              // 缓存每幅图像上检测到的角点
	std::vector<std::vector<Point2f>> image_points_seq;             // 保存检测到的所有角点



	//检测角点
	for (int i = 0; i < img.size(); i++)
	{
		image_count++;

		// 用于观察检验输出
		cout << "image_count = " << image_count << endl;
		Mat imageInput = img[i];
		if (image_count == 1)  //读入第一张图片时获取图像宽高信息
		{
			image_size.width = imageInput.cols;
			image_size.height = imageInput.rows;
			cout << "image_size.width = " << image_size.width << endl;
			cout << "image_size.height = " << image_size.height << endl;
		}

		/* 提取角点 */
		bool ok = findChessboardCorners(imageInput, board_size, image_points, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);
		if (0 == ok)
		{
			cout << "第" << image_count << "张照片提取角点失败，请删除后，重新标定！" << endl; //找不到角点

			return false;
		}
		else
		{
			Mat view_gray;
			cout << "imageInput.channels()=" << imageInput.channels() << endl;
			cvtColor(imageInput, view_gray, CV_RGB2GRAY);

			/* 亚像素精确化 */
			//find4QuadCornerSubpix(view_gray, image_points, Size(5, 5)); //对粗提取的角点进行精确化
			cv::cornerSubPix(view_gray, image_points, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 20, 0.01));

			image_points_seq.push_back(image_points);  //保存亚像素角点

													   /* 在图像上显示角点位置 */
			drawChessboardCorners(view_gray, board_size, image_points, true);

			//imshow("Camera Calibration", view_gray);//显示图片
			//waitKey(100);//暂停0.1S     
		}
	}
	cout << "角点提取完成！！！" << endl;


	/*棋盘三维信息*/
	vector<vector<Point3f>> object_points_seq;                     // 保存标定板上角点的三维坐标
	for (int t = 0; t < image_count; t++)
	{
		vector<Point3f> object_points;
		for (int i = 0; i < board_size.height; i++)
		{
			for (int j = 0; j < board_size.width; j++)
			{
				Point3f realPoint;
				/* 假设标定板放在世界坐标系中z=0的平面上 */
				realPoint.x = i*square_size.width;
				realPoint.y = j*square_size.height;
				realPoint.z = 0;
				object_points.push_back(realPoint);
			}
		}
		object_points_seq.push_back(object_points);
	}

	/* 运行标定函数 */

	double err_first = calibrateCamera(object_points_seq, image_points_seq, image_size, cameraMatrix, distCoeffs, rvecsMat, tvecsMat, CV_CALIB_FIX_K3);
	cout << "重投影误差1：" << err_first << "像素" << endl << endl;
	cout << "标定完成！！！" << endl;

	//如果误差太大，则返回错误
	if (err_first > err_th)
	{
		printf("error:误差太大");
		return false;
	}


	//添加IPC
	IPCmsg newIPC(rtsp);
	newIPC.cameraMatrix = cameraMatrix;
	newIPC.distCoeffs = distCoeffs;
	IPC.push_back(newIPC);

	//保存定标结果    
	UpdateXMLfile();

	return true;
}

std::vector<IPCobj> IPClocation::calculateAllObjection(std::vector<std::vector<IPCobj>> eobj)
{
	//检测输入参数合理性（ex：同一个IPC下看到了多个ID相同的机器人）

	//找robot，并且整合
	vector<uint8_t> robotID;//找到了那些robotID
	vector<vector<Vec2i>> fIPCindex;//这些robotID都出现在哪个IPC里,并且是在这个IPC里的第几个序号

	//遍历eobj
	for (size_t i = 0; i < eobj.size(); i++)
	{
		for (size_t j = 0; j < eobj[i].size(); j++)
		{
			//判断这个IPCOBJ是不是robot
			if (eobj[i][j].cls == IPCobj::Robot)
			{
				//查看这个robotid是否有入栈过
				int samIDindex = zfun::findVecterElm(robotID, eobj[i][j].ID);	
				if (samIDindex == -1)//否
				{
					robotID.push_back(eobj[i][j].ID);
					Vec2i x; x[0] = i; x[1] = j;
					vector<Vec2i> newindex; newindex.push_back(x);//第i个IPC出现了新robotID
					fIPCindex.push_back(newindex);
				}
				else
				{
					Vec2i x; x[0] = i; x[1] = j;
					fIPCindex[samIDindex].push_back(x);
				}
			}
		}
	}

	//针对机器人开始整合新的IPCobj
	vector<IPCobj> retobj;

	if (Algorithm == 0)//确认算法
	{
		for (size_t i = 0; i < robotID.size(); i++)
		{
			IPCobj newobj;
			newobj.dimension = 3;
			newobj.ID = robotID[i];
			newobj.cls = IPCobj::Robot;
			newobj.coordinate3D[0] = 0; newobj.coordinate3D[1] = 0; newobj.coordinate3D[2] = 0;
			newobj.direction3D[0] = 0; newobj.direction3D[1] = 0; newobj.direction3D[2] = 0;
			//遍历该robotID所有的IPC,并且平均坐标
			for (size_t j = 0; j < fIPCindex[i].size(); j++)
			{
				
				newobj.coordinate3D[0] += eobj[fIPCindex[i][j][0]][fIPCindex[i][j][1]].coordinate3D[0];
				newobj.coordinate3D[1] += eobj[fIPCindex[i][j][0]][fIPCindex[i][j][1]].coordinate3D[1];
				newobj.coordinate3D[2] += eobj[fIPCindex[i][j][0]][fIPCindex[i][j][1]].coordinate3D[2];
				newobj.direction3D[0] += eobj[fIPCindex[i][j][0]][fIPCindex[i][j][1]].direction3D[0];
				newobj.direction3D[1] += eobj[fIPCindex[i][j][0]][fIPCindex[i][j][1]].direction3D[1];
				newobj.direction3D[2] += eobj[fIPCindex[i][j][0]][fIPCindex[i][j][1]].direction3D[2];
			}
			newobj.coordinate3D[0] /= fIPCindex[i].size();
			newobj.coordinate3D[1] /= fIPCindex[i].size();
			newobj.coordinate3D[2] /= fIPCindex[i].size();
			//push
			retobj.push_back(newobj);
		}
	}
	else if(Algorithm == 1)//1:多相机交点定位
	{
		//测试用，要删掉
		//AllocConsole();
		//freopen("CONOUT$", "w", stdout);

		for (size_t i = 0; i < robotID.size(); i++)
		{

			if (fIPCindex[i].size() >= 2)//只有有两个以上的相机出现了该robot，则可以用融合算法
			{
				//计算k向量
				vector<Eigen::Vector3d> K;
				for (size_t n = 0; n < fIPCindex[i].size(); n++)
				{
					int IPCID= fIPCindex[i][n][0];//摄像头编号
					IPCobj thisobj = eobj[IPCID][fIPCindex[i][n][1]];

					Eigen::Matrix3d RI;
					cv2eigen(IPC[IPCID].RwMatrixI, RI);
					Eigen::Matrix3d MI;
					cv2eigen(IPC[IPCID].cameraMatrixI, MI);
					Eigen::Vector3d Pc;
					Pc << (double)thisobj.coordinate2D.x, (double)thisobj.coordinate2D.y, 1.0;
					K.push_back(RI*MI*Pc);
				}
				//计算b向量
				vector<Eigen::Vector3d> B;
				for (size_t n = 0; n < fIPCindex[i].size(); n++)
				{
					int IPCID = fIPCindex[i][n][0];//摄像头编号

					Eigen::Matrix3d RI;
					cv2eigen(IPC[IPCID].RwMatrixI, RI);
					Eigen::Vector3d T;
					cv2eigen(IPC[IPCID].TwVec, T);
					B.push_back(RI*T);
				}
				//根据IPC个数，定义HQ向量
				Eigen::MatrixXd H; H.setZero(3 * fIPCindex[i].size(), 3 + fIPCindex[i].size());
				Eigen::VectorXd Q(3 * fIPCindex[i].size());
				Eigen::MatrixXd E; E.setIdentity(3,3);

				//赋值HQ向量
				for (size_t n = 0; n <fIPCindex[i].size(); n++)
				{
					H.block(n * 3, 0, 3, 3) = -E;
					H.block(n * 3, 3+n, 3, 1) = K[n];
				}
	
				for (size_t n = 0; n <fIPCindex[i].size(); n++)
				{
					Q.block(n * 3, 0, 3, 1) = B[n];
				}
		
				//广义逆矩阵计算
				Eigen::VectorXd Pw = H.colPivHouseholderQr().solve(Q);
		
				//位置点已经知道，开始push到retobj里
				IPCobj newobj;
				newobj.dimension = 3;
				newobj.ID = robotID[i];
				newobj.cls = IPCobj::Robot;
				newobj.coordinate3D[0] = Pw[0]; newobj.coordinate3D[1] = Pw[1]; newobj.coordinate3D[2] = Pw[2];
				//如果是用了AR估计姿态算法，则需要平均姿态值
				if (estimation_Algorithm == 0)
				{
					newobj.direction3D[0] = 0; newobj.direction3D[1] = 0; newobj.direction3D[2] = 0;
					//遍历该robotID所有的IPC,并且平均坐标
					for (size_t j = 0; j < fIPCindex[i].size(); j++)
					{
						newobj.direction3D[0] += eobj[fIPCindex[i][j][0]][fIPCindex[i][j][1]].direction3D[0];
						newobj.direction3D[1] += eobj[fIPCindex[i][j][0]][fIPCindex[i][j][1]].direction3D[1];
						newobj.direction3D[2] += eobj[fIPCindex[i][j][0]][fIPCindex[i][j][1]].direction3D[2];
					}
					newobj.direction3D[0] /= fIPCindex[i].size();
					newobj.direction3D[1] /= fIPCindex[i].size();
					newobj.direction3D[2] /= fIPCindex[i].size();
				}
				retobj.push_back(newobj);
			}
		}
		//关闭控制台
		//FreeConsole();
	}

	
	return retobj;

}


//画表示物体方向的箭头
void IPClocation::drawArrow(cv::Mat& img, Point p, Point2f dirc, float size)
{
	int chan = arrowimg.channels();
	Mat arrow;
	resize(arrowimg, arrow, Size((int)size, (int)size));
	int w = arrow.rows;
	
	float a = 3.14159/2-atan2(dirc.y, dirc.x);

	for (int i = 0; i < arrow.rows; i++)
	{
		for (int j = 0; j < arrow.cols; j++)
		{
			if (arrow.at<Vec4b>(i, j)[3] > 0)
			{
				int xdst = (float)(j - w / 2)*cos(a) - (float)(i - w / 2)*sin(a) + (float)p.x;
				int ydst = (float)(j - w / 2)*sin(a) + (float)(i - w / 2)*cos(a) + (float)p.y;
				if (xdst > 0 && xdst < img.cols&&ydst>0 && ydst < img.rows)
				{
					img.at<Vec3b>(ydst, xdst)[0] = arrow.at<Vec4b>(i, j)[0];
					img.at<Vec3b>(ydst, xdst)[1] = arrow.at<Vec4b>(i, j)[1];
					img.at<Vec3b>(ydst, xdst)[2] = arrow.at<Vec4b>(i, j)[2];
				}
			}	
		}
	}
	
}



int IPClocation::findVecterElm(vector<IPCobj> vec, uint8_t robotidid)
{
	int ret;
	std::vector<IPCobj>::iterator iter;

	for (iter = vec.begin(); iter < vec.end(); iter++)
	{
		if (iter->ID == robotidid)
		{
			ret = &*iter - &vec[0];
			return ret;
		}
	}
	return -1;

}


void IPClocation::initMap(std::string mapname)
{
	map = imread(mapname);
	arrowimg = imread("arrow.png", CV_LOAD_IMAGE_UNCHANGED);

	vector<vector<Point>> fanP;
	//对每一个IPC进行计算
	for (size_t ipci = 0; ipci < getIPCNum(); ipci++)
	{
		//计算k向量
		vector<Eigen::Vector3d> K;
		Eigen::Matrix3d RI;
		cv2eigen(IPC[ipci].RwMatrixI, RI);
		Eigen::Matrix3d MI;
		cv2eigen(IPC[ipci].cameraMatrixI, MI);
		Eigen::Vector3d Pc;
		Pc << 0, 0, 1;
		K.push_back(RI*MI*Pc);
		Pc << 1920, 0, 1;
		K.push_back(RI*MI*Pc);
		Pc << 1920, 1080, 1;
		K.push_back(RI*MI*Pc);
		Pc << 0, 1080, 1;
		K.push_back(RI*MI*Pc);

		//计算b向量
		Eigen::Vector3d B;
		Eigen::Vector3d T;
		cv2eigen(IPC[ipci].TwVec, T);
		B = RI*T;

		vector<Point> piciP;
		for (size_t i = 0; i < 4; i++)
		{
			Eigen::Matrix3d H;
			H << -1, 0, K[i][0],
				0, -1, K[i][1],
				0, 0, K[i][2];
			Eigen::Vector3d	Po=H.inverse()*B;
			piciP.push_back(Point(Po[0]*m2pix+map.rows/2,-Po[1]* m2pix+map.cols/2));
		}
		fanP.push_back(piciP);	
	}
	//画画
	for (size_t i = 0; i < getIPCNum(); i++)
	{
		Mat dst;
		map.copyTo(dst);
		cv::Point pt[1][4];
		pt[0][0] = fanP[i][0];
		pt[0][1] = fanP[i][1];
		pt[0][2] = fanP[i][2];
		pt[0][3] = fanP[i][3];

		const cv::Point* ppt[1] = { pt[0] };
		int npt[1] = { 4 };
		cv::fillPoly(map, ppt, npt, 1, cv::Scalar(0, 255, 0));
		cv::addWeighted(dst, 0.95, map, 0.05, 0, map);
	}
		
		
	
	
	 
}
int IPClocation::getMapSize()
{
	return map.rows;
}

Mat IPClocation::paintObject(vector<IPCobj> input, Point2i lookCenter, int scale)
{
	int retmap_size = 800;
	vector<Point2d> points;
	vector<Point2d>	directions;
	Point2i visonxy; visonxy.x = lookCenter.x - scale / 2; visonxy.y = lookCenter.y - scale / 2;
	for (int i = 0; i < input.size(); i++)
	{
		points.push_back(Point2d(input[i].coordinate3D[0], input[i].coordinate3D[1]));
		directions.push_back(Point2d(input[i].direction3D[0], input[i].direction3D[1]));
	}

	if (scale<50)
	{
		scale = 50;
	}
	if (scale > 5000)
	{
		scale = 5000;
	}

	Point2d oPoint; oPoint.x = map.cols/2; oPoint.y = map.rows/2;//读取地图的中心点坐标


	//截取rect
	Rect rect(visonxy.x, visonxy.y, scale, scale);
	Mat retcap;
	map(rect).copyTo(retcap);

	//画栅格
	if (scale <= 100)
	{
		int lev= m2pix / 100;
		int startg_x = lev-(visonxy.x % lev);
		int lineNum_x = (int)(scale - startg_x) / lev;
		//显示厘米栅格
		for (size_t i = 0; i <= lineNum_x; i++)
		{
			line(retcap, Point(startg_x + i*lev, 0), Point(startg_x + i*lev,scale),Scalar(150,150,150));
		}

		int startg_y = lev - (visonxy.y % lev);
		int lineNum_y = (int)(scale - startg_y) / lev;
		//显示厘米栅格
		for (size_t i = 0; i <= lineNum_y; i++)
		{
			line(retcap, Point(0, startg_y + i*lev), Point(scale, startg_y + i*lev), Scalar(150, 150, 150));
		}
	}
	else if(scale <= 1000)
	{
		int lev = m2pix / 10;
		int startg_x = lev - (visonxy.x % lev);
		int lineNum_x = (int)(scale - startg_x) / lev;
		//显示d米栅格
		for (size_t i = 0; i <= lineNum_x; i++)
		{
			line(retcap, Point(startg_x + i*lev, 0), Point(startg_x + i*lev, scale), Scalar(0, 0, 0));
		}

		int startg_y = lev - (visonxy.y % lev);
		int lineNum_y = (int)(scale - startg_y) / lev;
		//显示d米栅格
		for (size_t i = 0; i <= lineNum_y; i++)
		{
			line(retcap, Point(0, startg_y + i*lev), Point(scale, startg_y + i*lev), Scalar(0, 0, 0));
		}
	}

	resize(retcap, retcap, Size(retmap_size, retmap_size));
	//标记机器人点
	for (size_t i = 0; i < input.size(); i++)
	{
		if (input[i].cls == IPCobj::Robot&& input[i].dimension==3)
		{
			//坐标转换，将世界坐标转换到map图片像素坐标。
			Point2i pointMap;
			pointMap.x = oPoint.x + points[i].x*m2pix;
			pointMap.y = oPoint.y - points[i].y*m2pix;
			//判断点是否在retcap内
			if ((pointMap.x > (int)(rect.x)) && (pointMap.x<(int)(rect.x + scale)) && (pointMap.y>rect.y) && (pointMap.y < (int)(rect.y + scale)))
			{
				Point pp;
				pp.x = (pointMap.x - rect.x)*retmap_size/ scale;
				pp.y = (pointMap.y - rect.y)*retmap_size / scale;

				//circle(retcap, pp, scale / 100, Scalar(255, 0, 0), -1);
				drawArrow(retcap, pp, directions[i], retmap_size / 30.0);
				//显示id号
				string idstr="id:";
				
				putText(retcap, idstr + to_string(input[i].ID),Point(pp.x,pp.y), FONT_HERSHEY_COMPLEX, 0.6, Scalar(0, 0, 0), 1, 8);

			}
		}
	}
	
	
	return retcap;
	

}

vector<IPCobj> IPClocation::detectColor(int ipcindex, Mat src)
{
	//定义颜色范围
	Scalar bule_lower = Scalar(100, 43, 46);
	Scalar bule_upper = Scalar(155, 255, 255);

	Scalar blk_lower = Scalar(0, 0, 0);
	Scalar blk_upper = Scalar(180, 255, 46);

	Scalar red0_lower = Scalar(170, 100, 70);
	Scalar red0_upper = Scalar(180, 255, 255);
	Scalar red_lower = Scalar(0, 100, 70);
	Scalar red_upper = Scalar(10, 255, 255);

	Scalar green_lower = Scalar(35, 43, 46);
	Scalar green_upper = Scalar(77, 255, 255);
	//物体高度信息。
	Eigen::Vector3d objheight[3];
	objheight[0] << 0, 0, 0.01;
	objheight[1] << 0, 0, 0.003;
	objheight[2] << 0, 0, 0.018;

	//计算识别范围
	vector<Point> kregion;
	Eigen::Matrix3d R;
	cv2eigen(IPC[ipcindex].RwMatrix, R);
	Eigen::Matrix3d M;
	cv2eigen(IPC[ipcindex].cameraMatrix, M);
	Eigen::Vector3d T;
	cv2eigen(IPC[ipcindex].TwVec, T);
	Eigen::Vector3d Pw[4];
	Pw[0] << 0.1, 0.1, 0;
	Pw[1] << 0.1, -0.4, 0;
	Pw[2] << -0.3, -0.4, 0;
	Pw[3] << -0.3, 0.1, 0;
	for (size_t i = 0; i < 4; i++)
	{
		Eigen::Vector3d Pc = M*(R*Pw[i] + T);
		Point NEWuvp = Point(Pc[0] / Pc[2], Pc[1] / Pc[2]);
		kregion.push_back(NEWuvp);
	}
	//生成区域图像
	Mat regionImg(src.rows, src.cols, CV_8UC1);
	regionImg.setTo(0);
	std::vector<std::vector<cv::Point >> kregions;
	kregions.push_back(kregion);
	fillPoly(regionImg, kregions, Scalar(255));

	//检测图片中的rgb
	Point2f obj[3];
	for (size_t i = 0; i < 3; i++)
	{
		vector<Point2f> retvec;
		Point2f ret = Point2f(0, 0);
		Mat dst;
		cvtColor(src, dst, COLOR_BGR2HSV);

		if (i == 0)//r
		{
			Mat ds1, ds2;
			inRange(dst, red0_lower, red0_upper, ds1);
			inRange(dst, red_lower, red_upper, ds2);
			bitwise_or(ds1, ds2, dst);
			//只看区域内的情况
			bitwise_and(regionImg, dst, dst);

		}
		else if (i == 2)//b
		{
			inRange(dst, bule_lower, bule_upper, dst);
			//只看区域内的情况
			bitwise_and(regionImg, dst, dst);
		}
		else if (i == 1)//g
		{
			inRange(dst, green_lower, green_upper, dst);
			//只看区域内的情况
			bitwise_and(regionImg, dst, dst);
		}
		// 腐蚀操作
		Mat element = getStructuringElement(0, Size(5, 5));
		erode(dst, dst, element);
		dilate(dst, dst, element);
		//定义轮廓和层次结构
		vector<vector<Point> >contours;
		vector<Vec4i> hierarchy;
		//查找轮廓
		findContours(dst, contours, hierarchy, CV_RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
		//遍历所有的顶层的轮廓，随机颜色绘制每个连接组件颜色
		if (hierarchy.size() > 0)
		{
			vector<Moments> mom(contours.size());
			vector<Point2f> m(contours.size());
			for (size_t i = 0; i < contours.size(); i++)
			{
				mom[i] = moments(contours[i], false);
				m[i] = Point(static_cast<float>(mom[i].m10 / mom[i].m00), static_cast<float>(mom[i].m01 / mom[i].m00));
			}
			for (size_t i = 0; i < m.size(); i++)
			{
				if (pointPolygonTest(kregion, m[i], false) == 1)
				{
					retvec.push_back(m[i]);
				}
			}
			for (size_t i = 0; i < retvec.size(); i++)
			{
				ret.x += retvec[i].x;
				ret.y += retvec[i].y;
			}
			ret.x = ret.x / retvec.size();
			ret.y = ret.y / retvec.size();

		}

		obj[i]= ret;
	}
	
	//计算物体的世界坐标点
	vector<IPCobj> wdobj;
	//计算b向量
	Eigen::Vector3d B;
	Eigen::Matrix3d RI;
	cv2eigen(IPC[ipcindex].RwMatrixI, RI);
	cv2eigen(IPC[ipcindex].TwVec, T);
	B = RI*T;
	//计算k向量
	Eigen::Vector3d K;

	Eigen::Matrix3d MI;
	cv2eigen(IPC[ipcindex].cameraMatrixI, MI);
	Eigen::Vector3d Pc;

	for (size_t i = 0; i < 3; i++)
	{
		if (obj[i].x != 0)
		{
			IPCobj newwdobj;
			//计算k向量
			Pc << obj[i].x, obj[i].y, 1;
			K=(RI*MI*Pc);
			//计算世界坐标
			Eigen::Matrix3d H;
			H << -1, 0, K[0],
				0, -1, K[1],
				0, 0, K[2];
			Eigen::Vector3d	Po = H.inverse()*(B+objheight[i]);
			//Eigen::Vector3d	Po=H.colPivHouseholderQr().solve(B + objheight[i]);
			//push到世界物体中去
			newwdobj.coordinate3D = Vec3d(Po[0], Po[1], objheight[i][2]*2);
			if (i == 0)
			{
				newwdobj.cls = IPCobj::objclass::redobj;
				newwdobj.ID = 81;
			}
			else if (i == 1)
			{
				newwdobj.cls = IPCobj::objclass::greenobj;
				newwdobj.ID = 82;
			}
			else if (i == 2)
			{
				newwdobj.cls = IPCobj::objclass::buleobj;
				newwdobj.ID = 83;
			}
			wdobj.push_back(newwdobj);
		}
	}
	return wdobj;
}
