#ifndef __WOLFCC_PROCPACKFACTORY_H__
#define __WOLFCC_PROCPACKFACTORY_H__

class Procpack;

class PackProcFactory
{
public:
    virtual ~PackProcFactory() {}

public:
    void SetProtocolType(int ptype){
        protocol_type_ = ptype;
    }

public:
    virtual Procpack * Create(int iType) = 0;
    virtual void Destroy(const Procpack *) = 0;

protected:
    int protocol_type_;
};

class AppPackProcFactory : public PackProcFactory
{
public:
    AppPackProcFactory();

public:
    virtual Procpack * Create(int iType);
    virtual void Destroy(const Procpack *);
};

#endif
