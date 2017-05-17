#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <string>
#include "nets/connecteventhandler.h"
#include "nets/reactor.h"
#include "nets/socket.h"
#include "utils/logging.h"
#include "utils/queue.h"

ConnectEventHandler::ConnectEventHandler()
    : timeout(0), extractor(0)
{
    
}

ConnectEventHandler::~ConnectEventHandler()
{
    
}

int time_interval(const timeval& t1, const timeval& t2);

int ConnectEventHandler::Open()
{
    sendbuffer.Clear();
    recvbuffer.Clear();
    closeaftersent = false;

    gettimeofday(&opentime, NULL);
    received = false;

    timeval tv = { timeout, 0 };
    return GetReactor()->RegisterHandler(ReadMask, this, &tv);
}

int ConnectEventHandler::Close()
{
    return GetReactor()->RemoveHandler(ReadMask | WriteMask, this);
}

void ConnectEventHandler::HandleInput()
{
    const int RECV_SIZE = 16384;

    size_t pos = recvbuffer.GetSize();
    recvbuffer.Resize(pos + RECV_SIZE);

    gettimeofday(&inputtime, NULL);
    int ret = ::recv(GetHandle(), recvbuffer.GetBegin() + pos, RECV_SIZE, 0);
    gettimeofday(&recvtime, NULL);

    if (ret == 0)
    {
        log(LOG_DEBUG, "Receive failed: client disconnectd");
        Close();
    }
    else if (ret == -1)
    {
        if (errno == EINTR)
            return;
        else
        {
            log(LOG_WARN, "Receive failed: %s", strerror(errno));
            Close();
        }
    }
    else
    {
        received = true;
        recvbuffer.IncEnd(ret);

        int eat = extractor->Extract(GetGlobalId(), GetProcessorId(), recvbuffer.GetBegin(), recvbuffer.GetSize(), peerAddr);
        if (eat < 0) {
            log(LOG_WARN, "Extract package fail");
            Close();
        }
        else
        {
            if (eat > 0)
            {
                recvbuffer.Copy(recvbuffer.GetBegin(), recvbuffer.GetBegin() + eat, recvbuffer.GetSize() - eat);
                recvbuffer.DecEnd(eat);
            }
            timeval tv = { timeout, 0 };
            if (GetReactor()->RegisterHandler(ReadMask, this, &tv) == -1)
            {
                log(LOG_ERR, "register read event fail");
                Close();
            }
        }
    }
}

void ConnectEventHandler::HandleOutput()
{
    if (sendbuffer.GetSize() <= 0)
        return;

    int ret = ::send(GetHandle(), sendbuffer.GetBegin(), sendbuffer.GetSize(), MSG_DONTWAIT);
    if (ret == 0)
    {
        log(LOG_DEBUG, "Send failed: client disconnectd");
        Close();
    }
    else if (ret == -1)
    {
        if (errno == EINTR)
        {
            return;
        }
        else if (errno == EAGAIN)
        {
            timeval tv = { timeout, 0 };
            if (GetReactor()->RegisterHandler(WriteMask, this, &tv) == -1)
            {
                log(LOG_ERR, "register write event fail");
                Close();
            }
        }
        else
        {
            log(LOG_WARN, "Receive failed: %s", strerror(errno));
            Close();
        }
    }
    else
    {
        if ((size_t)ret == sendbuffer.GetSize())
        {
            sendbuffer.Clear();
        }
        else
        {
            sendbuffer.Copy(sendbuffer.GetBegin(), sendbuffer.GetBegin() + ret, sendbuffer.GetSize() - ret);
            sendbuffer.Resize(sendbuffer.GetSize() - ret);
        }

        if (sendbuffer.IsEmpty())
        {
            if (closeaftersent)
            {
                log(LOG_DEBUG, "CloseAfterSent is set, remove all events");
                Close();
            }
            else
                GetReactor()->RemoveHandler(WriteMask, this);
        }
        else
        {
            timeval tv = { timeout, 0 };
            if (GetReactor()->RegisterHandler(WriteMask, this, &tv) == -1)
            {
                log(LOG_ERR, "register write event fail");
                Close();
            }
        }
    }
}

void ConnectEventHandler::HandleTimeout(void *)
{
    log(LOG_DEBUG, "Connection timeout, remove all events");
    Close();
}

void ConnectEventHandler::HandleClose()
{
    log(LOG_DEBUG, "Connection closed");

    ::close(GetHandle());
    EventHandler::HandleClose();
}

void ConnectEventHandler::Send(const char* data, size_t len)
{
    sendbuffer.Append(data, len);
    HandleOutput();
}
