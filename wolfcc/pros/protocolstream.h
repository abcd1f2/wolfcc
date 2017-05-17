#ifndef __WOLFCC_PROTOCOLSTREAM_H__
#define __WOLFCC_PROTOCOLSTREAM_H__
#include <stdlib.h>
#include <sys/types.h>
#include "pros/stream.h"

enum {
	TEXT_PACKLEN_LEN = 4,
	TEXT_PACKAGE_MAXLEN = 0xffff,
	BINARY_PACKLEN_LEN = 2,
	BINARY_PACKAGE_MAXLEN = 0xffff,

	TEXT_PACKLEN_LEN_2 = 6,
	TEXT_PACKAGE_MAXLEN_2 = 0xffffff,
	BINARY_PACKLEN_LEN_2 = 4,
	BINARY_PACKAGE_MAXLEN_2 = 0xffffff,
};

class TextReadStream : public ReadStream
{
private:
	size_t lenFieldLen;

public:
	TextReadStream(const char* ptr, size_t len, size_t lenFieldLen = TEXT_PACKLEN_LEN);

	virtual size_t GetSize() const;

	virtual bool IsEmpty() const;

	virtual bool Read(char* str, size_t strlen, /* out */ size_t & len);

	virtual bool Read(/* out */ int64_t & i);

	virtual bool Read(/* out */ int32_t & i);

	virtual bool Read(/* out */ int16_t & i);

	virtual bool Read(/* out */ char  & c);

	virtual const char* GetData() const;

	virtual size_t ReadAll(char * szBuffer, size_t iLen) const;

	virtual bool IsEnd() const;

	virtual bool IsValid() const;

	virtual const char* GetCurrent() const;

	virtual bool ReadNoCopy(const char*& str, size_t& len);

	virtual bool ReadContent(char* buf, size_t buflen);

	virtual bool ReadContentNoCopy(const char*& buf, size_t buflen);

	virtual void Reset();

	virtual void Assign(const char* p, size_t size);

public:

	bool ReadLength(size_t & len);

	bool ReadLengthWithoutOffset(size_t & len);

public:
	static TextReadStream ExtractStream(const char* ptr, size_t len, size_t lenFieldLen = TEXT_PACKLEN_LEN);
};


class TextWriteStream : public WriteStream
{
private:
	size_t lenFieldLen;

	TextWriteStream(const TextWriteStream&);
	TextWriteStream& operator=(const TextWriteStream&);

public:
	TextWriteStream(char* ptr, size_t len, size_t lenFieldLen = TEXT_PACKLEN_LEN);

	virtual bool Write(const char* str, size_t len);

	virtual bool Write(int64_t i);

	virtual bool Write(int32_t i);

	virtual bool Write(int16_t i);

	virtual bool Write(char  c);

	virtual size_t GetSize() const;

	virtual bool IsValid() const;

	virtual void Flush();

	virtual void Clear();

	virtual const char* GetData() const;

	virtual char* GetCurrent() const;

	virtual void Reset();

	virtual void Assign(char* p, size_t size);

public:
	bool WriteLength(size_t len);

	bool WriteContent(const char* buf, size_t buflen);
};

enum HEADER_FLAG{
	LENGTH_INCLUDE_SELF,
	LENGTH_EXCLUDE_SELF
};

class BinaryReadStream : public ReadStream
{
private:
	size_t lenFieldLen;
	HEADER_FLAG headerFlag;
public:
	BinaryReadStream(const char* ptr, size_t len, size_t lenFieldLen = BINARY_PACKLEN_LEN, HEADER_FLAG headerFlag = LENGTH_INCLUDE_SELF);

	virtual size_t GetSize() const;

	virtual bool IsEmpty() const;

	virtual bool Read(char* str, size_t strlen, /* out */ size_t& len);

	virtual bool Read(/* out */ int64_t & i);

	virtual bool Read(/* out */ int32_t & i);

	virtual bool Read(/* out */ int16_t & i);

	virtual bool Read(/* out */ char  & c);

	virtual const char* GetData() const;

	virtual size_t ReadAll(char * szBuffer, size_t iLen) const;

	virtual bool IsEnd() const;

	virtual bool IsValid() const;

	virtual const char* GetCurrent() const;

	virtual bool ReadNoCopy(const char*& str, size_t& len);

	virtual bool ReadContent(char* buf, size_t buflen);

	virtual bool ReadContentNoCopy(const char*& buf, size_t buflen);

	virtual void Reset();

	virtual void Assign(const char* p, size_t size);

public:

	bool ReadLength(size_t & len);

	bool ReadLengthWithoutOffset(size_t & outlen);

public:
	static BinaryReadStream ExtractStream(const char* ptr, size_t len, size_t lenFieldLen = BINARY_PACKLEN_LEN, HEADER_FLAG headerFlag = LENGTH_INCLUDE_SELF);
};

class BinaryWriteStream : public WriteStream
{
private:
	size_t lenFieldLen;
	HEADER_FLAG headerFlag;

	BinaryWriteStream(const BinaryWriteStream&);
	BinaryWriteStream& operator=(const BinaryWriteStream&);

public:
	BinaryWriteStream(char* ptr, size_t len, size_t lenFieldLen = BINARY_PACKLEN_LEN, HEADER_FLAG headerFlag = LENGTH_INCLUDE_SELF);

	virtual bool Write(const char* str, size_t len);

	virtual bool Write(int64_t i);

	virtual bool Write(int32_t i);

	virtual bool Write(int16_t i);

	virtual bool Write(char  c);

	virtual bool WriteContent(const char* str, size_t len);

	virtual size_t GetSize() const;

	virtual bool IsValid() const;

	virtual void Flush();

	virtual void Clear();

	virtual const char* GetData() const;

	virtual char* GetCurrent() const;

	virtual void Reset();

	virtual void Assign(char* p, size_t size);

public:
	bool WriteLength(size_t len);
};

#endif
