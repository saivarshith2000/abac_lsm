/*
	Definitions for ABAC User Attribute structures and methods
	Copyright (C) <2021>  Hariyala Omkara Naga Sai Varshith
*/

#ifndef _ABAC_USER_H
#define _ABAC_USER_H

#include "avp.h"

typedef struct abac_user abac_user;
struct abac_user {
    unsigned int uid;
    avp *attrs;
    abac_user *next;
};

abac_user *parse_user_attr(char *, int);
void print_user_attrs(abac_user *);
void destroy_user_list(abac_user *);

#endif /* _ABAC_USER_H */
