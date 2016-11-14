#include<iostream>
#include <vector>
#include<io.h>
using namespace std;
void getFile(string path,vector<string> & files)
{
	intptr_t hFile = 0;
	_finddata_t fileinfo;
	string p;
	if( (hFile = _findfirst(p.assign(path).append("\\*").c_str(),&fileinfo) )!= -1)
	{
		do
		{
			if(fileinfo.attrib & _A_SUBDIR)
			{
				if(strcmp(fileinfo.name,".") != 0 && strcmp(fileinfo.name,"..") != 0)
				{
					getFile(p.assign(path).append("\\").append(fileinfo.name), files);
				}
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		}
		while (_findnext(hFile,&fileinfo) == 0 );
		_findclose(hFile);
	}
}
int main()
{
	vector<string> files;
	getFile("C:\\Users\\mallox\\Documents\\Visual Studio 2015\\Projects\\opencv_test\\opencv_test\\code", files);
	int size = files.size();
	for(int i = 0 ; i < size ; ++i)
	{
		cout << files[i].c_str() << endl;
	}
	getchar();
}