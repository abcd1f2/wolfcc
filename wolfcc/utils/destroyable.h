#ifndef __WOLFCC_DESTROYABLE_H__
#define __WOLFCC_DESTROYABLE_H__

class Destroyable;

class DestroyCallback
{
public:
    virtual ~DestroyCallback() {};
    virtual void call(Destroyable* p) = 0;
};

class DeleteDestroyCallback : public DestroyCallback
{
public:
    virtual void call(Destroyable* p);

public:
    static DeleteDestroyCallback* Instance();
};

class Destroyable
{
public:
    Destroyable() : pCB_(0) {}
    virtual ~Destroyable() {}

    Destroyable(const Destroyable&) : pCB_(0) {}
    Destroyable& operator=(const Destroyable&) { return *this; }

public:
    void Destroy()
    {
        if (pCB_) {
            pCB_->call(this);
        }
    }

    inline void SetDestroyCallback(DestroyCallback * pCB) {
        pCB_ = pCB;
    }

    inline DestroyCallback * GetDestroyCallback() const {
        return pCB_;
    }

private:
    DestroyCallback * pCB_;
};

#endif 
