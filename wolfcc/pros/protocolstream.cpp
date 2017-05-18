#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <byteswap.h>
#include <cassert>
#include <algorithm>
#include "utils/logging.h"
#include "pros/protocolstream.h"

TextReadStream::TextReadStream(const char* ptr_, size_t len_, size_t lenFieldLen_)
: ReadStream(ptr_, len_), lenFieldLen(lenFieldLen_)
{
	cur += lenFieldLen;
}

bool TextReadStream::IsEmpty() const
{
	return len <= lenFieldLen;
}

size_t TextReadStream::GetSize() const
{
	return len;
}


bool TextReadStream::ReadNoCopy(const char*& str, size_t& outlen)
{
	size_t fieldlen;
	if (!ReadLengthWithoutOffset(fieldlen)) {
		return false;
	}

	if (cur + fieldlen + lenFieldLen > ptr + len) {
		return false;
	}

	str = cur + lenFieldLen;
	outlen = fieldlen;
	cur += lenFieldLen + fieldlen;
	return true;
}

bool TextReadStream::Read(char* str, size_t strlen, /* out */ size_t & len)
{
	const char* old = cur;
	size_t fieldlen;
	if (!ReadLengthWithoutOffset(fieldlen)) {
		cur = old;
		return false;
	}

	// user buffer is not enough
	if (strlen < fieldlen) {
		return false;
	}

	// 偏移到数据的位置
	cur += lenFieldLen;

	if (!ReadContent(str, fieldlen)) {
		cur = old;
		return false;	// 正确的包绝对不会出现这种情况
	}

	len = fieldlen;
	return true;
}

bool TextReadStream::Read(/* out */ int64_t & i)
{
	char intstr[32];
	size_t len = 0;

	if (!Read(intstr, sizeof(intstr)-1, len)) {
		return false;
	}

	intstr[len] = '\0';
	i = strtoll(intstr, NULL, 10);
	return true;
}

bool TextReadStream::Read(/* out */ int32_t & i)
{
	char intstr[32];
	size_t len = 0;

	if (!Read(intstr, sizeof(intstr)-1, len)) {
		return false;
	}

	intstr[len] = '\0';
	i = atoi(intstr);
	return true;
}

bool TextReadStream::Read(/* out */ int16_t & i)
{
	int i_;
	if (!Read(i_))
		return false;
	i = i_;
	return true;
}

bool TextReadStream::Read(/* out */ char  & c)
{
	size_t len;
	return Read((char*)&c, 1, len);
}

bool TextReadStream::ReadLength(size_t & outlen)
{
	if (!ReadLengthWithoutOffset(outlen)) {
		return false;
	}

	cur += lenFieldLen;
	return true;
}

bool TextReadStream::ReadLengthWithoutOffset(size_t & outlen)
{
	if (cur + lenFieldLen > ptr + len) {
		return false;
	}

	char lenstr[16];
	memcpy(lenstr, cur, lenFieldLen);
	lenstr[lenFieldLen] = '\0';
	outlen = strtol(lenstr, NULL, 16);
	return true;
}

bool TextReadStream::ReadContent(char* str, size_t maxlen)
{
	if (cur + maxlen > ptr + len) {
		return false;
	}

	memcpy(str, cur, maxlen);
	cur += maxlen;
	return true;
}

bool TextReadStream::ReadContentNoCopy(const char*& str, size_t maxlen)
{
	if (cur + maxlen > ptr + len) {
		return false;
	}

	str = cur;
	cur += maxlen;
	return true;
}

bool TextReadStream::IsEnd() const
{
	assert(cur <= ptr + len);
	return cur == ptr + len;
}

const char* TextReadStream::GetData() const
{
	return ptr;
}

size_t TextReadStream::ReadAll(char * szBuffer, size_t iLen) const
{
	size_t iRealLen = std::min(iLen, len);
	memcpy(szBuffer, ptr, iRealLen);
	return iRealLen;
}

TextReadStream TextReadStream::ExtractStream(const char* ptr, size_t len, size_t lenFieldLen)
{
	if (len < lenFieldLen) {
		return TextReadStream(ptr, 0, lenFieldLen);
	}

	char buf[16];
	memcpy(buf, ptr, lenFieldLen);
	buf[lenFieldLen] = '\0';

	size_t packlen = strtol(buf, NULL, 16) + lenFieldLen;
	if (len < packlen) {
		return TextReadStream(ptr, 0, lenFieldLen);
	}

	return TextReadStream(ptr, packlen, lenFieldLen);
}

