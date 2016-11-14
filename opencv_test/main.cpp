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
	//h_next:��һ����
	vector<vector<Point>>::iterator it = contours.begin();
	//imshow("sec", src);
	while (it != contours.end())
	{
		double tmparea = fabs(contourArea(*it));
		//cout << tmparea << endl;
		if (tmparea < MIN_CONTOUR_AREA )
		{
			//ɾ�����С���趨ֵ������  
			it = contours.erase(it);
			//wcout << "delete a  area" << endl;
		}
		else
		{
			++it;
		}
		//����������ֱ�����  
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
	
	//��ȡѵ����֤�����ݣ����Ƕ�Ӧ���ַ������ʾ���ַ�����������
	Mat matClassficationsInts;
	FileStorage fsClassfications("classifications.xml", FileStorage::READ);
	if (fsClassfications.isOpened() == false)
	{
		cerr << "error, unable to open the classfication file" << endl;
		return 0;
	}
	fsClassfications["classifications"] >> matClassficationsInts;
	fsClassfications.release();

	//��ȡ�����ַ�ͼƬ�ĸ��������;���
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
	//���ɻҶ�ͼƬ
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
			//��ֵ���Ҷ�ͼƬ
			//����˵�����Ǵ���170�Ķ����255��������0
		Mat threshold_result;
		threshold(source, threshold_result, 170, 255, THRESH_BINARY);

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
					//��ÿ����ͨ������ض�����Ϊһ������ɫ��1-254�������ԣ�����������ֻ��
					//֧�����254����ͨ���ͳ�ƣ����ں�����ͳ����ͨ������ظ���
					floodFill(threshold_result, Point(i, j), Scalar(color));
					color++;
				}
			}
		}

		//�ֱ������ͨ���������ж��ٺ�ɫ����
		int * colorCount = new int[color];//����ÿ���ǰ�ɫ��ͨ�������������
		memset(colorCount, 0, sizeof(int) * color);//Initial the array
		//ͳ��ÿ����ͨ������ظ���
		for (int j = 0; j < threshold_result.rows; j++)
		{
			//�õ���j�е��׵�ַ  
			uchar* data = threshold_result.ptr<uchar>(j);
			//����ÿ�е�����  
			for (int i = 0; i < threshold_result.cols; i++)
			{
				if (data[i] != 255)//���������ز��ǰ�ɫ�ľ��ڵ�ǰ����ɫ��Ӧ������������+1
					colorCount[data[i]]++;
			}
		}
		//�����ͨ������ظ���С��ĳ���ض�ֵ�����ǾͰ������� ��㣬���߸����� ȥ��
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
		delete[] colorCount;//�ͷ�����
		colorCount = nullptr;

		//��ɫ����ʵ����OPENCV�з�װ�õĺ������Դ��Ľ�(���޸�)
		bitwise_not(threshold_result, threshold_result);

		//TODO  ԭ���ĸ�ʴЧ�����Ǻܺã����Գ��Ա�Ľ�һ��ȥ�뷽��
		//�������ͽ�һ������
		Mat elem = getStructuringElement(MORPH_RECT, Size(1, 1));
		erode(threshold_result, threshold_result, elem);

		//�Ŵ�ͼƬ
		Mat resize_img;
		resize(threshold_result, resize_img, Size(), 10, 10);


		////////////////////////////////////////////////////////////////////////////////////////////////////////
		//                   Recongnize the Img 
		////////////////////////////////////////////////////////////////////////////////////////////////////////

				//���������Ϳ��Զ�ͼ����зָ��ˡ��������ǵķ�����ֻ�ܶ�����һ��һ����ʶ����������Ҫ��ÿ�����ַָ�
				//����������˼��������findContours()�����ѻ��������ҳ�����Ȼ��ͨ������֤��ȷ���Ƿ�Ϊ���ֵ���������
				//	����Щͨ����֤������������ȥ����boundingRect()�ҳ����ǵİ�Χ�С�
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