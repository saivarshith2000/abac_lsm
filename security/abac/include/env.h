/*
	Definitions for ABAC Environmental Attribute parsing methods
	Copyright (C) <2021>  Hariyala Omkara Naga Sai Varshith
*/

#ifndef _ABAC_ENV_H
#define _ABAC_ENV_H
#include "avp.h"

avp *parse_env_attr(char *, int);
void print_env_attrs(avp *);

#endif /* _ABAC_ENV_H */
