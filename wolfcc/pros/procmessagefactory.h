#ifndef __WOLFCC_PROCMESSAGEFACTORY_H__
#define __WOLFCC_PROCMESSAGEFACTORY_H__

class Message;

class MessageProcFactory
{
public:
    virtual ~MessageProcFactory() {}

public:
    void SetProtocolType(int ptype){
        protocol_type_ = ptype;
    }

public:
    virtual Message * Create(int iType) = 0;
    virtual void Destroy(const Message *) = 0;

protected:
    int protocol_type_;
};

class AppMessageProcFactory : public MessageProcFactory
{
public:
    AppMessageProcFactory();

public:
    virtual Message * Create(int iType);
    virtual void Destroy(const Message *);
};

#endif
