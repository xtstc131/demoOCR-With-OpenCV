// Tesseract3.2.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#pragma comment(lib, "public/lib/libtesseract302.lib")
#pragma comment(lib, "lib/liblept.lib")
#include "public/include/tesseract/baseapi.h"
#include "public/include/tesseract/basedir.h"
#include "public/include/tesseract/strngs.h"
#include "include/leptonica/allheaders.h"

//到谷歌官网下载tesseract-3.02.02-win32-lib-include-dirs.zip
//以及leptonica-1.68-win32-lib-include-dirs.zip

void ConvertUtf8ToGBK(char **amp,char *strUtf8)
{
	int len=MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)strUtf8, -1, NULL,0); 
	unsigned short * wszGBK = new unsigned short[len+1]; 
	memset(wszGBK, 0, len * 2 + 2); 
	MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)strUtf8, -1, (LPWSTR)wszGBK, len); 
	len = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wszGBK, -1, NULL, 0, NULL, NULL); 
	//char *szGBK=new char[len + 1]; 
	*amp = new char[len+1];
	memset(*amp, 0, len + 1); 
	WideCharToMultiByte (CP_ACP, 0, (LPCWSTR)wszGBK, -1, *amp, len, NULL,NULL); 
	//amp=szGBK; 
}

int _tmain(int argc, _TCHAR* argv[])
{
	char *filepath = "7.bmp";
	tesseract::TessBaseAPI api;
	//api.SetOutputName("output");
	api.SetInputName(filepath);
	api.SetPageSegMode(tesseract::PSM_AUTO);
	int nRet = api.Init("tessdata", "eng");
	if (nRet != 0)
	{
#if _DEBUG
		printf("初始化字库失败！");
#endif
		return 0;
	}

	PIX *pix = pixRead(filepath);
	api.SetImage(pix);


	char *pdata = NULL;
	char *pResult = NULL;
	api.SetRectangle(0, 0, 400, 80);
	pdata = api.GetUTF8Text();
	ConvertUtf8ToGBK(&pResult, pdata); //对结果转码
#if _DEBUG
	printf("识别结果:%s", pResult);
#endif
	delete pResult;
	pixDestroy(&pix);
	api.Clear();
	api.End();
	return 0;
}

