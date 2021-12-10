#ifndef __ADDR_H__
#define __ADDR_H__

int data_attr_cb(const struct nlattr *attr, void *data);

int request_ipv4_address_common(mnl_cb_t data_cb);
int request_ipv6_address_common(mnl_cb_t data_cb);

#endif /* __ADDR_H__ */
