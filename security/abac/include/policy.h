/*
	Definitions for ABAC Policy structures and methods
	Copyright (C) <2021>  Hariyala Omkara Naga Sai Varshith
*/

#ifndef _ABAC_POLICY_H
#define _ABAC_POLICY_H

#include "avp.h"

/* Supported modes of object access */
enum abac_op { ABAC_READ, ABAC_MODIFY, ABAC_IGNORE };

typedef struct abac_rule abac_rule;
struct abac_rule {
    avp *user;
    avp *object;
    avp *env;
    enum abac_op op;
    abac_rule *next;
    int id;
};

typedef struct {
    abac_rule *rules;
    int count;
} abac_policy;

char *abac_op_str(enum abac_op);
enum abac_op convert_to_abac_op(int);
abac_policy *parse_policy(char *, int);
void print_abac_policy(abac_policy *);
void destroy_policy(abac_policy *);

#endif /* _ABAC_POLICY_H */
