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

#include "stdafx.h"
#include "IPClocation.h"
#include "zfun.h"
#include "Eigen\Eigen"

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


/*-----------------------------------------------------------------------------
					IPClocation函数接口定义
							class
------------------------------------------------------------------------------*/
IPClocation::IPClocation()
{

}


bool IPClocation::AddIPC(string filedir, cv::Size board_size, cv::Size square_size, cv::String rtsp,double err_th)
{
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
		resize(image, imageCopy, Size(1280, 720));

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
	resize(img, img, Size(1280, 720));
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
		}
		else
		{
			iter++;
		}
	}
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

	//resize(img, imageCopy, Size(1280, 720));

	
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
	outimg = img.clone();
	//resize(img, imageCopy, Size(1280, 720));


	cv::aruco::detectMarkers(img, dictionary, corners, ids);
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

		rvecsx.clear(); tvecsx.clear(); idsx.clear(); cornersx.clear();
		//**选取大号机器人的id和角点位置
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


		rvecsx.clear(); tvecsx.clear(); idsx.clear(); cornersx.clear();
		//选取ARTAG
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
	resize(img, img, Size(1280, 720));
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
	//存储所有id号
	vector<int> ids;
	//存储所有角点
	vector<vector<Point2f> > corners;

	Mat imageCopy;
	Ptr<aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(ARtag_dict);

	//resize(img, imageCopy, Size(1280, 720));


	cv::aruco::detectMarkers(img, dictionary, corners, ids);
	// if at least one marker detected 
	if (ids.size() > 0)
	{

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

			retobj.push_back(newobj);
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
	return retobj;

}


bool IPClocation::cmp(Point2d &s1, Point2d &s2)
{
	return s1.y > s2.y;
}

//画表示物体方向的箭头
void IPClocation::drawArrow(cv::Mat& img, cv::Point2d pLocation, cv::Point2d pDirection, Point2d oPoint, int len, int alpha,
	cv::Scalar color, int thickness, int lineType)
{
	const double PI = 3.1415926;
	Point2d arrow;

	//计算 θ 角
	double angle = atan2((double)(oPoint.y - pDirection.y), (double)(oPoint.x - pDirection.x));

	pLocation.x = pLocation.x - sqrt(2) * 4 * cos(angle);
	pLocation.y = pLocation.y - sqrt(2) * 4 * sin(angle);
	//计算箭角边的另一端的端点位置
	arrow.x = pLocation.x + len * cos(angle + PI * alpha / 180);
	arrow.y = pLocation.y + len * sin(angle + PI * alpha / 180);
	line(img, pLocation, arrow, color, thickness, lineType);
	arrow.x = pLocation.x + len * cos(angle - PI * alpha / 180);
	arrow.y = pLocation.y + len * sin(angle - PI * alpha / 180);
	line(img, pLocation, arrow, color, thickness, lineType);
}

