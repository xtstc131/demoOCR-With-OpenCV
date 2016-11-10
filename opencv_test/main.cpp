#include <opencv2/opencv.hpp>
#include<iostream>
#include <string.h>
using namespace cv;
using namespace std;

void CharsSegment(Mat &img , Mat *imgArray)
{
	Mat src = img.clone();
	vector<Vec4i> hierarchy;
	vector< vector< Point> > contours;
	double minarea = 660;
	double maxarea = 0;
	int maxAreaIdx = 0;
	double whRatio = 0;
	/// Find contours  
	findContours(src, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
	//h_next:下一个序
	vector<vector<Point>>::iterator it = contours.begin();
	bool * check = new bool[contours.size()];
	for (; it != contours.end(); ++it)
	{
		double tmparea = fabs(contourArea(*it));
		if (tmparea > maxarea)
		{
			maxarea = tmparea;
			continue;
		}
		if (tmparea < minarea)
		{
			//删除面积小于设定值的轮廓  
			contours.erase(it);
			it = contours.begin();
			wcout << "delete a small area" << std::endl;
		}
		//计算轮廓的直径宽高  
	}
	for (int i = 0; i < contours.size(); i++)
	{
		Rect rect = boundingRect(Mat(contours[i]));
		imgArray[i] = img(rect);
		rectangle(img, rect.tl(), rect.br(), Scalar(255, 0, 0));
	}
	imshow("IMG", img);
	Size size(15, 15);
	for(int i =0 ; i < 4 ;i++)
	{
		string winNameStr = "img";
		winNameStr += char(48 + i);
		resize(imgArray[i], imgArray[i], size);
	//	imshow(winNameStr, imgArray[i]);
	}
}
int main()
{
	Mat threshold_result;
	//生成灰度图片
	Mat source = imread("1317.png", 0);
	//namedWindow("sourse", WINDOW_AUTOSIZE);
	Mat source_ext;
	//	resize(source, source_ext, Size(), 10.0, 10.0);
	imshow("source", source);

	//二值化灰度图片
	//简单来说，就是大于170的都变成255，否则变成0
	threshold(source, threshold_result, 170, 255, THRESH_BINARY);
	
	//Mat thredshold_ext;
	//resize(threshold_result, thredshold_ext, Size(), 10.0, 10.0);
	//imshow("result", thredshold_ext);


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
				floodFill(threshold_result, Point(i, j), Scalar(color));
				color++;
			}
		}
	}
	//分别计算联通的区域内有多少黑色像素
	int * colorCount = new int[color];
	memset(colorCount, 0, sizeof(int) * color);
	for (int j = 0; j < threshold_result.rows; j++)
	{
		//得到第j行的首地址  
		uchar* data = threshold_result.ptr<uchar>(j);
		//遍历每行的像素  
		for (int i = 0; i < threshold_result.cols; i++)
		{
			if(data[i] != 255)
				colorCount[data[i]]++;
		}
	}
	//test: colorCount[]
	/*for(int i =0; i < color ; i++)
	{
		cout << colorCount[i] << " ";
	}*/
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
	delete[] colorCount;
	colorCount = nullptr;

	//清除噪点，干扰线
	for (int j = 0; j < threshold_result.rows; j++)
	{
		//得到第j行的首地址  
		uchar* data = threshold_result.ptr<uchar>(j);
		//遍历每行的像素  
		for (int i = 0; i < threshold_result.cols; i++)
		{
			//如果连通区域的像素个数小于20，认定为噪点，清除为白
			if (data[i] == 255)
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
	
	//利用膨胀进一步处理
	//Mat elem = getStructuringElement(MORPH_RECT, Size(1, 1));
	//dilate(threshold_result, threshold_result, elem);

	Mat resize_img;
	resize(threshold_result, resize_img, Size(), 10, 10);
	//	imshow("Floodfill", resize_img);
	imshow("Fool3", threshold_result);
	//imwrite("px3w_result.png", threshold_result);

	//接下来，就可以对图像进行分割了。由于我们的分类器只能对数字一个一个地识别，所以首先要把每个数字分割
	//出来。基本思想是先用findContours()函数把基本轮廓找出来，然后通过简单验证以确认是否为数字的轮廓。对
	//	于那些通过验证的轮廓，接下去会用boundingRect()找出它们的包围盒。
	Mat * fourNumImg = new Mat[4];
	CharsSegment(resize_img,fourNumImg);
	waitKey();
	return 0;
}