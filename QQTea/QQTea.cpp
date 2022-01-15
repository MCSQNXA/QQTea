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

	// ����ͷ������ֽ���
	pos = (len + 0x0A) % 8;

	if (pos != 0) {
		pos = 8 - pos;
	}

	// ������������ĳ���
	out = new byte[*relen = (long long)len + pos + 10]; this->buff.push_back(out);
	// ����Ĳ�����pos�浽��plain�ĵ�һ���ֽ�����
	// 0xF8������λ�ǿյģ���������pos����Ϊpos��0��7��ֵ����ʾ�ı���ʼ���ֽ�λ��
	plain[0] = (byte)((random() & 0xF8) | pos);

	// ��������������������plain[1]��plain[pos]֮�������
	for (int i = 1; i <= pos; i++) {
		plain[i] = (byte)(random() & 0xFF);
	}

	pos++;
	// �������prePlain����һ��8�ֽڿ鵱Ȼû��prePlain������������һ��ȫ0�ĸ���һ��8�ֽڿ�
	for (int i = 0; i < 8; i++) {
		prePlain[i] = 0x0;
	}

	// �������2���ֽڵ������������������������8�ֽھͼ���֮
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

	// ͷ��������ˣ����￪ʼ�������������ˣ�Ҳ������8�ֽھͼ��ܣ�һֱ�����Ķ���
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

	// �������0���Ա�֤��8�ֽڵı���
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
	// ����������16��
	int loop = 0x10;
	// �õ����ĺ���Կ�ĸ������֣�ע��javaû���޷������ͣ�����Ϊ�˱�ʾһ���޷��ŵ�����
	// ��������long�����long��ǰ32λ��ȫ0�ģ�����ͨ�����ַ�ʽģ���޷��������������õ���longҲ����һ����
	// ����Ϊ�˱�֤ǰ32λΪ0����Ҫ��0xFFFFFFFF��һ��λ��            
	long long y = getUnsignedInt(in, 0, 4);
	long long z = getUnsignedInt(in, 4, 4);
	long long a = getUnsignedInt(key, 0, 4);
	long long b = getUnsignedInt(key, 4, 4);
	long long c = getUnsignedInt(key, 8, 4);
	long long d = getUnsignedInt(key, 12, 4);
	// �����㷨��һЩ���Ʊ�����Ϊʲôdelta��0x9E3779B9�أ�
	// �������TEA�㷨��delta��ʵ���Ǿ���(sqr(5) - 1) * 2^31 (����5����1���ٳ�2��31�η�)
	long long sum = 0;
	long long delta = 0x9E3779B9;
	delta &= 0xFFFFFFFFL;

	// ��ʼ�����ˣ����߰���ģ���Ҳ��������������DES֮��Ĳ�࣬��������������ȥ
	while (loop-- > 0)
	{
		sum += delta;
		sum &= 0xFFFFFFFFL;
		y += ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
		y &= 0xFFFFFFFFL;
		z += ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);
		z &= 0xFFFFFFFFL;
	}

	// �������������ģ���Ϊ���õ�long��������Ҫǿ��ת��һ�±��int
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
    // �ⲿ�������������˵�� plain ^ preCrypt��ע�������ж����ǲ��ǵ�һ��8�ֽڿ飬����ǵĻ����Ǹ�prePlain�͵���preCrypt��
    for (pos = 0; pos < 8; pos++)
    {
        if (header) {
            plain[pos] ^= prePlain[pos];
        }
        else {
            plain[pos] ^= out[preCrypt + pos];
        }
    }

    // ������������˵�� f(plain ^ preCrypt)
    const byte* crypted = encipher(plain);
    // ���ûʲô�����ǿ���һ�£�java����c��������ֻ����ô�ɣ�c�Ͳ�����һ����
    arraycopy(crypted, 0, out, crypt, 8);

    // �������� f(plain ^ preCrypt) ^ prePlain��ok�����濽��һ�¾�����
    for (pos = 0; pos < 8; pos++) {
        out[crypt + pos] ^= prePlain[pos];
    }

    arraycopy(plain, 0, prePlain, 0, 8);

    // ����˼��ܣ������ǵ���crypt��preCrypt�ȵȶ�����ʱ����
    preCrypt = crypt;
    crypt += 8;
    pos = 0;
    header = false;
}