bool TextReadStream::IsValid() const
{
	if (len < lenFieldLen) {
		return false;
	}

	char buf[16];
	memcpy(buf, ptr, lenFieldLen);
	buf[lenFieldLen] = '\0';

	size_t packlen = strtol(buf, NULL, 16) + lenFieldLen;
	if (len != packlen) {
		return false;
	}

	return true;
}

const char* TextReadStream::GetCurrent() const
{
	return cur;
}

void TextReadStream::Reset()
{
	cur = ptr + lenFieldLen;
}

void TextReadStream::Assign(const char* p, size_t size)
{
	ptr = p;
	len = size;
	cur = ptr + lenFieldLen;
}


TextWriteStream::TextWriteStream(char* ptr_, size_t len_, size_t lenFieldLen_)
: WriteStream(ptr_, len_), lenFieldLen(lenFieldLen_)
{
	cur += lenFieldLen;
}

bool TextWriteStream::Write(const char* str, size_t len)
{
	if (cur + lenFieldLen + len > ptr + this->len)
		return false;

	if (!WriteLength(len))
		return false;

	if (!WriteContent(str, len))
	{
		cur -= lenFieldLen;
		return false;
	}

	return true;
}

bool TextWriteStream::Write(int64_t i)
{
	char intstr[32];
	snprintf(intstr, sizeof(intstr), "%lld", (long long)i);

	return Write(intstr, strlen(intstr));
}

bool TextWriteStream::Write(int32_t i)
{
	char intstr[32];
	snprintf(intstr, sizeof(intstr), "%d", i);

	return Write(intstr, strlen(intstr));
}

bool TextWriteStream::Write(int16_t i)
{
	char intstr[32];
	snprintf(intstr, sizeof(intstr), "%d", i);

	return Write(intstr, strlen(intstr));
}

bool TextWriteStream::Write(char c)
{
	return Write((const char*)&c, 1);
}

bool TextWriteStream::WriteLength(size_t length)
{
	char finalstr[16];
	memset(finalstr, '0', lenFieldLen);
	finalstr[lenFieldLen] = '\0';

	char lengthstr[32] = { 0 };

	snprintf(lengthstr, sizeof(lengthstr), "%zx", length);
	size_t len = strlen(lengthstr);

	if (len > lenFieldLen) {
		return false;
	}

	size_t offset = lenFieldLen - len;
	memcpy(finalstr + offset, lengthstr, len);

	return WriteContent(finalstr, lenFieldLen);
}

bool TextWriteStream::WriteContent(const char* str, size_t length)
{
	memcpy(cur, str, length);
	cur += length;
	return true;
}

size_t TextWriteStream::GetSize() const
{
	return cur - ptr;
}

bool TextWriteStream::IsValid() const
{
	size_t datalen = GetSize() - lenFieldLen;
	size_t maxLen = (1 << (lenFieldLen * 4));
	return datalen <= maxLen;
}

void TextWriteStream::Flush()
{
	size_t datalen = GetSize() - lenFieldLen;

	memset(ptr, '0', lenFieldLen);

	char lengthstr[32] = { 0 };
	snprintf(lengthstr, sizeof(lengthstr), "%zx", datalen);
	size_t len = strlen(lengthstr);

	assert(len <= lenFieldLen);

	memcpy(ptr + lenFieldLen - len, lengthstr, len);
}

void TextWriteStream::Clear()
{
	cur = ptr + lenFieldLen;
}

const char* TextWriteStream::GetData() const
{
	return ptr;
}

char* TextWriteStream::GetCurrent() const
{
	return cur;
}

void TextWriteStream::Reset()
{
	cur = ptr + lenFieldLen;
}

void TextWriteStream::Assign(char* p, size_t size)
{
	ptr = p;
	len = size;
	cur = ptr + lenFieldLen;
}

// Binary stream

BinaryReadStream::BinaryReadStream(const char* ptr_, size_t len_, size_t lenFieldLen_, HEADER_FLAG headerFlag_)
: ReadStream(ptr_, len_), lenFieldLen(lenFieldLen_), headerFlag(headerFlag_)
{
	cur += lenFieldLen;
}

bool BinaryReadStream::IsEmpty() const
{
	return len <= lenFieldLen;
}

size_t BinaryReadStream::GetSize() const
{
	return len;
}

bool BinaryReadStream::ReadNoCopy(const char*& str, size_t& len)
{
	if (!ReadLength(len)) {
		return false;
	}

	str = cur;
	cur += len;
	return true;
}

