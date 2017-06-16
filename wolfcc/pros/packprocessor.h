#ifndef __WOLFCC_PACKPROCESSOR_H__
#define __WOLFCC_PACKPROCESSOR_H__

struct Request;
struct Response;

enum 
{
	PROCESSOR_PROTOCOL_MESSAGEPACK = 1,
	PROCESSOR_PROTOCOL_PROTOBUF
};

class Processor
{
public:
    virtual ~Processor(){}

public:
    virtual int Process(Request* request, Response* responce) = 0;
};

class PackProcessor : public Processor
{
public:
    virtual ~PackProcessor(){}

public:
    virtual int Process(Request* request, Response* responce){}

    bool AppendDataToCache(char *key, char *bdbname, const char *value, int valuelen, int realcount = 10, int limitcount = 20){}
    int GetCutStart(int cutcount, const char *output, int outlen){}
private:
};

class MessagepackProcessor : public Processor
{
public:
	MessagepackProcessor(){}
	virtual ~MessagepackProcessor(){}

public:
	virtual int Process(Request* request, Response* responce){}
};

class ProtoBufProcessor : public Processor
{
public:
	ProtoBufProcessor(){}
	virtual ~ProtoBufProcessor(){}

public:
	virtual int Process(Request* request, Response* responce){}
};

#endif
