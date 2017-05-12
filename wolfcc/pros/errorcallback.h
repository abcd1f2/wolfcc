#ifndef __WOLFCC_ERRORCALLBACK_H_
#define __WOLFCC_ERRORCALLBACK_H_

class ErrorCallback
{
    virtual ~ErrorCallback() {}

    virtual void OnInputQueueFull() {}

    virtual void OnOutputQueueFull() {}
};

#endif
