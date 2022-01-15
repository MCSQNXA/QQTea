#include "QQTea.h"

QQTea::~QQTea()
{
	this->clearBuff();
}

void QQTea::clearBuff()
{
	for (size_t i = 0; i < this->buff.size(); i++) {
		delete[] this->buff[i];
	}

	this->buff.clear();
}

ByteStream QQTea::encrypt(ByteStream& data, ByteStream& key)
{
	ByteStream re;

	if (data.size() > 0 && key.size() == 16) {
		int len = 0;
		const byte* encrpt = this->encrypt(data.toBytes(), 0, data.size(), key.toBytes(), &len);

		re.write(encrpt, 0, len);
	}

	this->clearBuff();

	return re;
}

ByteStream QQTea::decrypt(ByteStream& data, ByteStream& key)
{
	ByteStream re;

	if (data.size() > 0 && key.size() == 16) {
		int len = 0;
		const byte* decrpt = this->decrypt(data.toBytes(), 0, data.size(), key.toBytes(), &len);

		re.write(decrpt, 0, len);
	}

	this->clearBuff();

	return re;
}

const byte* QQTea::encrypt(const byte* in, int offset, int len, const byte* k, int* relen)
{
	plain = new byte[8]; this->buff.push_back(plain);
	prePlain = new byte[8]; this->buff.push_back(prePlain);
	pos = 1;
	padding = 0;
	crypt = 0;
	preCrypt = 0;
	this->key = k;
	header = true;

	// 计算头部填充字节数
	pos = (len + 0x0A) % 8;

	if (pos != 0) {
		pos = 8 - pos;
	}

	// 计算输出的密文长度
	out = new byte[*relen = (long long)len + pos + 10]; this->buff.push_back(out);
	// 这里的操作把pos存到了plain的第一个字节里面
	// 0xF8后面三位是空的，正好留给pos，因为pos是0到7的值，表示文本开始的字节位置
	plain[0] = (byte)((random() & 0xF8) | pos);

	// 这里用随机产生的数填充plain[1]到plain[pos]之间的内容
	for (int i = 1; i <= pos; i++) {
		plain[i] = (byte)(random() & 0xFF);
	}

	pos++;
	// 这个就是prePlain，第一个8字节块当然没有prePlain，所以我们做一个全0的给第一个8字节块
	for (int i = 0; i < 8; i++) {
		prePlain[i] = 0x0;
	}

	// 继续填充2个字节的随机数，这个过程中如果满了8字节就加密之
	padding = 1;
	while (padding <= 2)
	{
		if (pos < 8)
		{
			plain[pos++] = (byte)(random() & 0xFF);
			padding++;
		}
		if (pos == 8) {
			encrypt8Bytes();
		}
	}

	// 头部填充完了，这里开始填真正的明文了，也是满了8字节就加密，一直到明文读完
	int i = offset;
	while (len > 0)
	{
		if (pos < 8)
		{
			plain[pos++] = in[i++];
			len--;
		}

		if (pos == 8) {
			encrypt8Bytes();
		}
	}

	// 最后填上0，以保证是8字节的倍数
	padding = 1;
	while (padding <= 7)
	{
		if (pos < 8)
		{
			plain[pos++] = 0x0;
			padding++;
		}
		if (pos == 8) {
			encrypt8Bytes();
		}
	}

	return out;
}

const byte* QQTea::encipher(const byte* in)
{
	// 迭代次数，16次
	int loop = 0x10;
	// 得到明文和密钥的各个部分，注意java没有无符号类型，所以为了表示一个无符号的整数
	// 我们用了long，这个long的前32位是全0的，我们通过这种方式模拟无符号整数，后面用到的long也都是一样的
	// 而且为了保证前32位为0，需要和0xFFFFFFFF做一下位与            
	long long y = getUnsignedInt(in, 0, 4);
	long long z = getUnsignedInt(in, 4, 4);
	long long a = getUnsignedInt(key, 0, 4);
	long long b = getUnsignedInt(key, 4, 4);
	long long c = getUnsignedInt(key, 8, 4);
	long long d = getUnsignedInt(key, 12, 4);
	// 这是算法的一些控制变量，为什么delta是0x9E3779B9呢？
	// 这个数是TEA算法的delta，实际是就是(sqr(5) - 1) * 2^31 (根号5，减1，再乘2的31次方)
	long long sum = 0;
	long long delta = 0x9E3779B9;
	delta &= 0xFFFFFFFFL;

	// 开始迭代了，乱七八糟的，我也看不懂，反正和DES之类的差不多，都是这样倒来倒去
	while (loop-- > 0)
	{
		sum += delta;
		sum &= 0xFFFFFFFFL;
		y += ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
		y &= 0xFFFFFFFFL;
		z += ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);
		z &= 0xFFFFFFFFL;
	}

	// 最后，我们输出密文，因为我用的long，所以需要强制转换一下变成int
	baos.reset();
	writeInt((int)y);
	writeInt((int)z);

	return baos.toBytes();
}

