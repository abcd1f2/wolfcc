#ifndef __WOLFCC_ERRORCALLBACK_H_
#define __WOLFCC_ERRORCALLBACK_H_

class ErrorCallback
{
public:
    virtual ~ErrorCallback() {}

public:
    virtual void OnInputQueueFull() {}

    virtual void OnOutputQueueFull() {}
};

class StreamProtocolCallBack : public ErrorCallback
{
public:
	virtual ~StreamProtocolCallBack() {}

public:
	virtual void OnInputQueueFull();

	virtual void OnOutputQueueFull();
};

#endif
