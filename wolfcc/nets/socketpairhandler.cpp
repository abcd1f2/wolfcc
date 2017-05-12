#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "nets/socketpairhandler.h"
#include "nets/handle.h"
#include "nets/reactor.h"
#include "utils/logging.h"

int SocketPairHandler::Open()
{
	int fd[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) != 0) {
        log(LOG_ERR, "socket pair failed: %s", strerror(errno));
		return -1;
	}

	SetHandle(fd[0]);
	anotherhandle = fd[1];

	int ret = GetReactor()->RegisterHandler(ReadMask | PersistMask, this);
    if (ret != 0) {
        CloseHandles();
    }

	return ret;
}

void SocketPairHandler::HandleInput()
{
    log(LOG_ERR, "SocketPairHandler::HandleInput()");

	char buffer[64];
	int ret = ::recv(GetHandle(), buffer, sizeof(buffer), 0);
	if (ret == -1 && errno == EINTR) {
        log(LOG_ERR, "EINTR when recv");
		return;
    }

	if (ret <= 0) {
        log(LOG_ERR, "read notify socket fail: %s, remove all events", strerror(errno));
		GetReactor()->RemoveHandler(ReadMask, this);
		return;
	}

	HandleEvent();
}

void SocketPairHandler::HandleClose()
{
    log(LOG_ERR, "SocketPairHandler::HandleClose()");

	CloseHandles();

	EventHandler::HandleClose();
}

void SocketPairHandler::CloseHandles()
{
	::close(GetHandle());
	::close(GetAnotherHandle());
	SetHandle(Handle());
	anotherhandle = Handle();
}
