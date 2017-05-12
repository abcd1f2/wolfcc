#include <string.h>
#include <cassert>
#include <new>
#include "nets/eventhandler.h"
#include "nets/reactor.h"

void Reactor::Run()
{
	running = true;
	while(running)
	{
		int ret = RunOnce();
		if (ret == 1)
		{
			// No events
			running = false;
			break;
		}
	}
}


