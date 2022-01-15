#include "ByteStream.h"

ByteStream::ByteStream()
{
	this->data = new byte[this->_data_size];
}

ByteStream::ByteStream(int size)
{
	this->_data_size = (size < 1 ? 8 : size);

	this->data = new byte[this->_data_size];
}

ByteStream::ByteStream(const byte* data, int len)
{
	this->data = new byte[this->_data_size];

	this->write(data, len);
}

ByteStream::ByteStream(const byte* data, int offset, int len)
{
	this->data = new byte[this->_data_size];

	this->write(data, offset, len);
}

ByteStream::ByteStream(std::string hex)
{
	this->data = new byte[this->_data_size];

	this->writeHex(hex);
}

ByteStream::ByteStream(const ByteStream& c)
{
	this->_read = c._read;
	this->_data_size = c._data_size;

	this->data = new byte[c._data_size];

	this->write(c.data, 0, c._size);
}

ByteStream::~ByteStream()
{
	delete[] this->data;

	for (std::map<byte*, int>::iterator i = this->buff.begin(); i != this->buff.end(); i++) {
		delete[] i->first;
	}

	this->buff.clear();
}

byte* ByteStream::newBuff(int len)
{
	byte* buff = NULL;

	if (len > 0) {
		this->buff[buff = new byte[len]] = len;
	}

	return buff;
}

ByteStream& ByteStream::push(int len)
{
	if (this->_data_size <= this->_size + len) {
		byte* buffer = new byte[this->_data_size = (this->_data_size + ((long long)len << 1))];

		for (int i = 0; i < this->_size; i++) {
			buffer[i] = this->data[i];
		}

		delete[] this->data; this->data = buffer;
	}

	return *this;
}

int ByteStream::size()
{
	return this->_size;
}

int ByteStream::lengthOf(const byte* data)
{
	if (data == this->data) {
		return this->_size;
	}

	return this->buff.count((byte*)data) == 0 ? 0 : this->buff[(byte*)data];
}

int ByteStream::available()
{
	return this->_data_size - this->_size;
}

ByteStream& ByteStream::reset()
{
	if (this->_size > 0) {
		this->_size = 0;
		this->_read = 0;

		delete[] this->data; this->data = new byte[this->_data_size = 8];
	}

	return *this;
}

bool ByteStream::load(std::string file)
{
	FILE* flags;
	fopen_s(&flags, file.c_str(), "rb");

	if (flags == NULL) {
		return false;
	}

	fseek(flags, 0, SEEK_END);
	int len = (int)ftell(flags);
	fseek(flags, 0, SEEK_SET);

	if (len == 0) {
		fclose(flags); return true;
	}

	byte* data = new byte[len];

	fread(data, sizeof(byte), len, flags);
	fclose(flags);

	this->write(data, 0, len); delete[] data;

	return true;
}

bool ByteStream::save(std::string file)
{
	FILE* flags;
	fopen_s(&flags, file.c_str(), "wb");

	if (flags == NULL) {
		return false;
	}

	fwrite(this->data, sizeof(byte), this->_size, flags);
	fflush(flags);
	fclose(flags);

	return true;
}

int ByteStream::getReadIndex()
{
	return this->_read;
}

ByteStream& ByteStream::setReadIndex(int offset)
{
	if (offset > -1) {
		this->_read = offset;
	}

	return *this;
}

ByteStream& ByteStream::addReadIndex(int len)
{
	this->_read += len; return *this;
}

const byte ByteStream::getByte()
{
	return this->_read < this->_data_size ? this->data[this->_read] : 0;
}

short ByteStream::getShort()
{
	return (short)this->getShortUnsigned();
}

int ByteStream::getShortUnsigned()
{
	int value = 0;
	const byte* bytes = this->getBytes(2);

	if (bytes != NULL) {
		for (int i = 0; i < 2; i++) {
			value += (bytes[i] & 0xff) << ((1 - i) * 8);
		}
	}

	return value;
}

