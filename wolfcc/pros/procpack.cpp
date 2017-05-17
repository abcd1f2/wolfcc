#include "pros/procpack.h"
#include "utils/logging.h"
#include "pros/package.h"
#include "utils/errorcallback.h"

int TextStreamProcpack::Extract(size_t sockid, size_t processorid, const char* ptr, size_t len, const SockAddr& addr)
{
    size_t used = 0;

    while (true) {
        if (len <= TEXT_PACKLEN_LEN)
            return used;

        char buf[TEXT_PACKLEN_LEN + 1];
        memcpy(buf, ptr, TEXT_PACKLEN_LEN);
        buf[TEXT_PACKLEN_LEN] = '\0';

        unsigned short packlen = strtol(buf, NULL, 16);
        if (len < (size_t)(packlen + TEXT_PACKLEN_LEN))
            return used;

        int eat = TEXT_PACKLEN_LEN + packlen;
        log(LOG_DEBUG, "Eat %d bytes", eat);
        if (OnPackage(sockid, processorid, ptr, eat, addr) != 0)
            return -1;
        ptr += eat;
        len -= eat;
        used += eat;
    }
}

int BinaryStreamProcpack::Extract(size_t sockid, size_t processorid, const char* ptr, size_t len, const SockAddr& addr)
{
    size_t used = 0;

    while (true) {
        if (len <= BINARY_PACKLEN_LEN)
            return used;

        unsigned short packlen = *(unsigned short*)ptr;
        log(LOG_DEBUG, "packlen raw: %x", packlen);
        packlen = ntohs(packlen);
        log(LOG_DEBUG, "packlen: %x", packlen);

        if (packlen <= BINARY_PACKLEN_LEN){
            return -1;
        }

        if (len < (size_t)packlen) {
            return used;
        }

        int eat = packlen;
        log(LOG_DEBUG, "Eat %d bytes", eat);
        if (OnPackage(sockid, processorid, ptr, eat, addr) != 0) {
            return -1;
        }
         
        ptr += eat;
        len -= eat;
        used += eat;
    }
}

int AppTextStreamProcpack::OnPackage(size_t sockid, size_t processorid, const char* ptr, size_t len, const SockAddr& addr)
{
    if (queuelimit_ > 0 && queuepackage_p_->GetSize() >= queuelimit_) {
        log(LOG_WARN, "package queue full");
        return -1;
    }

    Package* pack = packageManager_p_->Create();
    if (pack == NULL) {
        log(LOG_WARN, "create package fail, no enough memory or allocator limit");
        return -1;
    }

    pack->SetAllocator(allocator_p_);
    pack->SetHandlerId(sockid);
    pack->SetProcessorId(processorid);
    pack->SetProtocol(protocol_);
    pack->SetData(ptr, len);
    pack->SetPeerAddr(addr);

    gettimeofday(&pack->starttime, NULL);

    if (queuepackage_p_->Push(pack) != 0) {
        log(LOG_WARN, "push package fail");
        pack->Destroy();
        if (errorcb_p_) {
            errorcb_p_->OnInputQueueFull();
        }
#ifdef KEEP_CONNECTION
        return 0;
#else
        return -1;
#endif
    }
    gettimeofday(&pack->puttime, NULL);

    return 0;
}

int AppBinaryStreamProcpack::OnPackage(size_t sockid, size_t proessorid, const char* ptr, size_t len, const SockAddr& addr)
{
    if (queuelimit_ > 0 && queuepackage_p_->GetSize() >= queuelimit_) {
        log(LOG_WARN, "package queue full");
        return -1;
    }

    Package* pack = packageManager_p_->Create();

    if (pack == NULL) {
        log(LOG_WARN, "create package fail, no enough memory or allocator limit");
        return -1;
    }

    pack->SetAllocator(allocator_p_);
    pack->SetHandlerId(sockid);
    pack->SetProcessorId(proessorid);
    pack->SetProtocol(protocol_);
    pack->SetData(ptr, len);
    pack->SetPeerAddr(addr);

    if (queuepackage_p_->Push(pack) != 0) {
        log(LOG_WARN, "push package fail");
        pack->Destroy();
        if (errorcb_p_) {
            errorcb_p_->OnInputQueueFull();
        }
#ifdef KEEP_CONNECTION
        return 0;
#else
        return -1;
#endif
    }

    return 0;
}