//画坐标轴的箭头
void IPClocation::drawCoorArrow(cv::Mat& img, cv::Point2d pLocation, cv::Point2d pDirection, Point2d oPoint, int len, int alpha,
	cv::Scalar color, int thickness , int lineType )
{
	const double PI = 3.1415926;
	Point2d arrow;

	//计算 θ 角
	double angle = atan2((double)(oPoint.y - pDirection.y), (double)(oPoint.x - pDirection.x));

	//计算箭角边的另一端的端点位置
	arrow.x = pLocation.x + len * cos(angle + PI * alpha / 180);
	arrow.y = pLocation.y + len * sin(angle + PI * alpha / 180);
	line(img, pLocation, arrow, color, thickness, lineType);
	arrow.x = pLocation.x + len * cos(angle - PI * alpha / 180);
	arrow.y = pLocation.y + len * sin(angle - PI * alpha / 180);
	line(img, pLocation, arrow, color, thickness, lineType);
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




Mat IPClocation::paintObject(vector<IPCobj> input, Point2d center, float scale)
{
	vector<Point2d> points;
	vector<Point2d>	directions;
	for (int i = 0; i < input.size(); i++)
	{
		points.push_back(Point2d(input[i].coordinate3D[0], input[i].coordinate3D[1]));
		directions.push_back(Point2d(input[i].direction3D[0], input[i].direction3D[1]));
	}
	double rows = center.y * 2; //图片的行
	double cols = center.x * 2;

	/*if (min(rows, cols) < 10 * scale)
	{
		cout << "图片的高宽太小,程序退出" << endl;
		exit(0);
	}*/
	Mat back(rows, cols, CV_8UC3, Scalar(255, 255, 255));
	Point2d oPoint = center; //中心点坐标
	vector<Point2d> xPoint; //x轴坐标
	vector<Point2d> yPoint;
	for (int i = -5; i <= 5; i++)
	{
		xPoint.push_back(Point2d(oPoint.x + i * scale, oPoint.y));
	}

	for (int i = -5; i <= 5; i++)
	{
		yPoint.push_back(Point2d(oPoint.x, oPoint.y + i * scale));
	}
	//sort(yPoint.begin(), yPoint.end(), cmp); //降序排列

	Point2d xEnd = Point2d(xPoint[10].x + 20, xPoint[10].y); //x轴终点
	Point2d yEnd = Point2d(yPoint[10].x, yPoint[10].y - 20); //y轴终点
	line(back, xPoint[0], xEnd, 0); //画出x轴
	drawCoorArrow(back, xEnd, Point2d(scale * 1 + cols / 2, -scale * 0 + rows / 2), oPoint, 5, 45, Scalar(0, 0, 0), 1);//画x轴的箭头

	line(back, yPoint[0], yEnd, 0);//画出y轴
	drawCoorArrow(back, yEnd, Point2d(scale * 0 + cols / 2, -scale * 1 + rows / 2), oPoint, 5, 45, Scalar(0, 0, 0), 1);//画y轴的箭头

																													   //显示x，y轴的坐标值
	char text[11];
	for (int i = -5; i <= 5; i++)
	{
		if (i == 0)
		{
			continue;
		}
		sprintf_s(text, "%d", i); //格式化输出
		putText(back, text, xPoint[i + 5], CV_FONT_HERSHEY_COMPLEX, 0.4, Scalar(0, 0, 255));
	}
	for (int i = -5; i <= 5; i++)
	{
		sprintf_s(text, "%d", i);
		putText(back, text, yPoint[i + 5], CV_FONT_HERSHEY_COMPLEX, 0.4, Scalar(0, 0, 255));
	}

	for (int i = 0; i < points.size(); i++)
	{
		int id = input[i].ID;
		//显示机器人的信息
		if (input[i].cls == 1)
		{
			circle(back, Point2d(scale * points[i].x + cols / 2, -scale * points[i].y + rows / 2), 4, Scalar(0, 0, 255), -1); //显示位置
			drawArrow(back, Point2d(scale * points[i].x + cols / 2, -scale * points[i].y + rows / 2), Point2d(scale * directions[i].x + cols / 2, -scale * directions[i].y + rows / 2),
				oPoint, 10, 45, Scalar(255, 0, 0)); //显示方向，用箭头表示
													//显示id
			string str;
			stringstream ss;
			ss << id;
			ss >> str;
			putText(back, str, Point2d(scale * points[i].x + cols / 2 - 8, -scale * points[i].y + rows / 2 - 10), CV_FONT_HERSHEY_COMPLEX, 0.4, Scalar(0, 0, 255));
		}
		//显示其他物体的位置
		else
		{
			rectangle(back, Point2d(scale * points[i].x + cols / 2 - 5, -scale * points[i].y + rows / 2 - 5),
				Point2d(scale * points[i].x + cols / 2 + 5, -scale * points[i].y + rows / 2 + 5), Scalar(0, 255, 0), -1); //显示位置
		}
	}

	//imshow("back", back);
	//imwrite("2.jpg", back);
	//waitKey(0);
	return back;
}