int ByteStream::getInt()
{
	return (int)this->getIntUnsigned();
}

long long ByteStream::getIntUnsigned()
{
	unsigned long value = 0;
	const byte* bytes = this->getBytes(4);

	if (bytes != NULL) {
		for (int i = 0; i < 4; i++) {
			value += (bytes[i] & 0xff) << ((3 - i) * 8);
		}
	}

	return value;
}

long long ByteStream::getLong()
{
	unsigned long value = 0;
	const byte* bytes = this->getBytes(8);

	if (bytes != NULL) {
		for (int i = 0; i < 8; i++) {
			value += (bytes[i] & 0xff) << ((7 - i) * 8);
		}
	}

	return value;
}

const byte* ByteStream::getAll(int* relen)
{
	const byte* data = this->getBytes(*relen = (this->_size - this->_read));

	if (data == NULL) {
		*relen = 0;
	}

	return data;
}

const byte* ByteStream::getBytes(int len)
{
	if (this->_read + len <= this->_size && len > 0) {
		byte* data = this->newBuff(len);

		for (int i = 0; i < len; i++) {
			data[i] = this->data[this->_read + i];
		}

		return data;
	}

	return NULL;
}

std::string ByteStream::getString(int len)
{
	const byte* data = this->getBytes(len);

	if (data == NULL) {
		return "";
	}

	return std::string((char*)data, len);
}

std::string ByteStream::getAllString()
{
	int len = 0;
	const byte* data = this->getAll(&len);

	return std::string((char*)data, len);
}

std::string ByteStream::getHexString(int len)
{
	return this->toHexString(this->getBytes(len), len);
}

std::string ByteStream::getAllHexString()
{
	int len = 0;
	const byte* data = this->getAll(&len);

	return this->toHexString(data, len);
}

ByteStream ByteStream::getByteStream(int len)
{
	ByteStream re;
	re.write(this->getBytes(len), 0, len);

	return re;
}

ByteStream ByteStream::getAllByteStream()
{
	int len = 0;
	const byte* data = this->getAll(&len);

	ByteStream stream;
	stream.write(data, 0, len);

	return stream;
}

const byte* ByteStream::read(int len)
{
	if (len < 1 || len > this->_size || this->_read + len > this->_size) {
		return NULL;
	}

	byte* buffer = this->newBuff(len);

	for (int i = 0; i < len; ) {
		buffer[i++] = this->data[this->_read++];
	}

	return buffer;
}

const byte* ByteStream::readAll()
{
	return this->read(this->_size - this->_read);
}

std::string ByteStream::readAllString()
{
	const byte* data = this->readAll();

	return std::string((char*)data, this->lengthOf(data));
}

std::string ByteStream::readAllHexString()
{
	const byte* data = this->readAll();

	return this->toHexString(data, this->lengthOf(data));
}

ByteStream ByteStream::readAllByteStream()
{
	const byte* data = this->readAll();

	ByteStream stream;
	stream.write(data, 0, this->lengthOf(data));

	return stream;
}

bool ByteStream::readBoolean()
{
	const byte* data = this->read(1);

	if (data == NULL) {
		return false;
	}

	return data[0] == 1;
}

byte ByteStream::readByte()
{
	const byte* data = this->read(1);

	if (data == NULL) {
		return 0;
	}

	return data[0];
}

short ByteStream::readShort()
{
	short value = 0;
	const byte* data = this->read(2);

	if (data != NULL) {
		for (int i = 0; i < 2; i++) {
			value += (data[i] & 0xff) << ((1 - i) * 8);
		}
	}

	return value;
}

int ByteStream::readShortUnsigned()
{
	int value = 0;
	const byte* data = this->read(2);

	if (data != NULL) {
		for (int i = 0; i < 2; i++) {
			value += (data[i] & 0xff) << ((1 - i) * 8);
		}
	}

	return value;
}

