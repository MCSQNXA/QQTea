#pragma once

#include <vector>
#include <time.h>
#include "ByteStream.h"

#define byte unsigned char

class QQTea
{
public:
	~QQTea();

private:
	std::vector<byte*> buff;

private:
	void clearBuff();

	// ָ��ǰ�����Ŀ�
private:
	byte* plain = 0;

	// ��ָ��ǰ��һ�����Ŀ�
private:
	byte* prePlain = 0;

	// ��������Ļ�������
private:
	byte* out = 0;

	// ��ǰ���ܵ�����λ�ú���һ�μ��ܵ����Ŀ�λ�ã��������8
private:
	int crypt = 0, preCrypt = 0;

	// ��ǰ����ļ��ܽ��ܿ��λ��
private:
	int pos = 0;

	// �����
private:
	int padding = 0;

	// ��Կ
private:
	const byte* key = 0;

	// ���ڼ���ʱ����ʾ��ǰ�Ƿ��ǵ�һ��8�ֽڿ飬��Ϊ�����㷨�Ƿ�����
	// �����ʼ��8���ֽ�û�з������ã�������Ҫ�����������
private:
	bool header = true;

	// �����ʾ��ǰ���ܿ�ʼ��λ�ã�֮����Ҫ��ôһ��������Ϊ�˱��⵱���ܵ����ʱ
	// �����Ѿ�û�����ݣ���ʱ��ͻ��������������������ж����������ó���
private:
	int contextStart = 0;

private:
	ByteStream baos;

public:
	ByteStream encrypt(ByteStream& data, ByteStream& key);
	ByteStream decrypt(ByteStream& data, ByteStream& key);

private:
	const byte* encrypt(const byte* in, int offset, int len, const byte* k, int* relen);
	const byte* encipher(const byte* in);

private:
	void writeInt(int t);

private:
	void encrypt8Bytes();
	bool decrypt8Bytes(const byte* in, int offset, int len);

private:
	int random() {
		srand((unsigned int)time(NULL));

		return rand();
	}

private:
	long long getUnsignedInt(const byte* in, int offset, int len);

private:
	void arraycopy(const byte* src, int srcPos, byte* dest, int destPos, int length);

private:
	const byte* decrypt(const byte* in, int offset, int len, const byte* k, int* relen);
	const byte* decipher(const byte* in, int offset);
};