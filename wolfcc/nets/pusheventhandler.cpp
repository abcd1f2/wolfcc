#include <errno.h>
#include <sys/socket.h>
#include <string>
#include "nets/pusheventhandler.h"
#include "nets/reactor.h"
#include "utils/logging.h"
#include "utils/handlerregistry.h"
#include "pros/package.h"

int time_interval(const timeval& t1, const timeval& t2)
{
    return t1.tv_usec - t2.tv_usec + (t1.tv_sec - t2.tv_sec) * 1000000;
}

void PushEventHandler::HandleEvent()
{
    Package* p;
    while (queue_p_->Pop(p) == 0)
    {
        gettimeofday(&p->gettime2, NULL);
        AcfEventHandler* handler = dynamic_cast<AcfEventHandler*>(registry_p_->GetHandler(p->GetHandlerId()));
        if (!handler) {
            log(LOG_WARN, "Handler not found, maybe it disconnected");
        }
        else {
            if (p->GetDataLen() > 0) {
                if (p->GetRetCode() != 0)
                    handler->CloseAfterSent();

                handler->Send(p->GetData(), p->GetDataLen());
            }
            else {
                if (p->GetRetCode() != 0)
                    handler->Close();
            }
        }

        p->Destroy();
    }
}
