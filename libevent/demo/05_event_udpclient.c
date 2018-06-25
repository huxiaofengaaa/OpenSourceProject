#include "event2/event.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <signal.h>


#define SERVER_PORT 8776
#define SERVER_IP "127.0.0.1"

int g_sockfd = 0;

void signal_cb(evutil_socket_t fd, short event, void *arg)
{
	static int called = 0;
	struct event *signal_event = arg;
	printf("signal_cb: got signal %d\n", event_get_signal(signal_event));
	
	// 创建信号event时，带有EV_SIGNAL|EV_PERSIST标记，因此不会自动移除
	if (called >= 2)
	{
		struct event_base* event_base = event_get_base(signal_event);
		event_base_loopbreak(event_base);
	}
	called++;
}

void socket_read_cb(evutil_socket_t sockfd, short events, void* arg)
{
	struct event *sock_event = arg;

	char buf[4096] = { 0 };
	recv(g_sockfd, buf, sizeof(buf), 0);

	printf("in udp client, read from server - %s\n", buf);
}

void timeout_cb(evutil_socket_t fd, short event, void *arg)
{
	struct event *event_timeout = arg;

	printf("call event_timeout callback ....\n");

	char buf[] = "abcdefghjg";
	send(g_sockfd, buf, sizeof(buf), 0);
	
	struct timeval tv;
	evutil_timerclear(&tv);
	tv.tv_sec = 2;
	event_add(event_timeout, &tv);
}

int create_an_udp_socket_client()
{
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0)
	{
		perror("create socket failed\n");
		return -1;
	}

	struct sockaddr_in ser_addr;
	bzero(&ser_addr, sizeof(struct sockaddr_in));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = SERVER_PORT;
	ser_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

	connect(sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr_in));

	return sockfd;
}

int main(int argc, char** argv)
{
	struct event_base *event_base = event_base_new();

	// monitor signal event
	{
		struct event *signal_int = evsignal_new(event_base, SIGINT, signal_cb, event_self_cbarg());
		event_add(signal_int, NULL);
	}

	// monitor sockfd ready for read event
	{
		g_sockfd = create_an_udp_socket_client();
		if(g_sockfd < 0)
		{
			return -1;
		}
		printf("create udp socket client and connect success...\n");
		
		struct event* sock_event = event_new(event_base, g_sockfd, EV_READ|EV_PERSIST,
			socket_read_cb, event_self_cbarg());
		event_add(sock_event, NULL);
	}

	// montor timeout event
	{
		struct event* event_timeout = event_new(event_base, -1, 0, timeout_cb, event_self_cbarg());
		struct timeval tv;
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		event_add(event_timeout, &tv);
	}

	event_base_dispatch(event_base);
	event_base_free(event_base);
	return 0;
}

