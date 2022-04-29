# $FreeBSD$

.PATH: ${.CURDIR}/../../netmpls
CFLAGS += -I${.CURDIR}/../../ -D INET -D VIMAGE
KMOD	= netmpls
SRCS	= mpls.h
SRCS	+= in4_cksum.c
SRCS	+= mpls_fib.c
SRCS	+= mpls_input.c
SRCS	+= mpls_output.c
SRCS	+= mpls_proto.c
SRCS	+= mpls_raw.c
SRCS	+= mpls_shim.c

.include <bsd.kmod.mk>
