/*	$NetBSD: xdr_mem.c,v 1.16 2003/03/16 15:42:22 christos Exp $	*/

/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 * 
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 * 
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 * 
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 * 
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 * 
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */


/*
 * xdr_mem.h, XDR implementation using memory buffers.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 * If you have some data to be interpreted as external data representation
 * or to be converted to external data representation in a memory buffer,
 * then this is the package for you.
 *
 */

#include "xdr.h"
#ifdef __weak_alias
__weak_alias(xdrmem_create,_xdrmem_create)
#endif
//coverity[unnecessary_header], need following header file to avoid link error for undefined lswap
#include "util_lswap.h"
static void xdrmem_destroy __P((XDR *));
static bool_t xdrmem_getlong_aligned __P((XDR *, long *));
static bool_t xdrmem_putlong_aligned __P((XDR *, const long *));
static bool_t xdrmem_getlong_unaligned __P((XDR *, long *));
static bool_t xdrmem_putlong_unaligned __P((XDR *, const long *));
static bool_t xdrmem_getbytes __P((XDR *, char *, u_int));
static bool_t xdrmem_putbytes __P((XDR *, const char *, u_int));
/* XXX: w/64-bit pointers, u_int not enough! */
static u_int xdrmem_getpos __P((XDR *));
static bool_t xdrmem_setpos __P((XDR *, u_int));
static int32_t *xdrmem_inline_aligned __P((XDR *, u_int));
static int32_t *xdrmem_inline_unaligned __P((XDR *, u_int));
//static void* xdrmem_alloc __P((XDR *, int));
//static void xdrmem_free __P((XDR *, void*, int));

#define TEMP_STR_LEN 40
#define XDR_LOG_STR_CAT(xdrs,src) strncat(xdrs->x_logbuffer, src, (xdrs->x_logsize - strlen(xdrs->x_logbuffer) - 1));

#define XDR_LOG_BUF_AVAILABLE(xdrs,reqSize) (xdrs->x_logbuffer != NULL && (strlen(xdrs->x_logbuffer)+reqSize < xdrs->x_logsize) )?TRUE:FALSE

static const struct	xdr_ops xdrmem_ops_aligned = {
	xdrmem_getlong_aligned,
	xdrmem_putlong_aligned,
	xdrmem_getbytes,
	xdrmem_putbytes,
	xdrmem_getpos,
	xdrmem_setpos,
	xdrmem_inline_aligned,
	xdrmem_destroy,
	NULL,
	NULL,//xdrmem_alloc,
	NULL//xdrmem_free
};

static const struct	xdr_ops xdrmem_ops_unaligned = {
	xdrmem_getlong_unaligned,
	xdrmem_putlong_unaligned,
	xdrmem_getbytes,
	xdrmem_putbytes,
	xdrmem_getpos,
	xdrmem_setpos,
	xdrmem_inline_unaligned,
	xdrmem_destroy,
	NULL,
	NULL,//xdrmem_alloc,
	NULL//xdrmem_free
};

void xdrmem_logbytes(XDR *xdrs, const char *addr, u_int len);


/*
 * The procedure xdrmem_create initializes a stream descriptor for a
 * memory buffer.  
 */
void
xdrmem_create(XDR *xdrs, 
              char *addr, 
              u_int size, 
              char *logbuffer, 
              u_int logsize, 
              enum xdr_op op)
{
	xdrs->x_logbuffer = logbuffer;
	xdrs->x_logsize = logsize;
	if(logbuffer != NULL)
	{
		memset(logbuffer,0xCD,logsize);
	}

	xdrs->x_op = op;
	xdrs->x_ops = ((unsigned long)addr & (sizeof(int32_t) - 1))
	    ? &xdrmem_ops_unaligned : &xdrmem_ops_aligned;
	xdrs->x_private = xdrs->x_base = addr;
	xdrs->x_handy = size;
	xdrs->x_basiclogbuffer = NULL;
	
	xdrs->x_decodeCurbuffer = xdrs->x_decodebuffer = NULL;
	xdrs->x_decodeCurBufsize = xdrs->x_decodeBufsize = 0;

