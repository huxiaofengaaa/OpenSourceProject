#include "event2/event.h"
#include "event2/dns.h"

struct event_base *event_base = NULL;
struct evdns_base *evdns_base = NULL;

void evdns_cb(int result, char type, int count, int ttl, void *addresses, void *arg)
{
	unsigned char* addr = (char*)addresses;

	if(result == DNS_ERR_NONE)
	{
		printf("type: %s", type == DNS_IPv4_A ? "DNS_IPv4_A" : type == DNS_PTR ? "DNS_PTR" : "DNS_IPv6_AAAA");
		printf(", ttl:%d", ttl);
		for(int i = 0 ; i < count ; i++)
		{
			printf(", address[%d]:%d.%d.%d.%d ", i + 1, addr[0 + i*4], addr[1 + i*4], addr[2 + i*4], addr[3 + i*4]);
		}
		printf("\n");
	}
	else
	{
		printf("dns resolve error\n");
	}

	evdns_base_clear_nameservers_and_suspend(evdns_base);
}

int main(int argc, char** argv)
{
	if(argc < 2)
	{
		printf("usage: ./03_event_dns host name\n");
		return 0;
	}
	event_base = event_base_new();
	evdns_base =  evdns_base_new(event_base,  0);

	evdns_base_nameserver_ip_add(evdns_base, "8.8.8.8");
	evdns_base_nameserver_ip_add(evdns_base, "114.114.114.114");
	printf("nameserver count: %d\n", evdns_base_count_nameservers(evdns_base));

	struct evdns_request *req = evdns_base_resolve_ipv4(evdns_base, argv[1], 0, evdns_cb, NULL);

	event_base_dispatch(event_base);

	event_base_free(event_base);
	evdns_base_free(evdns_base, 0);
	return 0;
}