bool BinaryReadStream::Read(char* str, size_t strlen, /* out */ size_t& outlen)
{
	size_t fieldlen;
	if (!ReadLengthWithoutOffset(fieldlen)) {
		return false;
	}

	// user buffer is not enough
	if (strlen < fieldlen) {
		return false;
	}

	// 偏移到数据的位置
	cur += lenFieldLen;

	if (!ReadContent(str, fieldlen)) {
		cur -= lenFieldLen;
		return false;	// 正确的包绝对不会出现这种情况
	}

	outlen = fieldlen;
	return true;
}

bool BinaryReadStream::Read(/* out */ int64_t & i)
{
	const int VALUE_SIZE = sizeof(int64_t);

	if (cur + VALUE_SIZE > ptr + len) {
		return false;
	}

	memcpy(&i, cur, VALUE_SIZE);
#if __BYTE_ORDER == __LITTLE_ENDIAN
	i = bswap_64(i);
#endif

	cur += VALUE_SIZE;

	return true;
}

bool BinaryReadStream::Read(/* out */ int32_t & i)
{
	const int VALUE_SIZE = sizeof(int32_t);

	if (cur + VALUE_SIZE > ptr + len) {
		return false;
	}

	memcpy(&i, cur, VALUE_SIZE);
	i = ntohl(i);

	cur += VALUE_SIZE;

	return true;
}

bool BinaryReadStream::Read(/* out */ int16_t & i)
{
	const int VALUE_SIZE = sizeof(int16_t);

	if (cur + VALUE_SIZE > ptr + len) {
		return false;
	}

	memcpy(&i, cur, VALUE_SIZE);
	i = ntohs(i);

	cur += VALUE_SIZE;

	return true;
}

bool BinaryReadStream::Read(/* out */ char & c)
{
	const int VALUE_SIZE = sizeof(char);

	if (cur + VALUE_SIZE > ptr + len) {
		return false;
	}

	memcpy(&c, cur, VALUE_SIZE);
	cur += VALUE_SIZE;

	return true;
}

bool BinaryReadStream::ReadLength(size_t & outlen)
{
	if (!ReadLengthWithoutOffset(outlen)) {
		return false;
	}

	cur += lenFieldLen;
	return true;
}

bool BinaryReadStream::ReadLengthWithoutOffset(size_t & outlen)
{
	if (cur + lenFieldLen > ptr + len) {
		return false;
	}

	if (sizeof(short) == lenFieldLen) {
		unsigned short tmp;
		memcpy(&tmp, cur, sizeof(tmp));
		outlen = ntohs(tmp);
	}
	else {
		unsigned int tmp;
		memcpy(&tmp, cur, sizeof(tmp));
		outlen = ntohl(tmp);
	}
	return true;
}

bool BinaryReadStream::ReadContent(char* str, size_t maxlen)
{
	if (cur + maxlen > ptr + len) {
		return false;
	}

	memcpy(str, cur, maxlen);
	cur += maxlen;
	return true;
}

bool BinaryReadStream::ReadContentNoCopy(const char*& str, size_t maxlen)
{
	if (cur + maxlen > ptr + len) {
		return false;
	}

	str = cur;
	cur += maxlen;
	return true;
}

bool BinaryReadStream::IsEnd() const
{
	assert(cur <= ptr + len);
	return cur == ptr + len;
}

const char* BinaryReadStream::GetData() const
{
	return ptr;
}

size_t BinaryReadStream::ReadAll(char * szBuffer, size_t iLen) const
{
	size_t iRealLen = std::min(iLen, len);
	memcpy(szBuffer, ptr, iRealLen);
	return iRealLen;
}

BinaryReadStream BinaryReadStream::ExtractStream(const char* ptr, size_t len, size_t lenFieldLen, HEADER_FLAG headerFlag)
{
	if (len < lenFieldLen) {
		return BinaryReadStream(ptr, 0, lenFieldLen, headerFlag);
	}

	size_t packlen;
	if (sizeof(short) == lenFieldLen) {
		unsigned short slen = *(unsigned short*)ptr;
		packlen = ntohs(slen);
	}
	else {
		unsigned int ilen = *(unsigned int*)ptr;
		packlen = ntohl(ilen);
	}

	if (headerFlag == LENGTH_EXCLUDE_SELF) {
		packlen += lenFieldLen;
	}

	if (packlen <= lenFieldLen) {
		return BinaryReadStream(ptr, 0, lenFieldLen, headerFlag);
	}

	if (len < packlen) {
		return BinaryReadStream(ptr, 0, lenFieldLen, headerFlag);
	}

	return BinaryReadStream(ptr, packlen, lenFieldLen, headerFlag);
}