	if((xdrs->x_ops->x_alloc) && size > 0)
	{
		xdrs->x_decodeCurbuffer = xdrs->x_decodebuffer = (char*) mem_alloc(size);
		xdrs->x_decodeCurBufsize = xdrs->x_decodeBufsize = size;
	}

}

/*ARGSUSED*/
static void xdrmem_destroy(XDR *xdrs)
{
	if(xdrs && xdrs->x_ops->x_free && xdrs->x_decodebuffer)
		mem_free(xdrs->x_decodebuffer, xdrs->x_decodeBufsize);
}

/*

static void*	xdrmem_alloc(XDR *xdrs, int sz)
{
	void* buf = NULL;
	int req_sz = sz/sizeof(int) + sizeof(int);

	if(xdrs->x_decodeCurbuffer && req_sz <  xdrs->x_decodeCurBufsize)
	{
		buf = xdrs->x_decodeCurbuffer;
		xdrs->x_decodeCurBufsize -= req_sz;
		xdrs->x_decodeCurbuffer += req_sz;
	}
	return buf;
}


static void	xdrmem_free(XDR *xdrs, void* buf, int sz)
{

}
*/

static bool_t
xdrmem_getlong_aligned(XDR *xdrs, long *lp)
{

	if (xdrs->x_handy < sizeof(int32_t))
		return (FALSE);
	xdrs->x_handy -= sizeof(int32_t);
	*lp = ntohl(*(u_int32_t *)xdrs->x_private);
	xdrs->x_private = (char *)xdrs->x_private + sizeof(int32_t);

	if(XDR_LOG_BUF_AVAILABLE(xdrs, TEMP_STR_LEN))
	{
		char temp[TEMP_STR_LEN];
		snprintf(temp,TEMP_STR_LEN,"{%lx} ",*lp);
		XDR_LOG_STR_CAT(xdrs,temp);
	}
	return (TRUE);
}

static bool_t
xdrmem_putlong_aligned(	XDR *xdrs, const long *lp)
{

	if (xdrs->x_handy < sizeof(int32_t))
		return (FALSE);
	xdrs->x_handy -= sizeof(int32_t);
	*(u_int32_t *)xdrs->x_private = htonl((u_int32_t)*lp);
	xdrs->x_private = (char *)xdrs->x_private + sizeof(int32_t);

	if(XDR_LOG_BUF_AVAILABLE(xdrs, TEMP_STR_LEN))
	{
		char temp[TEMP_STR_LEN];
		snprintf(temp,TEMP_STR_LEN,"{%lx} ",*lp);
		XDR_LOG_STR_CAT(xdrs,temp);
	}
	return (TRUE);
}

static bool_t
xdrmem_getlong_unaligned(XDR *xdrs, long *lp)
{
	u_int32_t l;

	if (xdrs->x_handy < sizeof(int32_t))
		return (FALSE);
	xdrs->x_handy -= sizeof(int32_t);
	memmove(&l, xdrs->x_private, sizeof(int32_t));
	*lp = ntohl(l);
	xdrs->x_private = (char *)xdrs->x_private + sizeof(int32_t);
	return (TRUE);
}

static bool_t
xdrmem_putlong_unaligned(XDR *xdrs, const long *lp)
{
	u_int32_t l;

	if (xdrs->x_handy < sizeof(int32_t))
		return (FALSE);
	xdrs->x_handy -= sizeof(int32_t);
	l = htonl((u_int32_t)*lp);
	memmove(xdrs->x_private, &l, sizeof(int32_t));
	xdrs->x_private = (char *)xdrs->x_private + sizeof(int32_t);
	return (TRUE);
}

static bool_t isAlpha(char y)
{
	if(y >= ' ' && y <= 'z')
		return TRUE;

	return FALSE;
}

static bool_t
xdrmem_getbytes(XDR *xdrs, char *addr, u_int len)
{

	if (xdrs->x_handy < len)
		return (FALSE);
	xdrs->x_handy -= len;
	memmove(addr, xdrs->x_private, len);
	xdrs->x_private = (char *)xdrs->x_private + len;
	xdrmem_logbytes(xdrs, addr, len);
	return (TRUE);
}

