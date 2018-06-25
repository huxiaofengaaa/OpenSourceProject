#include "event2/event.h"
#include <event2/event_struct.h>
#include <event2/util.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <sys/stat.h>

int event_is_persistent = 0;
struct timeval lasttime;

void timeout_cb(evutil_socket_t fd, short event, void *arg)
{
	struct event *timeout = arg;
	
	struct timeval newtime, difference;
	double elapsed;
	
	evutil_gettimeofday(&newtime, NULL);
	evutil_timersub(&newtime, &lasttime, &difference);
	elapsed = difference.tv_sec + (difference.tv_usec / 1.0e6);
	
	printf("timeout_cb called at %d: %.3f seconds elapsed.\n", (int)newtime.tv_sec, elapsed);
	lasttime = newtime;
	
	if (! event_is_persistent) 
	{
		struct timeval tv;
		evutil_timerclear(&tv);
		tv.tv_sec = 2;
		event_add(timeout, &tv);
	}
}

int main(int argc, char** argv)
{
	// 01 create event_base
	struct event_base *base = event_base_new();
	
	// 02 create event
	int flags;
	struct event timeout;
	if(event_is_persistent == 0)
	{
		flags = 0;
	}
	else
	{
		flags = EV_PERSIST;
	}
	event_assign(&timeout, base, -1, flags, timeout_cb, (void*) &timeout);
	
	// 03 add timer event to set for monitor
	struct timeval tv;
	evutil_timerclear(&tv);
	tv.tv_sec = 2;
	event_add(&timeout, &tv);
	
	// 04 set current time
	evutil_gettimeofday(&lasttime, NULL);
	
	// 05 run loop event_base
	event_base_dispatch(base);
	
	// 06 release event_base resource
	event_base_free(base);
	return 0;
}