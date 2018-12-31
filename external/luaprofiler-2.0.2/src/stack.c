/*
** LuaProfiler
** Copyright Kepler Project 2005-2007 (http://www.keplerproject.org/luaprofiler)
** $Id: stack.c,v 1.4 2007/08/22 19:23:53 carregal Exp $
*/

/*****************************************************************************
stack.c:
   Simple stack manipulation
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

void lprofS_push(lprofS_STACK *p, lprofS_STACK_RECORD r) {
lprofS_STACK q;
        q=(lprofS_STACK)malloc(sizeof(lprofS_STACK_RECORD));
        *q=r;
        q->next=*p;
        *p=q;
}

lprofS_STACK_RECORD lprofS_pop(lprofS_STACK *p) {
lprofS_STACK_RECORD r;
lprofS_STACK q;

        r=**p;
        q=*p;
        *p=(*p)->next;
        free(q);
        return r;
}
