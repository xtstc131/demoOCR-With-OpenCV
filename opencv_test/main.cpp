#include <opencv2/opencv.hpp>
#include<sys/types.h>
#include <direct.h>
#include<iostream>
#include <corecrt_io.h>
using namespace cv;
using namespace std;
const int MAX_CONTOUR_AREA = 400;
const int MIN_CONTOUR_AREA = 0;
const int RESIZED_IMAGE_WIDTH = 20;
const int RESIZED_IMAGE_HEIGHT = 30;
class  ContourWithData
{
public:
	vector<Point>  ptContour;
	Rect boundingRect;
	float fltArea;
	bool checkIfContourValid() const
	{
		if (fltArea < MIN_CONTOUR_AREA )
			return false;
		return true;
	}
	static bool sortByBoudingRextXPosition(const ContourWithData & cwdLeft, const ContourWithData & cwdRight)
	{
		return (cwdLeft.boundingRect.x < cwdRight.boundingRect.x);
	}
};
void CharsSegment(Mat &img, Mat* &imgArray,int &contourNum)
{
	Mat src = img.clone();
	vector<Vec4i> hierarchy;
	vector< vector< Point> > contours;
	/// Find contours  
	findContours(src, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	//h_next:下一个序
	vector<vector<Point>>::iterator it = contours.begin();
	//imshow("sec", src);
	while (it != contours.end())
	{
		double tmparea = fabs(contourArea(*it));
		//cout << tmparea << endl;
		if (tmparea < MIN_CONTOUR_AREA )
		{
			//删除面积小于设定值的轮廓  
			it = contours.erase(it);
			//wcout << "delete a  area" << endl;
		}
		else
		{
			++it;
		}
		//计算轮廓的直径宽高  
	}
	vector<ContourWithData> allContoursWithData;
	for (int i = 0; i < contours.size(); i++) {               // for each contour
		ContourWithData contourWithData;                                                    // instantiate a contour with data object
		contourWithData.ptContour = contours[i];                                          // assign contour to contour with data
		contourWithData.boundingRect = cv::boundingRect(contourWithData.ptContour);         // get the bounding rect              // calculate the contour area
		allContoursWithData.push_back(contourWithData);						                                 // add contour with data object to list of all contours with data
	}
	sort(allContoursWithData.begin(), allContoursWithData.end(), ContourWithData::sortByBoudingRextXPosition);
	Mat imgCopy = img.clone();
	contourNum = allContoursWithData.size();
	if(contourNum)
		imgArray = new Mat[contourNum];
	else
	{
		return;
	}
	for (int i = 0; i < contourNum; i++)
	{
		Rect rect = allContoursWithData[i].boundingRect;
		imgArray[i] = img(rect);
	//	imshow("img", imgArray[i]);
		rectangle(imgCopy, rect.tl(), rect.br(), Scalar(255, 0, 0));
	}
	//imshow("IMG", imgCopy);
	Size size(RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT);
	for (int i = 0; i < contourNum; i++)
	{
		string winNameStr = "img";
		winNameStr += char(48 + i);
		resize(imgArray[i], imgArray[i], size);
		//imshow(winNameStr, imgArray[i]);
	}
}
void getFile(string path, vector<string> & files ,vector<string> & fileNames)
{
	intptr_t hFile = 0;
	_finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if (fileinfo.attrib & _A_SUBDIR)
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				{
					getFile(p.assign(path).append("\\").append(fileinfo.name), files,fileNames);
				}
			}
			else
			{
				fileNames.push_back(fileinfo.name);
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}
int main()
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	//                 Read the trianning data
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	
	//读取训练的证明数据（就是对应的字符矩阵表示的字符究竟是神马
	Mat matClassficationsInts;
	FileStorage fsClassfications("classifications.xml", FileStorage::READ);
	if (fsClassfications.isOpened() == false)
	{
		cerr << "error, unable to open the classfication file" << endl;
		return 0;
	}
	fsClassfications["classifications"] >> matClassficationsInts;
	fsClassfications.release();

	//读取样本字符图片的浮点数类型矩阵
	Mat matTrainningImagesAsFlattenedFloats;
	FileStorage fsTrainningImages("images.xml", FileStorage::READ);
	if (fsTrainningImages.isOpened() == false)
	{
		cerr << "error, unable to open the images file" << endl;
		return 0;
	}
	fsTrainningImages["images"] >> matTrainningImagesAsFlattenedFloats;
	fsTrainningImages.release();

	Ptr<ml::KNearest> kNearest(ml::KNearest::create());
	kNearest->train(matTrainningImagesAsFlattenedFloats, ml::ROW_SAMPLE, matClassficationsInts);
	//////////////////////////////////////////////////////////////////
	// Trianning completed , then we are going to preprocess the Image which we will recongenize
	//////////////////////////////////////////////////////////////////
	vector<string> files;
	vector<string> fileNames;
	getFile("C:\\Users\\mallox\\Documents\\Visual Studio 2015\\Projects\\opencv_test\\opencv_test\\code", files,fileNames);
	//生成灰度图片
	double correctNum = 0;
	for (int index = 0; index < files.size(); ++index)
	{
		Mat source = imread(files[index], 0);
		if (source.empty())
		{
			cerr << "error, Can't open the image" << endl;
			return 0;
		}
		//namedWindow("sourse", WINDOW_AUTOSIZE);
		/*Mat source_ext;*/
		//	resize(source, source_ext, Size(), 10.0, 10.0);
		//imshow("source", source);

///////////////////////////////////////////////////////////////////////////////////////////////////////
//                   preprocess the image 
///////////////////////////////////////////////////////////////////////////////////////////////////////
			//二值化灰度图片
			//简单来说，就是大于170的都变成255，否则变成0
		Mat threshold_result;
		threshold(source, threshold_result, 170, 255, THRESH_BINARY);

		//用洪水填充法，来实现统计有多少个黑色的连通区域
		int color = 1;
		for (int j = 0; j < threshold_result.rows; j++)
		{
			//得到第j行的首地址  
			uchar* data = threshold_result.ptr<uchar>(j);
			//遍历每行的像素  
			for (int i = 0; i < threshold_result.cols; i++)
			{
				if (data[i] == 0)
				{
					//把每个连通域的像素都设置为一样的颜色（1-254）都可以，所以理论上只能
					//支持最多254个连通域的统计，便于后续的统计连通域的像素个数
					floodFill(threshold_result, Point(i, j), Scalar(color));
					color++;
				}
			}
		}

		//分别计算联通的区域内有多少黑色像素
		int * colorCount = new int[color];//保存每个非白色连通区域的像素数量
		memset(colorCount, 0, sizeof(int) * color);//Initial the array
		//统计每个连通域的像素个数
		for (int j = 0; j < threshold_result.rows; j++)
		{
			//得到第j行的首地址  
			uchar* data = threshold_result.ptr<uchar>(j);
			//遍历每行的像素  
			for (int i = 0; i < threshold_result.cols; i++)
			{
				if (data[i] != 255)//如果这个像素不是白色的就在当前的颜色对应的像素数量就+1
					colorCount[data[i]]++;
			}
		}
		//如果连通域的像素个数小于某个特定值，我们就把他当成 噪点，或者干扰线 去除
		for (int j = 0; j < threshold_result.rows; j++)
		{
			//得到第j行的首地址  
			uchar* data = threshold_result.ptr<uchar>(j);
			//遍历每行的像素  
			for (int i = 0; i < threshold_result.cols; i++)
			{
				//如果连通区域的像素个数小于20，认定为噪点，清除为白
				if (data[i] != 255 && colorCount[data[i]] > 20)
				{
					data[i] = 0;
				}
				//否则设置为黑色，这个分支是必须的，因为上一步的FloodFill把所有的黑色都变成了，灰色（0 < data[i] < 255）
				else
				{
					data[i] = 255;
				}
			}
		}
		delete[] colorCount;//释放数组
		colorCount = nullptr;

		//反色，其实可能OPENCV有封装好的函数所以待改进(已修复)
		bitwise_not(threshold_result, threshold_result);

		//TODO  原来的腐蚀效果不是很好，所以尝试别的进一步去噪方法
		//利用膨胀进一步处理
		Mat elem = getStructuringElement(MORPH_RECT, Size(1, 1));
		erode(threshold_result, threshold_result, elem);

		//放大图片
		Mat resize_img;
		resize(threshold_result, resize_img, Size(), 10, 10);


		////////////////////////////////////////////////////////////////////////////////////////////////////////
		//                   Recongnize the Img 
		////////////////////////////////////////////////////////////////////////////////////////////////////////

				//接下来，就可以对图像进行分割了。由于我们的分类器只能对数字一个一个地识别，所以首先要把每个数字分割
				//出来。基本思想是先用findContours()函数把基本轮廓找出来，然后通过简单验证以确认是否为数字的轮廓。对
				//	于那些通过验证的轮廓，接下去会用boundingRect()找出它们的包围盒。
		Mat * EachNumArray = nullptr;
		int contourNum = 0;
		CharsSegment(threshold_result, EachNumArray, contourNum);
		if (!EachNumArray)
		{
			cerr << "Haven't detective a Number" << endl;
			return 0;
		}
		string strFinalString;
		for (int i = 0; i < contourNum; i++)
		{
			Mat  matROIFloat;
			EachNumArray[i].convertTo(matROIFloat, CV_32FC1);
			Mat matROIFlattenedFloat = matROIFloat.reshape(1, 1);
			cv::Mat matCurrentChar(0, 0, CV_32F);
			kNearest->findNearest(matROIFlattenedFloat, 1, matCurrentChar);     // finally we can call find_nearest !!!

			auto fltCurrentChar = static_cast<float>(matCurrentChar.at<float>(0, 0));

			strFinalString = strFinalString + char(int(fltCurrentChar));
		}
		delete[]EachNumArray;
		cout <<"FileName = "<< fileNames[index].c_str();
		cout << "	numbers read = " << strFinalString;
		size_t pos = fileNames[index].find_first_of(".");
		string trueValue = fileNames[index].substr(0,pos);
		if (trueValue == strFinalString)
		{
			correctNum++; 
			cout << "		True" << endl;
		}
		else
		{
			cout << "		False"<<endl;
		}
	
	}
	double correctPer = correctNum / double(files.size());
	printf_s("Correct Percentage : %.2f", correctPer);
	getchar();
	waitKey();
	return 0;
}