bool BinaryReadStream::IsValid() const
{
	if (len < lenFieldLen) {
		return false;
	}

	size_t packlen = 0;
	if (sizeof(short) == lenFieldLen) {
		unsigned short s = *(unsigned short*)ptr;
		packlen = ntohs(s);
	}
	else {
		unsigned int i = *(unsigned int*)ptr;
		packlen = ntohl(i);
	}

	if (headerFlag == LENGTH_EXCLUDE_SELF) {
		packlen += lenFieldLen;
	}

	if (packlen != len) {
		return false;
	}

	return true;
}

const char* BinaryReadStream::GetCurrent() const
{
	return cur;
}

void BinaryReadStream::Reset()
{
	cur = ptr + lenFieldLen;
}

void BinaryReadStream::Assign(const char* p, size_t size)
{
	ptr = p;
	len = size;
	cur = ptr + lenFieldLen;
}



BinaryWriteStream::BinaryWriteStream(char* ptr_, size_t len_, size_t lenFieldLen_, HEADER_FLAG headerFlag_)
: WriteStream(ptr_, len_), lenFieldLen(lenFieldLen_), headerFlag(headerFlag_)
{
	cur += lenFieldLen;
}

bool BinaryWriteStream::Write(const char* str, size_t length)
{
	if (cur + length + lenFieldLen > ptr + len) {
		return false;
	}

	if (!WriteLength(length)) {
		cur -= lenFieldLen;
		return false;
	}

	memcpy(cur, str, length);
	cur += length;

	return true;
}

bool BinaryWriteStream::Write(int64_t i)
{
	if (cur + sizeof(i) > ptr + len) {
		return false;
	}

#if  __BYTE_ORDER == __LITTLE_ENDIAN
	i = bswap_64(i);
#endif
	memcpy(cur, &i, sizeof(i));
	cur += sizeof(i);

	return true;
}

bool BinaryWriteStream::Write(int32_t i)
{
	if (cur + sizeof(i) > ptr + len) {
		return false;
	}

	int iTmp = htonl(i);
	memcpy(cur, &iTmp, sizeof(iTmp));
	cur += sizeof(iTmp);

	return true;
}

bool BinaryWriteStream::Write(int16_t i)
{
	if (cur + sizeof(i) > ptr + len) {
		return false;
	}

	short iTmp = htons(i);
	memcpy(cur, &iTmp, sizeof(iTmp));
	cur += sizeof(iTmp);

	return true;
}

bool BinaryWriteStream::Write(char c)
{
	if (cur + sizeof(c) > ptr + len) {
		return false;
	}

	*cur = c;
	++cur;
	return true;
}

bool BinaryWriteStream::WriteLength(size_t length)
{
	if (sizeof(short) == lenFieldLen) {
		unsigned short ulen = length;

		ulen = htons(ulen);
		memcpy(cur, &ulen, sizeof(ulen));
		cur += sizeof(ulen);
	}
	else {
		unsigned int ulen = length;

		ulen = htonl(ulen);
		memcpy(cur, &ulen, sizeof(ulen));
		cur += sizeof(ulen);
	}
	return true;
}

bool BinaryWriteStream::WriteContent(const char* str, size_t length)
{
	if (cur + length > ptr + len) {
		return false;
	}

	memcpy(cur, str, length);
	cur += length;

	return true;
}

size_t BinaryWriteStream::GetSize() const
{
	return cur - ptr;
}

bool BinaryWriteStream::IsValid() const
{
	size_t datalen = GetSize();
	return datalen > lenFieldLen && datalen <= BINARY_PACKAGE_MAXLEN;
}

void BinaryWriteStream::Flush()
{
	size_t packlen = GetSize();
	if (headerFlag == LENGTH_EXCLUDE_SELF) {
		packlen -= lenFieldLen;
	}

	if (lenFieldLen == sizeof(short)) {
		unsigned short datalen = packlen;
		datalen = htons(datalen);
		unsigned short* ps = reinterpret_cast<unsigned short*>(ptr);
		*ps = datalen;
	}
	else {
		unsigned int datalen = packlen;
		datalen = htonl(datalen);
		unsigned int* pi = reinterpret_cast<unsigned int*>(ptr);
		*pi = datalen;
	}
}

void BinaryWriteStream::Clear()
{
	cur = ptr + lenFieldLen;
}

const char* BinaryWriteStream::GetData() const
{
	return ptr;
}

char* BinaryWriteStream::GetCurrent() const
{
	return cur;
}

void BinaryWriteStream::Reset()
{
	cur = ptr + lenFieldLen;
}

void BinaryWriteStream::Assign(char* p, size_t size)
{
	ptr = p;
	len = size;
	cur = ptr + lenFieldLen;
}
