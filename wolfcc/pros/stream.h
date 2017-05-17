#ifndef __WOLFCC_STREAM_H__
#define __WOLFCC_STREAM_H__
#include <sys/types.h>
#include <stdint.h>

enum {
	TEXT_STREAM_PROTOCOL = 0x01,
	BINARY_STREAM_PROTOCOL = 0x02,
	TEXT_STREAM_PROTOCOL_2 = 0x03,
	BINARY_STREAM_PROTOCOL_2 = 0x04,
	SERIALNO_WRAPPER_PROTOCOL = 0x11,
	XCODE_PROTOCOL = 0x20,

	FAKE_HTTP_PROTOCOL = 0x21,
	FAKE_HTTP_PROTOCOL_2 = 0x22,
	FAKE_HTTP_PROTOCOL_4 = 0x21,
	FAKE_HTTP_INCLUDE_PROTOCOL_2 = 0x23,
	FAKE_HTTP_INCLUDE_PROTOCOL_4 = 0x24,
};

enum {
	COMPOSITE_FLAG = 0x10000000,
	COMPOSITE_TEXT_STREAM_PROTOCOL = TEXT_STREAM_PROTOCOL | COMPOSITE_FLAG,
	COMPOSITE_BINARY_STREAM_PROTOCOL = BINARY_STREAM_PROTOCOL | COMPOSITE_FLAG,
	COMPOSITE_TEXT_STREAM_PROTOCOL_2 = TEXT_STREAM_PROTOCOL_2 | COMPOSITE_FLAG
};

size_t GetMaxPacketSize(int protocol);
int ParseProtocol(const char* str);

class ReadStream
{
protected:
	const char* ptr;
	const char* cur;
	size_t len;

public:
	ReadStream(const char* p, size_t l) : ptr(p), cur(p), len(l) {}
	virtual ~ReadStream(){}

	virtual size_t GetSize() const = 0;

	virtual bool IsEmpty() const = 0;

	virtual bool Read(char* str, size_t strlen, /* out */ size_t & len) = 0;

	virtual bool Read(/* out */ int64_t & i) = 0;

	virtual bool Read(/* out */ int32_t & i) = 0;

	virtual bool Read(/* out */ int16_t & i) = 0;

	virtual bool Read(/* out */ char & c) = 0;

	bool Read(/* out */ double& f);

	virtual bool ReadNoCopy(const char*& str, size_t& len) = 0;

	virtual bool ReadContent(char* buf, size_t buflen) = 0;

	virtual bool ReadContentNoCopy(const char*& str, size_t len) = 0;

	virtual size_t ReadAll(char * szBuffer, size_t iLen) const = 0;

	virtual bool IsEnd() const = 0;

	virtual const char* GetData() const = 0;

	virtual const char* GetCurrent() const = 0;

	virtual void Reset() = 0;

	virtual void Assign(const char* p, size_t size) = 0;
};

class WriteStream
{
protected:
	char* ptr;
	char* cur;
	size_t len;

public:
	WriteStream(char* p, size_t l) : ptr(p), cur(p), len(l) {}

	virtual ~WriteStream(){}

	virtual bool Write(const char* str, size_t len) = 0;

	virtual bool Write(int64_t i) = 0;

	virtual bool Write(int32_t i) = 0;

	virtual bool Write(int16_t i) = 0;

	virtual bool Write(char  c) = 0;

	bool Write(const double& f);

	virtual bool WriteContent(const char* buf, size_t buflen) = 0;

	virtual void Clear() = 0;

	virtual void Flush() = 0;

	virtual const char* GetData() const = 0;

	virtual size_t GetSize() const = 0;

	virtual char* GetCurrent() const = 0;

	virtual bool IsValid() const = 0;

	virtual void Reset() = 0;

	virtual void Assign(char* p, size_t size) = 0;
};

#endif
