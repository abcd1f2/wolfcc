#ifndef __WOLFCC_PACKPROCESSOR_H__
#define __WOLFCC_PACKPROCESSOR_H__

class PackProcessor
{
    virtual ~PackProcessor(){}

    virtual int Process(Request* request, Responce* responce) = 0;
};

class Processor : public PackProcessor
{
public:
    //virtual int Process(ReadStream* inputstream, WriteStream* outputstream);
    virtual int Process(Request* request, Responce* responce);

    bool AppendDataToCache(char *key, char *bdbname, const char *value, int valuelen, int realcount = 10, int limitcount = 20);
    int GetCutStart(int cutcount, const char *output, int outlen);
private:
};

#endif
