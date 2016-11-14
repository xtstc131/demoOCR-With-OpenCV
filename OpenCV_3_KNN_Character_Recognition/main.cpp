#include<opencv2/opencv.hpp>
#include<vector>
using namespace std;
using namespace cv;

const int MIN_CONTOUR_AREA = 80;
const int RESIZED_IMAGE_WIDTH = 20;
const int RESIZED_IMAGE_HEIGHT = 30;
int main()
{
	Mat imgTrainingNumbers;         // 读入的训练图片
	Mat imgGrayscale;               // 转化出来的灰度图片 
	Mat imgBlurred;                 // 高斯模糊进行一波预处理所得的图片
	Mat imgThresh;                  // 二值化（黑白化）的图片
	Mat imgThreshCopy;              // 二值图片的拷贝

	vector<vector<Point> > ptContours;				 // declare contours vector
	vector<Vec4i> v4iHierarchy;                    // declare contours hierarchy

	Mat matClassificationInts;      // these are our training classifications, note we will have to perform some conversions before writing to file later

										// these are our training images, due to the data types that the KNN object KNearest requires, we have to declare a single Mat,
										// then append to it as though it's a vector, also we will have to perform some conversions before writing to file later
	Mat matTrainingImagesAsFlattenedFloats;

	// possible chars we are interested in are digits 0 through 9 and capital letters A through Z, put these in vector intValidChars
	vector<int> intValidChars = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

	imgTrainingNumbers = imread("training_chars.png");          // 读取等待训练的图片
	if (imgTrainingNumbers.empty()) {                               // 打不开就报错
		cout << "error: image not read from file\n\n";         // show error message on command line
		return(0);                                                  // and exit program
	}
	cvtColor(imgTrainingNumbers, imgGrayscale, CV_BGR2GRAY);
	GaussianBlur(imgGrayscale, imgBlurred, Size(5, 5), 0);
	adaptiveThreshold(imgGrayscale, imgThresh, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 11, 2);
	imshow("imgThresh", imgThresh);
	imgThreshCopy = imgThresh.clone();
	findContours(imgThreshCopy, ptContours, v4iHierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	vector<vector<Point>>::iterator Iter = ptContours.begin();
	while(Iter != ptContours.end())
	{
		if (contourArea(*Iter) > MIN_CONTOUR_AREA)
		{
			Rect boudingRect = boundingRect(*Iter);
			rectangle(imgTrainingNumbers, boudingRect.tl(), boudingRect.br(), Scalar(0, 0, 255));
			Mat matROI = imgThresh(boudingRect);
			Mat matROIResized;
			resize(matROI, matROIResized, cv::Size(RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT));

			imshow("matROI", matROI);                               // show ROI image for reference
			imshow("matROIResized", matROIResized);                 // show resized ROI image for reference
			imshow("imgTrainingNumbers", imgTrainingNumbers);
			int inChar = waitKey(0);
			if(inChar == 27)
			{
				return (0);
			}
			if (find(intValidChars.begin(), intValidChars.end(), inChar) != intValidChars.end())
			{
				matClassificationInts.push_back(inChar);
				Mat matImageFloat;
				matROIResized.convertTo(matImageFloat, CV_32FC1);//把样本转化为32位的矩阵
				Mat matImageFlattenedFloat = matImageFloat.reshape(1, 1);       // flatten
				matTrainingImagesAsFlattenedFloats.push_back(matImageFlattenedFloat);
			}
		}
		++Iter;
	}
	cout << "Train complete!!!" << endl;
	imshow("imgThresh", imgTrainingNumbers);
	FileStorage fsClassfications("classifications.xml", FileStorage::WRITE);
	if(fsClassfications.isOpened() == false)
	{
		cout << "unable to open classfications file"<<endl;
		return 0;
	}
	fsClassfications << "classifications" << matClassificationInts;
	fsClassfications.release();
	
	FileStorage fsTrainingImgs("images.xml", FileStorage::WRITE);
	if (fsTrainingImgs.isOpened() == false)
	{
		cout << "unable to open Training file" << endl;
		return 0;
	}
	fsTrainingImgs << "images" << matTrainingImagesAsFlattenedFloats;
	fsTrainingImgs.release();
	waitKey();
}