bool QQTea::decrypt8Bytes(const byte* in, int offset, int len)
{
	// �����һ�������жϺ��滹��û�����ݣ�û�оͷ��أ�����У���ִ�� crypt ^ prePlain
	for (pos = 0; pos < 8; pos++)
	{
		if (contextStart + pos >= len) {
			return true;
		}

		prePlain[pos] ^= in[offset + crypt + pos];
	}

	// �ã�����ִ�е��� d(crypt ^ prePlain)
	prePlain = (byte*)decipher(prePlain, 0);
	if (prePlain == NULL) {
		return false;
	}

	// ������ɣ����һ������û���� 
	// �������һ���ŵ�decrypt����ȥ���ˣ���Ϊ���ܵĲ����е㲻̫һ��
	// ������Щ������ֵ��
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

	// ��ΪQQ��Ϣ����֮��������16�ֽڣ����ҿ϶���8�ı������������������
	if ((len % 8 != 0) || (len < 16))
	{
		*relen = 0; return NULL;
	}

	// �õ���Ϣ��ͷ�����ؼ��ǵõ��������Ŀ�ʼ��λ�ã������Ϣ���ڵ�һ���ֽ����棬�������ý��ܵõ��ĵ�һ���ֽ���7����
	prePlain = (byte*)decipher(in, offset);
	pos = prePlain[0] & 0x7;
	// �õ��������ĵĳ���
	count = len - pos - 10;
	// ������ĳ���С��0���ǿ϶��ǳ����ˣ����紫�����֮��ģ�����
	if (count < 0) {
		*relen = 0; return NULL;
	}

	// �������ʱ��preCrypt���ͼ���ʱ��һ��8�ֽڿ�û��prePlainһ��������ʱ
	// ��һ��8�ֽڿ�Ҳû��preCrypt���������ｨһ��ȫ0��
	for (int i = offset; i < mlen; i++) {
		m[i] = 0;
	}

	// ͨ��������Ĵ��룬����Ӧ����û�������ˣ����Ƿ������������
	out = new byte[*relen = count]; this->buff.push_back(out);
	// ����preCrypt��λ�õ���0��ע��Ŀǰ��preCryptλ����ָ��m�ģ���Ϊjavaû��ָ�룬���������ں���Ҫ���Ƶ�ǰ����buf������
	preCrypt = 0;
	// ��ǰ������λ�ã�Ϊʲô��8����0�أ�ע��ǰ�������Ѿ�������ͷ����Ϣ�ˣ����ڵ�Ȼ��8��
	crypt = 8;
	// ��Ȼ���Ҳ��8
	contextStart = 8;
	// ��1���ͼ����㷨�Ƕ�Ӧ��
	pos++;

	// ��ʼ����ͷ����������������������8�ֽڣ��������һ��
	// ��Ϊ�ǽ�����һ�飬����������һ����� m = in����һ�鵱Ȼ��preCrypt�ˣ����ǲ�����m��
	// �����������8����˵����ʲô��˵����ͷ8���ֽڵ������ǰ�����������Ϣ�ģ���Ȼ����Ҫ��m������Ū����
	// ���ԣ�����Ȼ������8�Ļ���˵����ͷ8���ֽڵ����ĳ���һ��������Ϣ����֮�⣬�����������õ����
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

	// �����ǽ��ܵ���Ҫ�׶Σ����ʱ��ͷ������䶼�Ѿ������ˣ���ʼ����
	// ע���������һ��whileû����8�������һ��if�����õľ���ԭʼ��m���������m����in��
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

	// ���Ľ��ܲ��֣�����һ��while�Ѿ������Ķ�������ˣ���ʣ��β��������ˣ�Ӧ��ȫ��0
	// ���������м���Ƿ������֮���ǲ���0��������ǵĻ��ǿ϶������ˣ�����null
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
	// ����������16��
	int loop = 0x10;
	// �õ����ĺ���Կ�ĸ������֣�ע��javaû���޷������ͣ�����Ϊ�˱�ʾһ���޷��ŵ�����
	// ��������long�����long��ǰ32λ��ȫ0�ģ�����ͨ�����ַ�ʽģ���޷��������������õ���longҲ����һ����
	// ����Ϊ�˱�֤ǰ32λΪ0����Ҫ��0xFFFFFFFF��һ��λ��
	long long y = this->getUnsignedInt(in, offset, 4);
	long long z = this->getUnsignedInt(in, offset + 4, 4);
	long long a = this->getUnsignedInt(key, 0, 4);
	long long b = this->getUnsignedInt(key, 4, 4);
	long long c = this->getUnsignedInt(key, 8, 4);
	long long d = this->getUnsignedInt(key, 12, 4);
	// �㷨��һЩ���Ʊ�����sum������Ҳ�����ˣ����sum�͵��������й�ϵ
	// ��Ϊdelta����ô�࣬����sum�������ô��Ļ���������ʱ�����������16�Σ����
	// �õ�0�����������Ϊ�˵õ��ͼ���ʱ�෴˳��Ŀ��Ʊ������������ܽ���ѽ����
	long long sum = 0xE3779B90;
	sum &= 0xFFFFFFFFL;
	long long delta = 0x9E3779B9;
	delta &= 0xFFFFFFFFL;

	// ������ʼ�ˣ� @_@
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