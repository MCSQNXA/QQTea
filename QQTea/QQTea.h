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

	// 指向当前的明文块
private:
	byte* plain = 0;

	// 这指向前面一个明文块
private:
	byte* prePlain = 0;

	// 输出的密文或者明文
private:
	byte* out = 0;

	// 当前加密的密文位置和上一次加密的密文块位置，他们相差8
private:
	int crypt = 0, preCrypt = 0;

	// 当前处理的加密解密块的位置
private:
	int pos = 0;

	// 填充数
private:
	int padding = 0;

	// 密钥
private:
	const byte* key = 0;

	// 用于加密时，表示当前是否是第一个8字节块，因为加密算法是反馈的
	// 但是最开始的8个字节没有反馈可用，所有需要标明这种情况
private:
	bool header = true;

	// 这个表示当前解密开始的位置，之所以要这么一个变量是为了避免当解密到最后时
	// 后面已经没有数据，这时候就会出错，这个变量就是用来判断这种情况免得出错
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