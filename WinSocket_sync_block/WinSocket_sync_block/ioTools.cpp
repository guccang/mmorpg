#include "stdafx.h"
#include "ioTools.h"

fileHandle::fileHandle()
{
	strcpy_s(fileName, "./playerData.txt");
	of = new ofstream(fileName, ios::out | ios::app | ios::binary);
	is = new ifstream(fileName, ios::binary);
}

fileHandle::fileHandle(char* name)
{
	strcpy_s(fileName, name);
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
	std::streamoff temp = is->tellg(); // ��¼�µ�ǰλ��
	is->seekg(0, ios_base::end); // �ƶ����ļ�β
	std::streamoff len = is->tellg(); // ȡ�õ�ǰλ�õ�ָ�볤�� ���ļ�����
	is->seekg(temp); // �ƶ���ԭ����λ��
	return (int)len;
}
int fileHandle::read(char** data, int size)
{
	is->read(*data, size);
	return size;
}
int fileHandle::readByte(char* data, int size)
{
	is->read(data, size);
	return size;
}

void fileHandle::close()
{
	of->close();
	is->close();
	delete of;
	delete is;
}