void QQTea::writeInt(int t)
{
	baos.write(t >> 24);
	baos.write(t >> 16);
	baos.write(t >> 8);
	baos.write(t);
}

void QQTea::encrypt8Bytes()
{
    // 这部分完成我上面所说的 plain ^ preCrypt，注意这里判断了是不是第一个8字节块，如果是的话，那个prePlain就当作preCrypt用
    for (pos = 0; pos < 8; pos++)
    {
        if (header) {
            plain[pos] ^= prePlain[pos];
        }
        else {
            plain[pos] ^= out[preCrypt + pos];
        }
    }

    // 这个完成我上面说的 f(plain ^ preCrypt)
    const byte* crypted = encipher(plain);
    // 这个没什么，就是拷贝一下，java不像c，所以我只好这么干，c就不用这一步了
    arraycopy(crypted, 0, out, crypt, 8);

    // 这个完成了 f(plain ^ preCrypt) ^ prePlain，ok，下面拷贝一下就行了
    for (pos = 0; pos < 8; pos++) {
        out[crypt + pos] ^= prePlain[pos];
    }

    arraycopy(plain, 0, prePlain, 0, 8);

    // 完成了加密，现在是调整crypt，preCrypt等等东西的时候了
    preCrypt = crypt;
    crypt += 8;
    pos = 0;
    header = false;
}

bool QQTea::decrypt8Bytes(const byte* in, int offset, int len)
{
	// 这里第一步就是判断后面还有没有数据，没有就返回，如果有，就执行 crypt ^ prePlain
	for (pos = 0; pos < 8; pos++)
	{
		if (contextStart + pos >= len) {
			return true;
		}

		prePlain[pos] ^= in[offset + crypt + pos];
	}

	// 好，这里执行到了 d(crypt ^ prePlain)
	prePlain = (byte*)decipher(prePlain, 0);
	if (prePlain == NULL) {
		return false;
	}

	// 解密完成，最后一步好像没做？ 
	// 这里最后一步放到decrypt里面去做了，因为解密的步骤有点不太一样
	// 调整这些变量的值先
	contextStart += 8;
	crypt += 8;
	pos = 0;
	return true;
}

long long QQTea::getUnsignedInt(const byte* in, int offset, int len)
{
    long long ret = 0L;
    int end = len > 8 ? offset + 8 : offset + len;

    for (int i = offset; i < end; i++)
    {
        ret <<= 8;
        ret |= in[i] & 0xff;
    }

	return  ret & 0xffffffffL | (ret >> 32 & 0xff);
}

void QQTea::arraycopy(const byte* src, int srcPos, byte* dest, int destPos, int length)
{
    for (int i = 0; i < length; i++) {
		dest[destPos++] = src[srcPos++];
    }
}

