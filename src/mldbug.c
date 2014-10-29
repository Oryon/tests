#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <linux/mroute.h>
#include <linux/mroute6.h>
#include <net/if.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

uint16_t querier_mrc(int mrd)
{
	if (mrd >= 32768) {
		int exp = 3;

		while ((mrd >> exp) > 0x1fff && exp <= 10)
			++exp;

		if (exp > 10)
			mrd = 0xffff;
		else
			mrd = 0x8000 | ((exp - 3) << 12) | ((mrd >> exp) & 0xfff);
	}
	return htons(mrd);
}

uint8_t querier_qqic(int qqi)
{
	if (qqi >= 128) {
		int exp = 3;

		while ((qqi >> exp) > 0x1f && exp <= 10)
			++exp;

		if (exp > 10)
			qqi = 0xff;
		else
			qqi = 0x80 | ((exp - 3) << 4) | ((qqi >> exp) & 0xf);
	}
	return qqi;
}

int main(int argc, char *argv[])
{
	if(argc < 3) {
		printf("Usage: <ifname> <group>\n");
		return -1;
	}

	int fd;
	int one = 1;
	int two = 2;

	struct {
		struct ip6_hbh hdr;
		struct ip6_opt_router rt;
		uint8_t pad[2];
	} ipv6_rtr_alert = {
		.hdr = {0, 0},
		.rt = {IP6OPT_ROUTER_ALERT, 2, {0, IP6_ALERT_MLD}},
		.pad = {0, 0}
	};

	printf("Opening ICMPv6 socket\n");
	if (((fd = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6)) < 0) ||
			setsockopt(fd, IPPROTO_IPV6, MRT6_INIT, &one, sizeof(one)) ||
			setsockopt(fd, IPPROTO_IPV6, IPV6_RECVHOPOPTS, &one, sizeof(one)) ||
			setsockopt(fd, IPPROTO_IPV6, IPV6_RECVHOPLIMIT, &one, sizeof(one)) ||
			setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &one, sizeof(one)) ||
			setsockopt(fd, IPPROTO_RAW, IPV6_CHECKSUM, &two, sizeof(two)) ||
			setsockopt(fd, IPPROTO_IPV6, IPV6_HOPOPTS, &ipv6_rtr_alert, sizeof(ipv6_rtr_alert)))
		goto err;

	printf("Getting interface %s index\n", argv[1]);
	int index;
	if(!(index = if_nametoindex(argv[1])))
		goto err;
	printf("Index is %d\n", index);

	printf("Parsing group address %s\n", argv[2]);
	struct in6_addr group;
	if(!inet_pton(AF_INET6, argv[2], &group)) {
		errno = EINVAL;
		goto err;
	}

	printf("Subscribing to %s\n", argv[2]);
	struct group_req greq = {
			.gr_interface = index,
	};
	((struct sockaddr_in6 *)(&greq.gr_group))->sin6_family = AF_INET6;
	((struct sockaddr_in6 *)(&greq.gr_group))->sin6_addr = group;
	((struct sockaddr_in6 *)(&greq.gr_group))->sin6_scope_id = index;

	if(setsockopt(fd, SOL_IPV6, MCAST_JOIN_GROUP, &greq, sizeof(greq)))
		goto err;

	printf("Send a group specific query\n");
	struct mld_query {
		struct mld_hdr mld;
		uint8_t s_qrv;
		uint8_t qqic;
		uint16_t nsrc;
		struct in6_addr addrs[0];
	} query = {
#define MRC_MS 500
			.mld = {.mld_icmp6_hdr = {MLD_LISTENER_QUERY, 0, 0, {.icmp6_un_data16 = {querier_mrc(MRC_MS), 0}}},
					.mld_addr = group},
#define ROBUSTNESS 6
#define SUPPRESS 0
#define QQIC_S 20
			.s_qrv = (ROBUSTNESS & 0x7) | (SUPPRESS),
			.qqic = querier_qqic(QQIC_S),
			.nsrc = 0
	};

#define IPV6_ALL_NODES_INIT		{ { { 0xff,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0,0x1 } } }
	//struct sockaddr_in6 dst = {.sin6_family = AF_INET6, .sin6_addr = IPV6_ALL_NODES_INIT, .sin6_scope_id = index};
	struct sockaddr_in6 dst = {.sin6_family = AF_INET6, .sin6_addr = group, .sin6_scope_id = index};

	if((sendto(fd, &query, sizeof(query), 0, (struct sockaddr *)&dst, sizeof(dst))) != sizeof(query) )
		goto err;

	printf("Looping for incoming messages\n");
	while(1) {
		struct sockaddr_in6 from;
		uint8_t buf[1024];
		uint8_t bufaddr[INET6_ADDRSTRLEN];
		socklen_t socklen =  sizeof(from);
		ssize_t len;
		if((len = recvfrom(fd, buf, 1024, 0, (struct sockaddr *)&from, &socklen)) < 0)
			goto err;

		inet_ntop(AF_INET6, &from.sin6_addr, bufaddr, INET6_ADDRSTRLEN);
		struct mld_hdr *mld = (struct mld_hdr *)buf;
		printf("Received message of length %d from %s (mld type %d)\n", (int)len, bufaddr, mld->mld_icmp6_hdr.icmp6_type);
	}

	return 0;
err:
	perror("Error");
	return -1;
}
