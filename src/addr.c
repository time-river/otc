/*
 * (C) 2021 by river <river@vvl.me>
 *
 * Get local host address.
 *
 */

#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <libmnl/libmnl.h>

#include <linux/rtnetlink.h>

#include "log.h"

int data_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	/* skip unsupported attribute in user-space */
	if (mnl_attr_type_valid(attr, IFA_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case IFA_ADDRESS:
		if (mnl_attr_validate(attr, MNL_TYPE_BINARY) < 0) {
			pr_perror("mnl_attr_validate failed");
			return MNL_CB_ERROR;
		}
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

/* Just as an example, copied from `libmnl/examples/rtnl/rtnl-addr-dump.c` */
static int default_data_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[IFLA_MAX+1] = {};
	struct ifaddrmsg *ifa = mnl_nlmsg_get_payload(nlh);

	pr_info("index=%d family=%d ", ifa->ifa_index, ifa->ifa_family);

	mnl_attr_parse(nlh, sizeof(*ifa), data_attr_cb, tb);
	pr_cont("addr=");
	if (tb[IFA_ADDRESS]) {
		void *addr = mnl_attr_get_payload(tb[IFA_ADDRESS]);
		char out[INET6_ADDRSTRLEN];

		if (inet_ntop(ifa->ifa_family, addr, out, sizeof(out)))
			printf("%s ", out);
	}
	pr_cont("scope=");
	switch(ifa->ifa_scope) {
	case RT_SCOPE_UNIVERSE:
		pr_cont("global ");
		break;
	case RT_SCOPE_SITE:
		pr_cont("site ");
		break;
	case RT_SCOPE_LINK:
		pr_cont("link ");
		break;
	case RT_SCOPE_HOST:
		pr_cont("host ");
		break;
	case RT_SCOPE_NOWHERE:
		pr_cont("nowhere ");
		break;
	default:
		pr_cont("unknow(%d) ", ifa->ifa_scope);
		break;
	}

	pr_cont("\n");
	return MNL_CB_OK;
}

static int request_address_common(uint8_t family, mnl_cb_t data_cb) {
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	struct rtgenmsg *rt;
	unsigned int seq, portid;
	int retval = -1;

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type = RTM_GETADDR;
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	nlh->nlmsg_seq = seq = time(NULL);
	rt = mnl_nlmsg_put_extra_header(nlh, sizeof(struct rtgenmsg));
	rt->rtgen_family = family;

	nl = mnl_socket_open(NETLINK_ROUTE);
	if (nl == NULL) {
		pr_perror("mnl_socket_open failed");
		return -1;
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		pr_perror("mnl_socket_bind failed");
		goto close;
	}
	portid = mnl_socket_get_portid(nl);

	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
		pr_perror("mnl_socket_sendto failed");
		goto close;
	}

	retval = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	while (retval > 0) {
		retval = mnl_cb_run(buf, retval, seq, portid, data_cb, NULL);
		if (retval <= MNL_CB_STOP)
			break;
		retval = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	}

	retval = 0;
close:
	mnl_socket_close(nl);

	return retval;
}

int request_ipv4_address_common(mnl_cb_t data_cb) {
	if (data_cb == NULL)
		data_cb = default_data_cb;

	return request_address_common(AF_INET, data_cb);
}

int request_ipv6_address_common(mnl_cb_t data_cb) {
	if (data_cb == NULL)
		data_cb = default_data_cb;

	return request_address_common(AF_INET6, data_cb);
}
