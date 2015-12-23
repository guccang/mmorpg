#pragma  once
#include <iostream>
#include <fstream>
using namespace std;

class fileHandle
{
	char fileName[128];
	ofstream *of = NULL;
	ifstream *is = NULL;
public :
	fileHandle();
	fileHandle(char* fileName);
	int size();
	int write(const char* data, int size);
	int read(char** data, int size);
	int readByte(char*, int size);
	void close();
};