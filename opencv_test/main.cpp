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
	//h_next:��һ����
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
			//ɾ�����С���趨ֵ������  
			contours.erase(it);
			it = contours.begin();
			wcout << "delete a small area" << std::endl;
		}
		//����������ֱ�����  
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
	//���ɻҶ�ͼƬ
	Mat source = imread("1317.png", 0);
	//namedWindow("sourse", WINDOW_AUTOSIZE);
	Mat source_ext;
	//	resize(source, source_ext, Size(), 10.0, 10.0);
	imshow("source", source);

	//��ֵ���Ҷ�ͼƬ
	//����˵�����Ǵ���170�Ķ����255��������0
	threshold(source, threshold_result, 170, 255, THRESH_BINARY);
	
	//Mat thredshold_ext;
	//resize(threshold_result, thredshold_ext, Size(), 10.0, 10.0);
	//imshow("result", thredshold_ext);


	//�ú�ˮ��䷨����ʵ��ͳ���ж��ٸ���ɫ����ͨ����
	int color = 1;
	for (int j = 0; j < threshold_result.rows; j++)
	{
		//�õ���j�е��׵�ַ  
		uchar* data = threshold_result.ptr<uchar>(j);
		//����ÿ�е�����  
		for (int i = 0; i < threshold_result.cols; i++)
		{
			if (data[i] == 0)
			{
				floodFill(threshold_result, Point(i, j), Scalar(color));
				color++;
			}
		}
	}
	//�ֱ������ͨ���������ж��ٺ�ɫ����
	int * colorCount = new int[color];
	memset(colorCount, 0, sizeof(int) * color);
	for (int j = 0; j < threshold_result.rows; j++)
	{
		//�õ���j�е��׵�ַ  
		uchar* data = threshold_result.ptr<uchar>(j);
		//����ÿ�е�����  
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
		//�õ���j�е��׵�ַ  
		uchar* data = threshold_result.ptr<uchar>(j);
		//����ÿ�е�����  
		for (int i = 0; i < threshold_result.cols; i++)
		{
			//�����ͨ��������ظ���С��20���϶�Ϊ��㣬���Ϊ��
			if (data[i] != 255 && colorCount[data[i]] > 20)
			{
				data[i] = 0;
			}
			//��������Ϊ��ɫ�������֧�Ǳ���ģ���Ϊ��һ����FloodFill�����еĺ�ɫ������ˣ���ɫ��0 < data[i] < 255��
			else
			{
				data[i] = 255;
			}
		}
	}
	delete[] colorCount;
	colorCount = nullptr;

	//�����㣬������
	for (int j = 0; j < threshold_result.rows; j++)
	{
		//�õ���j�е��׵�ַ  
		uchar* data = threshold_result.ptr<uchar>(j);
		//����ÿ�е�����  
		for (int i = 0; i < threshold_result.cols; i++)
		{
			//�����ͨ��������ظ���С��20���϶�Ϊ��㣬���Ϊ��
			if (data[i] == 255)
			{
				data[i] = 0;
			}
			//��������Ϊ��ɫ�������֧�Ǳ���ģ���Ϊ��һ����FloodFill�����еĺ�ɫ������ˣ���ɫ��0 < data[i] < 255��
			else
			{
				data[i] = 255;
			}
		}
	}
	
	//�������ͽ�һ������
	//Mat elem = getStructuringElement(MORPH_RECT, Size(1, 1));
	//dilate(threshold_result, threshold_result, elem);

	Mat resize_img;
	resize(threshold_result, resize_img, Size(), 10, 10);
	//	imshow("Floodfill", resize_img);
	imshow("Fool3", threshold_result);
	//imwrite("px3w_result.png", threshold_result);

	//���������Ϳ��Զ�ͼ����зָ��ˡ��������ǵķ�����ֻ�ܶ�����һ��һ����ʶ����������Ҫ��ÿ�����ַָ�
	//����������˼��������findContours()�����ѻ��������ҳ�����Ȼ��ͨ������֤��ȷ���Ƿ�Ϊ���ֵ���������
	//	����Щͨ����֤������������ȥ����boundingRect()�ҳ����ǵİ�Χ�С�
	Mat * fourNumImg = new Mat[4];
	CharsSegment(resize_img,fourNumImg);
	waitKey();
	return 0;
}