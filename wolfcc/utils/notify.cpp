#include <sys/socket.h>
#include <pthread.h>
#include "utils/notify.h"

void SocketNotification::notify(void* sender)
{
    ::send(handle, "0", 1, MSG_DONTWAIT);
}
