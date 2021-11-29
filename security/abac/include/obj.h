/*
	Definitions for ABAC Object Attribute structures and methods
	Copyright (C) <2021>  Hariyala Omkara Naga Sai Varshith
*/

#ifndef _ABAC_OBJ_H
#define _ABAC_OBJ_H

#include "avp.h"
#include <linux/limits.h>

typedef struct abac_obj abac_obj;
struct abac_obj {
    char path[PATH_MAX];
    avp *attrs;
    abac_obj *next;
};

abac_obj *parse_obj_attr(char *, int);
void print_obj_attrs(abac_obj *);
void destroy_obj_list(abac_obj *);

#endif /* _ABAC_OBJ_H */
