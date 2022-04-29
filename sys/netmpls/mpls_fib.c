/*-
 * Copyright (c) 2021
 * 	Neel Chauhan <nc@FreeBSD.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/* TODO: Clean #includes */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/lock.h>
#include <sys/rmlock.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/kernel.h>

#include <net/if.h>
#include <net/if_var.h>
#include <net/if_dl.h>
#include <net/route.h>
#include <net/route/route_ctl.h>
#include <net/route/route_var.h>
#include <net/route/fib_algo.h>
#include <net/route/nhop.h>
#include <net/toeplitz.h>
#include <net/vnet.h>

#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_fib.h>

#include "mpls.h"

#ifdef INET

/*
 * Function returning prefix match data along with the nexthop data.
 * Intended to be used by the control plane code.
 * Supported flags:
 *  NHR_UNLOCKED: do not lock radix during lookup.
 * Returns pointer to rtentry and raw nexthop in @rnd. Both rtentry
 *  and nexthop are safe to use within current epoch. Note:
 * Note: rnd_nhop can actually be the nexthop group.
 */
struct rtentry *
fib_mpls_lookup_rt(uint32_t fibnum, struct sockaddr *dst, uint32_t flags,
    struct route_nhop_data *rnd)
{
	RIB_RLOCK_TRACKER;
	struct rib_head *rh;
	struct radix_node *rn;
	struct rtentry *rt;

	KASSERT((fibnum < rt_numfibs), ("fib4_lookup_rt: bad fibnum"));
	rh = rt_tables_get_rnh(fibnum, AF_MPLS);
	if (rh == NULL)
		return (NULL);

	rt = NULL;
	if (!(flags & NHR_UNLOCKED))
		RIB_RLOCK(rh);
	rn = rh->rnh_matchaddr((void *)dst, &rh->head);
	if (rn != NULL && ((rn->rn_flags & RNF_ROOT) == 0)) {
		rt = (struct rtentry *)rn;
		rnd->rnd_nhop = rt->rt_nhop;
		rnd->rnd_weight = rt->rt_weight;
	}
	if (!(flags & NHR_UNLOCKED))
		RIB_RUNLOCK(rh);

	return (rt);
}

#endif
