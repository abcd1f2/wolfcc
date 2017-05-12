#include "utils/destroyable.h"

void DeleteDestroyCallback::call(Destroyable* p)
{
    delete p;
}

DeleteDestroyCallback* DeleteDestroyCallback::Instance()
{
    static DeleteDestroyCallback callback;
    return &callback;
}