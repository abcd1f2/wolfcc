message(STATUS "buidl wolfcc main dir")

include_directories(../dep/libconfig-1.5/lib ../dep/libevent-2.1.8-stable/include ../dep/msgpack-c-cpp-2.1.1/include)
add_definitions(-g -W -Wall)
link_libraries(wolfccbase libeventreactor config messagepack rt)

set(executable_name "wolfcc-1.0")
set(executable_output_path ./)

add_library(wolfccbase static
    nets/acceptor.cpp
    nets/addressfilteracceptor.cpp
    nets/connecteventhandler.cpp
    nets/connector.cpp
    nets/eventhandler.cpp
    nets/inetaddr.cpp
    nets/libeventreactor.cpp
    nets/networkbase.cpp
    nets/pusheventhandler.cpp
    nets/reactor.cpp
    nets/selectreactor.cpp
    nets/sockaddr.cpp
    nets/socket.cpp
    nets/socketpairhandler.cpp
    nets/udplistener.cpp
    nets/unixaddr.cpp
    pros/package.cpp
    pros/packprocessor.cpp
    pros/procpack.cpp
    pros/protocolstream.cpp
    pros/stream.cpp
    utils/allocator.cpp
    utils/datablock.cpp
    utils/destroyable.cpp
    utils/errorcallback.cpp
    utils/handlerregistry.cpp
    utils/logging.cpp
    utils/mutex.cpp
    utils/mutexcond.cpp
    utils/notify.cpp
    utils/thread.cpp
    utils/threadpool.cpp
    utils/workthreadpool.cpp
    utils/wtime.cpp
)

add_executable($(executable_name)
    backgroudio.cpp
    configmanager.cpp
    dbmanager.cpp
    netmanager.cpp
    serverapp.cpp
    wolfcc.cpp
)
