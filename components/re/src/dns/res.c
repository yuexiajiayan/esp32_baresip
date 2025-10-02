/**
 * @file res.c  Get DNS Server IP using resolv
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <sys/types.h>
#include <netinet/in.h>
 
#include <string.h>
#include <re_types.h>
#include <re_fmt.h>
#include <re_mbuf.h>
#include <re_list.h>
#include <re_sa.h>
#include <re_dns.h>
#include <re_mem.h>
#include "dns.h"


int get_resolv_dns(char *domain, size_t dsize, struct sa *nsv, uint32_t *n)
{
	return ENOENT;
}
