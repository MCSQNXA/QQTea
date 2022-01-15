#pragma once

#include<map>
#include<string>

#define byte unsigned char

class ByteStream
{
private:
	int _size = 0;
	int _read = 0;
	int _data_size = 8;
	byte* data = NULL;
	std::map<byte*, int> buff;

public:
	ByteStream();
	ByteStream(int size);
	ByteStream(const byte* data, int len);
	ByteStream(const byte* data, int offset, int len);
	ByteStream(std::string hex);
	ByteStream(const ByteStream& c);
	~ByteStream();

protected:
	byte* newBuff(int len);

protected:
	ByteStream& push(int len);

public:
	int size();

public:
	int lengthOf(const byte* data);

public:
	int available();

public:
	ByteStream& reset();

public:
	bool load(std::string file);

public:
	bool save(std::string file);

public:
	int getReadIndex();

public:
	ByteStream& setReadIndex(int offset);

public:
	ByteStream& addReadIndex(int len);

public:
	const byte getByte();
	
public:
	short getShort();

public:
	int getShortUnsigned();

public:
	int getInt();

public:
	long long getIntUnsigned();

public:
	long long getLong();

public:
	const byte* getAll(int* relen);

public:
	const byte* getBytes(int len);

public:
	std::string getString(int len);

public:
	std::string getAllString();

public:
	std::string getHexString(int len);

public:
	std::string getAllHexString();

public:
	ByteStream getByteStream(int len);

public:
	ByteStream getAllByteStream();

public:
	const byte* read(int len);

public:
	const byte* readAll();

public:
	std::string readAllString();

public:
	std::string readAllHexString();

public:
	ByteStream readAllByteStream();

public:
	bool readBoolean();

public:
	byte readByte();

public:
	short readShort();

public:
	int readShortUnsigned();

public:
	int readInt();

public:
	long long readIntUnsigned();

public:
	long long readLong();

public:
	std::string readHex(int len);

public:
	std::string readString(int len);

public:
	ByteStream readByteStream(int len);

public:
	void write(byte b);

public:
	void write(const byte* data, int len);

public:
	void write(const byte* data, int offset, int len);

public:
	void writeStream(ByteStream* b);
	void writeStream(ByteStream& b);

public:
	void writeByteLenAndHex(const char* value);
	void writeByteLenAndHex(std::string& value);

public:
	void writeByteLenAndBytes(const byte* value, int len);

public:
	void writeByteLenAndBytes(ByteStream* value);
	void writeByteLenAndBytes(ByteStream& value);

public:
	void writeByteLenAndString(const char* value);
	void writeByteLenAndString(std::string& value);

public:
	void writeBoolean(bool value);

public:
	void writeShort(short value);

public:
	void writeShort(int value);

public:
	void writeShortLenAndHex(const char* value);
	void writeShortLenAndHex(std::string& value);

public:
	void writeShortLenAndBytes(const byte* value, int len);

public:
	void writeShortLenAndBytes(ByteStream* value);
	void writeShortLenAndBytes(ByteStream& value);

public:
	void writeShortLenAndString(const char* value);
	void writeShortLenAndString(std::string& value);

public:
	void writeInt(int value);

public:
	void writeInt(long long value);

public:
	void writeIntLenAndHex(const char* value);
	void writeIntLenAndHex(std::string& value);

public:
	void writeIntLenAndBytes(const byte* value, int len);

public:
	void writeIntLenAndBytes(ByteStream* value);
	void writeIntLenAndBytes(ByteStream& value);

public:
	void writeIntLenAndString(const char* value);
	void writeIntLenAndString(std::string& value);

public:
	void writeLong(long long value);

public:
	void writeLongLenAndHex(const char* value);
	void writeLongLenAndHex(std::string& value);

public:
	void writeLongLenAndBytes(const byte* value, int len);

public:
	void writeLongLenAndBytes(ByteStream* value);
	void writeLongLenAndBytes(ByteStream& value);

public:
	void writeLongLenAndString(const char* value);
	void writeLongLenAndString(std::string& value);

public:
	void writeHex(const char* value);
	void writeHex(std::string& value);

public:
	void writeString(const char* value);
	void writeString(std::string& value);

public:
	std::string toHexString();

public:
	std::string toHexString(const byte* hex, int len);

public:
	const byte* toBytes();

public:
	const byte* getData();

public:
	int getDataSize();

protected:
	const byte* getBytes(const char* value);
	const byte* getBytes(std::string& value);

public:
	static std::string getHexString(const byte* data, int len);
	static std::string getHexString(const byte* data, int offset, int len);

public:
	static std::string getHexString(ByteStream* data);
	static std::string getHexString(ByteStream& data);
};