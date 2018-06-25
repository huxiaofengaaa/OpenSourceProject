#include "event2/event.h"


int main(int argc, char** argv)
{
	struct event_base *event_base = event_base_new();


	event_base_dispatch(event_base);
	return 0;
}