int ByteStream::readInt()
{
	int value = 0;
	const byte* data = this->read(4);

	if (data != NULL) {
		for (int i = 0; i < 4; i++) {
			value += (data[i] & 0xff) << ((3 - i) * 8);
		}
	}

	return value;
}

long long ByteStream::readIntUnsigned()
{
	long long value = 0;
	const byte* data = this->read(4);

	if (data != NULL) {
		for (int i = 0; i < 4; i++) {
			value += (long long)(data[i] & 0xff) << ((3 - i) * 8);
		}
	}

	return value;
}

long long ByteStream::readLong()
{
	long long value = 0;
	const byte* data = this->read(8);

	if (data != NULL) {
		for (int i = 0; i < 8; i++) {
			value += (long long)(data[i] & 0xff) << ((7 - i) * 8);
		}
	}

	return value;
}

std::string ByteStream::readHex(int len)
{
	const byte* data = this->read(len);

	if (data == NULL) {
		return "";
	}

	return this->toHexString(data, len);
}

std::string ByteStream::readString(int len)
{
	const byte* data = this->read(len);

	if (data == NULL) {
		return "";
	}

	return std::string((char*)data, len);
}

ByteStream ByteStream::readByteStream(int len)
{
	ByteStream stream;
	const byte* data = this->read(len);

	if (data != NULL) {
		stream.write(data, 0, len);
	}

	return stream;
}

void ByteStream::write(byte b)
{
	this->push(1);
	this->data[this->_size++] = b;
}

void ByteStream::write(const byte* data, int len)
{
	this->write(data, 0, len);
}

void ByteStream::write(const byte* data, int offset, int len)
{
	if (data == NULL || offset < 0 || len < 1) {
		return;
	}

	this->push(len);

	for (int i = 0; i < len; i++) {
		this->data[this->_size++] = data[offset++];
	}
}

void ByteStream::writeStream(ByteStream* b)
{
	if (b != NULL && b->size() > 0) {
		this->write(b->getData(), 0, b->size());
	}
}

void ByteStream::writeStream(ByteStream& b)
{
	this->writeStream(&b);
}

void ByteStream::writeByteLenAndHex(const char* value)
{
	const byte* data = this->getBytes(value);

	if (this->lengthOf(data) > 0) {
		this->writeByteLenAndBytes(data, this->lengthOf(data));
	}
}

void ByteStream::writeByteLenAndHex(std::string& value)
{
	this->writeByteLenAndHex(value.c_str());
}

void ByteStream::writeByteLenAndBytes(const byte* value, int len)
{
	if (value != NULL && len > 0) {
		this->write((byte)len);
		this->write(value, 0, len);
	}
}

void ByteStream::writeByteLenAndBytes(ByteStream* value)
{
	if (value != NULL && value->size() > 0) {
		this->writeByteLenAndBytes(value->getData(), value->size());
	}
}

void ByteStream::writeByteLenAndBytes(ByteStream& value)
{
	this->writeByteLenAndBytes(&value);
}

void ByteStream::writeByteLenAndString(const char* value)
{
	if (value != NULL) {
		this->writeByteLenAndBytes((byte*)value, (int)strlen(value));
	}
}

void ByteStream::writeByteLenAndString(std::string& value)
{
	this->writeByteLenAndString(value.c_str());
}

void ByteStream::writeBoolean(bool value)
{
	this->write((byte)(value ? 1 : 0));
}

void ByteStream::writeShort(short value)
{
	this->write((byte)((value >> 8) & 0xff));
	this->write((byte)(value & 0xff));
}

void ByteStream::writeShort(int value)
{
	this->write((byte)((value >> 8) & 0xff));
	this->write((byte)(value & 0xff));
}

void ByteStream::writeShortLenAndHex(const char* value)
{
	const byte* data = this->getBytes(value);

	if (this->lengthOf(data) > 0) {
		this->writeShortLenAndBytes(data, this->lengthOf(data));
	}
}

void ByteStream::writeShortLenAndHex(std::string& value)
{
	this->writeShortLenAndHex(value.c_str());
}

