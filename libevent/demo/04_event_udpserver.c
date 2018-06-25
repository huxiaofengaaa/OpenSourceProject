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

int create_an_udp_socket_server()
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
	bind(sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr_in));

	return sockfd;
}

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

void socket_data_ready_callback(evutil_socket_t sockfd, short events, void* arg)
{
	struct event *sock_event = arg;

	struct sockaddr_in cli_addr;
	socklen_t len = sizeof(struct sockaddr_in);
	bzero(&cli_addr, sizeof(struct sockaddr_in));

	char buf[4096] = { 0 };
	recvfrom(g_sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&cli_addr, &len);

	printf("in udp server, read from client - %s\n", buf);

	sendto(g_sockfd, buf, strlen(buf), 0, (struct sockaddr *)&cli_addr, len);
}

int main(int argc, char** argv)
{
	struct event_base *event_base = event_base_new();

	{
		struct event *signal_int = evsignal_new(event_base, SIGINT, signal_cb, event_self_cbarg());
		event_add(signal_int, NULL);
	}

	{
		g_sockfd = create_an_udp_socket_server();
		if(g_sockfd < 0)
		{
			return -1;
		}
		printf("create udp socket server success...\n");
		
		struct event* sock_event = event_new(event_base, g_sockfd, EV_READ|EV_PERSIST,
			socket_data_ready_callback, event_self_cbarg());
		event_add(sock_event, NULL);
	}

	event_base_dispatch(event_base);
	event_base_free(event_base);
	return 0;
}

