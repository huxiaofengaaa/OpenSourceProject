#include "event2/event.h"
#include <stdio.h>
#include <signal.h>

int called = 0;

void signal_cb(evutil_socket_t fd, short event, void *arg)
{
	struct event *signal = arg;
	printf("signal_cb: got signal %d\n", event_get_signal(signal));
	
	// 创建信号event时，带有EV_SIGNAL|EV_PERSIST标记，因此不会自动移除
	if (called >= 2)
		event_del(signal);

	called++;	
}

int main(int argc, char** argv)
{
	struct event_base *base = event_base_new();
	struct event *signal_int = evsignal_new(base, SIGINT, signal_cb, event_self_cbarg());
	event_add(signal_int, NULL);
	event_base_dispatch(base);
	event_free(signal_int);
	event_base_free(base);	
	return 0;
}