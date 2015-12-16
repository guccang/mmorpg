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
	int size();
	int write(const char* data, int size);
	int read(char** data, int size);
	void close();
};