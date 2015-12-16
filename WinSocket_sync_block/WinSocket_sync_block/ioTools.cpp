#include "stdafx.h"
#include "ioTools.h"

fileHandle::fileHandle()
{
	strcpy_s(fileName, "./playerData.txt");
	of = new ofstream(fileName, ios::out | ios::app | ios::binary);
	is = new ifstream(fileName, ios::binary);
}
int fileHandle::write(const char* data, int size)
{
	of->write(data, size);
	of->flush();
	return size;
}
int fileHandle::size()
{
	int temp = is->tellg(); // 记录下当前位置
	is->seekg(0, ios_base::end); // 移动到文件尾
	int len = is->tellg(); // 取得当前位置的指针长度 即文件长度
	is->seekg(temp); // 移动到原来的位置
	return len;
}
int fileHandle::read(char** data, int size)
{
	is->read(*data, size);
	return size;
}
void fileHandle::close()
{
	of->close();
	is->close();
	delete of;
	delete is;
}