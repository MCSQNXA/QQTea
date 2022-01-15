#include "QQTea.h"
#include "ByteStream.h"

#include <iostream>


int main()
{
	ByteStream data;
	ByteStream key;

	for (int i = 0; i < 16; i++) {
		data.write(0);
		key.write(0);
	}

	QQTea tea;
	ByteStream encode = tea.encrypt(data, key);

	std::cout << encode.toHexString() << std::endl;


	QQTea t;
	ByteStream decode = t.decrypt(encode, key);
	
	std::cout << decode.toHexString() << std::endl;










}