static bool_t
xdrmem_putbytes(XDR *xdrs, 
                const char *addr, 
                u_int len)
{

	if (xdrs->x_handy < len)
		return (FALSE);
	xdrs->x_handy -= len;
	memmove(xdrs->x_private, addr, len);
	xdrs->x_private = (char *)xdrs->x_private + len;
	xdrmem_logbytes(xdrs, addr, len);
	return (TRUE);
}

static u_int
xdrmem_getpos(XDR * xdrs)
{

	/* XXX w/64-bit pointers, u_int not enough! */
	return (u_int)((u_long)xdrs->x_private - (u_long)xdrs->x_base);
}

static bool_t
xdrmem_setpos(XDR *xdrs, u_int pos)
{
	char *newaddr = xdrs->x_base + pos;
	char *lastaddr = (char *)xdrs->x_private + xdrs->x_handy;

	if ((long)newaddr > (long)lastaddr)
		return (FALSE);
	xdrs->x_private = newaddr;
	xdrs->x_handy = (int)((long)lastaddr - (long)newaddr);
	return (TRUE);
}

static int32_t *
xdrmem_inline_aligned(XDR *xdrs, u_int len)
{
	int32_t *buf = 0;

	if (xdrs->x_handy >= len) {
		xdrs->x_handy -= len;
		buf = (int32_t *)xdrs->x_private;
		xdrs->x_private = (char *)xdrs->x_private + len;
	}
	return (buf);
}

/* ARGSUSED */
static int32_t *
xdrmem_inline_unaligned(XDR *xdrs, u_int len)
{
   if (xdrs || len) { } //fixes compiler warnings
	return (0);
}

void xdrmem_log(XDR* xdrs, char* str)
{
	if(XDR_LOG_BUF_AVAILABLE(xdrs,5 ))
	{
		XDR_LOG_STR_CAT(xdrs,"\r\n");
		XDR_LOG_STR_CAT(xdrs,str);
	}
}

void xdrmem_log_start(XDR* xdrs, char* str)
{
	if(xdrs->x_logbuffer != NULL && (strlen(str)+5 < xdrs->x_logsize) )
	{
		memset(xdrs->x_logbuffer,0,xdrs->x_logsize);
		strncpy(xdrs->x_logbuffer,str, strlen(str));
	}
}

#define MAX_ARRAY_LEN 300
#define MAX_BYTES_PER_LINE 20

void xdrmem_logbytes(XDR *xdrs, 
                     const char *addr, 
                     u_int len)
{

	//Check if we have any space left
	if(XDR_LOG_BUF_AVAILABLE(xdrs, TEMP_STR_LEN))
	{
		int availSize;
		u_int i, max_len;
		char temp[TEMP_STR_LEN];
		bool_t alphaFound = TRUE;

		//Check for avail buffer
		availSize = xdrs->x_logsize - strlen(xdrs->x_logbuffer) - TEMP_STR_LEN;
		//3 bytes per BYTE including comma and \r\n per 20 entries
		max_len = (availSize/3) - (availSize/MAX_BYTES_PER_LINE)*2 - 10;
		//choose the minimum
		max_len = (len < max_len ) ? len : max_len;
		
		//Start
		XDR_LOG_STR_CAT(xdrs,"{");
		for(i=0;i<max_len;i++)
		{
			//Log as chars until you find non-ascii char
			if(isAlpha(addr[i]) && alphaFound)
				snprintf(temp,TEMP_STR_LEN,"%c",addr[i]);
			else
			{
				alphaFound = FALSE;
				snprintf(temp,TEMP_STR_LEN,"%2x,",(unsigned char)(addr[i]));
			}
			//append
			XDR_LOG_STR_CAT(xdrs,temp);

			//wrap up the line
			if( i > 0 && (i%MAX_BYTES_PER_LINE) == 0)
			{
				XDR_LOG_STR_CAT(xdrs,"\r\n");
			}
			
			//Should not come here, but check in case
			if(strlen(xdrs->x_logbuffer)+TEMP_STR_LEN > xdrs->x_logsize)
				break;
		}
		
		if(XDR_LOG_BUF_AVAILABLE(xdrs, TEMP_STR_LEN))
		{
			//Did we log every thing?
			if(len > MAX_ARRAY_LEN )
				XDR_LOG_STR_CAT(xdrs,"...");
			//Done
			XDR_LOG_STR_CAT(xdrs,"}");
		}
	}
}
