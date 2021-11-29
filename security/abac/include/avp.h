/*
	Definitions for Attribute Value Pair (AVP) strctures and methods
	Copyright (C) <2021>  Hariyala Omkara Naga Sai Varshith
*/

#ifndef _ABAC_AVP_H
#define _ABAC_AVP_H

typedef struct avp avp;
struct avp {
    char *name;
    char *value;
    avp *next;
};

int findidx(char *, char, int, int);
avp *parse_avp(char *, int, int);
avp *parse_avp_section(char *, int, int);
void destroy_avp_list(avp *head);

#endif /* _ABAC_AVP_H */