void ByteStream::writeShortLenAndBytes(const byte* value, int len)
{
	if (value != NULL && len > 0) {
		this->writeShort(len);
		this->write(value, 0, len);
	}
}

void ByteStream::writeShortLenAndBytes(ByteStream* value)
{
	if (value != NULL && value->size() > 0) {
		this->writeShortLenAndBytes(value->getData(), value->size());
	}
}

void ByteStream::writeShortLenAndBytes(ByteStream& value)
{
	this->writeShortLenAndBytes(&value);
}

void ByteStream::writeShortLenAndString(const char* value)
{
	if (value != NULL) {
		const int len = (int)strlen(value);

		if (len > 0) {
			this->writeShortLenAndBytes((byte*)value, len);
		}
	}
}

void ByteStream::writeShortLenAndString(std::string& value)
{
	this->writeShortLenAndString(value.c_str());
}

void ByteStream::writeInt(int value)
{
	this->write((byte)((value >> 24) & 0xff));
	this->write((byte)((value >> 16) & 0xff));
	this->write((byte)((value >> 8) & 0xff));
	this->write((byte)(value & 0xff));
}

void ByteStream::writeInt(long long value)
{
	this->write((byte)((value >> 24) & 0xff));
	this->write((byte)((value >> 16) & 0xff));
	this->write((byte)((value >> 8) & 0xff));
	this->write((byte)(value & 0xff));
}

void ByteStream::writeIntLenAndHex(const char* value)
{
	const byte* data = this->getBytes(value);

	if (this->lengthOf(data) > 0) {
		this->writeIntLenAndBytes(data, this->lengthOf(data));
	}
}

void ByteStream::writeIntLenAndHex(std::string& value)
{
	this->writeIntLenAndHex(value.c_str());
}

void ByteStream::writeIntLenAndBytes(const byte* value, int len)
{
	if (value != NULL && len > 0) {
		this->writeInt(len);
		this->write(value, 0, len);
	}
}

void ByteStream::writeIntLenAndBytes(ByteStream* value)
{
	if (value != NULL && value->size() > 0) {
		this->writeIntLenAndBytes(value->getData(), value->size());
	}
}

void ByteStream::writeIntLenAndBytes(ByteStream& value)
{
	this->writeIntLenAndBytes(&value);
}

void ByteStream::writeIntLenAndString(const char* value)
{
	if (value != NULL) {
		const int len = (int)strlen(value);

		if (len > 0) {
			this->writeIntLenAndBytes((byte*)value, len);
		}
	}
}

void ByteStream::writeIntLenAndString(std::string& value)
{
	this->writeIntLenAndString(value.c_str());
}

void ByteStream::writeLong(long long value)
{
	this->write((byte)((value >> 56) & 0xff));
	this->write((byte)((value >> 48) & 0xff));
	this->write((byte)((value >> 40) & 0xff));
	this->write((byte)((value >> 32) & 0xff));
	this->write((byte)((value >> 24) & 0xff));
	this->write((byte)((value >> 16) & 0xff));
	this->write((byte)((value >> 8) & 0xff));
	this->write((byte)(value & 0xff));
}

void ByteStream::writeLongLenAndHex(const char* value)
{
	const byte* data = this->getBytes(value);

	if (this->lengthOf(data) > 0) {
		this->writeLongLenAndBytes(data, this->lengthOf(data));
	}
}

void ByteStream::writeLongLenAndHex(std::string& value)
{
	this->writeLongLenAndHex(value.c_str());
}

void ByteStream::writeLongLenAndBytes(const byte* value, int len)
{
	if (value != NULL && len > 0) {
		this->writeLong(len);
		this->write(value, 0, len);
	}
}

void ByteStream::writeLongLenAndBytes(ByteStream* value)
{
	if (value != NULL && value->size() > 0) {
		this->writeLongLenAndBytes(value->getData(), value->size());
	}
}

void ByteStream::writeLongLenAndBytes(ByteStream& value)
{
	this->writeLongLenAndBytes(&value);
}