const byte* QQTea::decrypt(const byte* in, int offset, int len, const byte* k, int* relen)
{
	crypt = 0;
	preCrypt = 0;
	this->key = k;
	int count;
	byte* m = new byte[(long long)offset + 8]; int mlen = offset + 8; this->buff.push_back(m);

	// 因为QQ消息加密之后至少是16字节，并且肯定是8的倍数，这里检查这种情况
	if ((len % 8 != 0) || (len < 16))
	{
		*relen = 0; return NULL;
	}

	// 得到消息的头部，关键是得到真正明文开始的位置，这个信息存在第一个字节里面，所以其用解密得到的第一个字节与7做与
	prePlain = (byte*)decipher(in, offset);
	pos = prePlain[0] & 0x7;
	// 得到真正明文的长度
	count = len - pos - 10;
	// 如果明文长度小于0，那肯定是出错了，比如传输错误之类的，返回
	if (count < 0) {
		*relen = 0; return NULL;
	}

	// 这个是临时的preCrypt，和加密时第一个8字节块没有prePlain一样，解密时
	// 第一个8字节块也没有preCrypt，所有这里建一个全0的
	for (int i = offset; i < mlen; i++) {
		m[i] = 0;
	}

	// 通过了上面的代码，密文应该是没有问题了，我们分配输出缓冲区
	out = new byte[*relen = count]; this->buff.push_back(out);
	// 设置preCrypt的位置等于0，注意目前的preCrypt位置是指向m的，因为java没有指针，所以我们在后面要控制当前密文buf的引用
	preCrypt = 0;
	// 当前的密文位置，为什么是8不是0呢？注意前面我们已经解密了头部信息了，现在当然该8了
	crypt = 8;
	// 自然这个也是8
	contextStart = 8;
	// 加1，和加密算法是对应的
	pos++;

	// 开始跳过头部，如果在这个过程中满了8字节，则解密下一块
	// 因为是解密下一块，所以我们有一个语句 m = in，下一块当然有preCrypt了，我们不再用m了
	// 但是如果不满8，这说明了什么？说明了头8个字节的密文是包含了明文信息的，当然还是要用m把明文弄出来
	// 所以，很显然，满了8的话，说明了头8个字节的密文除了一个长度信息有用之外，其他都是无用的填充
	padding = 1;
	while (padding <= 2)
	{
		if (pos < 8)
		{
			pos++;
			padding++;
		}

		if (pos == 8)
		{
			m = (byte*)in;
			if (!decrypt8Bytes(in, offset, len)) {
				*relen = 0; return NULL;
			}
		}
	}

	// 这里是解密的重要阶段，这个时候头部的填充都已经跳过了，开始解密
	// 注意如果上面一个while没有满8，这里第一个if里面用的就是原始的m，否则这个m就是in了
	int i = 0;
	while (count != 0)
	{
		if (pos < 8)
		{
			out[i] = (byte)(m[offset + preCrypt + pos] ^ prePlain[pos]);
			i++;
			count--;
			pos++;
		}
		if (pos == 8)
		{
			m = (byte*)in;
			preCrypt = crypt - 8;

			if (!decrypt8Bytes(in, offset, len)) {
				*relen = 0; return NULL;
			}
		}
	}

	// 最后的解密部分，上面一个while已经把明文都解出来了，就剩下尾部的填充了，应该全是0
	// 所以这里有检查是否解密了之后是不是0，如果不是的话那肯定出错了，返回null
	for (padding = 1; padding < 8; padding++)
	{
		if (pos < 8)
		{
			if ((m[offset + preCrypt + pos] ^ prePlain[pos]) != 0) {
				*relen = 0; return NULL;
			}

			pos++;
		}
		if (pos == 8)
		{
			m = (byte*)in;
			preCrypt = crypt;
			if (!decrypt8Bytes(in, offset, len)) {
				*relen = 0; return NULL;
			}
		}
	}

	return out;
}

const byte* QQTea::decipher(const byte* in, int offset)
{
	// 迭代次数，16次
	int loop = 0x10;
	// 得到密文和密钥的各个部分，注意java没有无符号类型，所以为了表示一个无符号的整数
	// 我们用了long，这个long的前32位是全0的，我们通过这种方式模拟无符号整数，后面用到的long也都是一样的
	// 而且为了保证前32位为0，需要和0xFFFFFFFF做一下位与
	long long y = this->getUnsignedInt(in, offset, 4);
	long long z = this->getUnsignedInt(in, offset + 4, 4);
	long long a = this->getUnsignedInt(key, 0, 4);
	long long b = this->getUnsignedInt(key, 4, 4);
	long long c = this->getUnsignedInt(key, 8, 4);
	long long d = this->getUnsignedInt(key, 12, 4);
	// 算法的一些控制变量，sum在这里也有数了，这个sum和迭代次数有关系
	// 因为delta是这么多，所以sum如果是这么多的话，迭代的时候减减减，减16次，最后
	// 得到0。反正这就是为了得到和加密时相反顺序的控制变量，这样才能解密呀～～
	long long sum = 0xE3779B90;
	sum &= 0xFFFFFFFFL;
	long long delta = 0x9E3779B9;
	delta &= 0xFFFFFFFFL;

	// 迭代开始了， @_@
	while (loop-- > 0)
	{
		z -= ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);
		z &= 0xFFFFFFFFL;
		y -= ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
		y &= 0xFFFFFFFFL;
		sum -= delta;
		sum &= 0xFFFFFFFFL;
	}

	baos.reset();
	writeInt((int)y);
	writeInt((int)z);

	return baos.toBytes();
}