void ByteStream::writeLongLenAndString(const char* value)
{
	if (value != NULL) {
		const int len = (int)strlen(value);

		if (len == 0) {
			return;
		}

		this->writeLongLenAndBytes((byte*)value, len);
	}
}

void ByteStream::writeLongLenAndString(std::string& value)
{
	this->writeLongLenAndString(value.c_str());
}

void ByteStream::writeHex(const char* value)
{
	const byte* data = this->getBytes(value);

	if (this->lengthOf(data) > 0) {
		this->write(data, 0, this->lengthOf(data));
	}
}

void ByteStream::writeHex(std::string& value)
{
	this->writeHex(value.c_str());
}

void ByteStream::writeString(const char* value)
{
	if (value != NULL) {
		const int len = (int)strlen(value);

		if (len == 0) {
			return;
		}

		this->write((byte*)value, 0, len);
	}
}

void ByteStream::writeString(std::string& value)
{
	this->writeString(value.c_str());
}

std::string ByteStream::toHexString()
{
	return this->toHexString(this->getData(), this->size());
}

std::string ByteStream::toHexString(const byte* hex, int len)
{
	if (hex == NULL || len < 1) {
		return "";
	}

	std::string string;
	char chars[] = {
			'0', '1', '2', '3',
			'4', '5', '6', '7',
			'8', '9', 'A', 'B',
			'C', 'D', 'E', 'F' };

	for (int i = 0; i < len; i++) {
		string.append(1, chars[hex[i] >> 4 & 0xF]);
		string.append(1, chars[hex[i] & 0xF]);

		if (i + 1 < len) {
			string.append(" ");
		}
	}

	return string;
}

const byte* ByteStream::toBytes()
{
	if (this->_size < 1 || this->_size == this->_data_size) {
		return this->data;
	}

	byte* buffer = new byte[this->_size];

	for (int i = 0; i < this->_size; i++) {
		buffer[i] = this->data[i];
	}

	delete[] this->data; this->data = buffer; return this->data;
}

const byte* ByteStream::getData()
{
	return this->data;
}

int ByteStream::getDataSize()
{
	return this->_data_size;
}

const byte* ByteStream::getBytes(const char* value)
{
	if (value == NULL) {
		return NULL;
	}

	std::string trim;
	int len = (int)strlen(value);

	for (int i = 0; i < len; i++) {
		if (value[i] != ' ') {
			switch (value[i]) {
			case 'a':
				trim.append("A");
				break;
			case 'b':
				trim.append("B");
				break;
			case 'c':
				trim.append("C");
				break;
			case 'd':
				trim.append("D");
				break;
			case 'e':
				trim.append("E");
				break;
			case 'f':
				trim.append("F");
				break;
			default:
				trim.append(1, value[i]);
			}
		}
	}

	if (trim.length() > 1 && trim.length() % 2 == 0) {
		std::string hex = "0123456789ABCDEF";

		int length = (int)(trim.length() >> 1);
		byte* buffer = this->newBuff(length);

		for (int i = 0; i < length; i++) {
			buffer[i] = (byte)(hex.find(trim.at((long long)i * 2)) << 4
				| (byte)hex.find(trim.at((long long)i * 2 + 1)));
		}

		return buffer;
	}

	return NULL;
}

const byte* ByteStream::getBytes(std::string& value)
{
	return this->getBytes(value.c_str());
}

std::string ByteStream::getHexString(const byte* data, int len)
{
	return ByteStream::getHexString(data, 0, len);
}

std::string ByteStream::getHexString(const byte* data, int offset, int len)
{
	ByteStream stream;
	stream.write(data, offset, len);

	return stream.toHexString();
}

std::string ByteStream::getHexString(ByteStream* data)
{
	if (data == NULL || data->size() == 0) {
		return "";
	}

	return ByteStream::getHexString(data->getData(),0, data->size());
}

std::string ByteStream::getHexString(ByteStream& data)
{
	return ByteStream::getHexString(&data);
}