/*
	ABAC policy parsing methods
	Copyright (C) <2021>  Hariyala Omkara Naga Sai Varshith
*/

#include <linux/fs.h>
#include <linux/string.h>
#include <linux/slab.h>
#include "avp.h"
#include "policy.h"

void destroy_policy(abac_policy *p) {
	abac_rule *cursor = p->rules;
	abac_rule *to_free = NULL;
	while (cursor != NULL) {
		destroy_avp_list(cursor->user);
		destroy_avp_list(cursor->object);
		destroy_avp_list(cursor->env);
		to_free = cursor;
		cursor = cursor->next;
		if (to_free)kfree(to_free);
}
	kfree(p);
} 

/* Print ABAC access mode */
char *abac_op_str(enum abac_op mode)
{
	if (mode == ABAC_READ)
		return "ABAC_READ";
	if (mode == ABAC_MODIFY)
		return "ABAC_MODIFY";
	return "ABAC_IGNORE";
}

/* convert file permission mask to ABAC access type*/
enum abac_op convert_to_abac_op(int mask)
{
	switch (mask) {
	case MAY_READ:
	case MAY_OPEN:
	case MAY_ACCESS:
		return ABAC_READ;
	case MAY_WRITE:
	case MAY_APPEND:
		return ABAC_MODIFY;
	}
	return ABAC_IGNORE;
}

static abac_rule *parse_line(char *buffer, int start, int end)
{
	abac_rule *r = kcalloc(1, sizeof(abac_rule), GFP_KERNEL);
	// parse user attributes in the rule
	int user_delim_index = findidx(buffer, '|', start, end);
	r->user = parse_avp_section(buffer, start, user_delim_index);
	// parse object attributes in the rule
	int object_delim_index =
		findidx(buffer, '|', user_delim_index + 1, end);
	r->object = parse_avp_section(buffer, user_delim_index + 1,
				      object_delim_index);
	// parse env attributes in the rule
	int env_delim_index = findidx(buffer, '|', object_delim_index + 1, end);
	if (buffer[object_delim_index + 1] != '*') {
		// if environment avps are present in the rule
		r->env = parse_avp_section(buffer, object_delim_index + 1,
					   env_delim_index);
	}
	int op_delim_index = findidx(buffer, '|', object_delim_index + 1, end);
	enum abac_op op = ABAC_IGNORE;
	// printk("debug: %d, %s", op_delim_index, buffer + op_delim_index + 1);
	if (strncmp("MODIFY", buffer + op_delim_index + 1, 6) == 0) {
		op = ABAC_MODIFY;
	} else if (strncmp("READ", buffer + op_delim_index + 1, 4) == 0) {
		op = ABAC_READ;
	}
	r->op = op;
	return r;
}

abac_policy *parse_policy(char *buffer, int length)
{
	if (buffer == NULL || length < 1) {
		return NULL;
	}
	printk(buffer);
	int start = 0;
	int end = 0;
	int i;
	int id = 0;
	abac_policy *policy = kcalloc(1, sizeof(abac_policy), GFP_KERNEL);
	abac_rule *cursor, *head;
	cursor = NULL;
	head = NULL;
	for (i = 0; i < length; i++) {
		if (buffer[i] == '\n') {
			end = i;
			id++;
			if (cursor) {
				cursor->next = parse_line(buffer, start, end);
				cursor->next->id = id;
				cursor = cursor->next;
			} else {
				head = parse_line(buffer, start, end);
				cursor = head;
				cursor->id = id;
			}
			start = i + 1;
		}
	}
	policy->rules = head;
	policy->count = id;
	return policy;
}

void print_abac_policy(abac_policy *p)
{
	abac_rule *r_cursor;
	avp *avp_cursor;
	r_cursor = p->rules;
	printk("Policy contains %d rules", p->count);
	while (r_cursor != NULL) {
		printk("------------------------------------------------");
		printk("ID: %d", r_cursor->id);
		printk("User attributes: ");
		avp_cursor = r_cursor->user;
		while (avp_cursor != NULL) {
			printk("%s=%s, ", avp_cursor->name, avp_cursor->value);
			avp_cursor = avp_cursor->next;
		}
		printk("Object attributes: ");
		avp_cursor = r_cursor->object;
		while (avp_cursor != NULL) {
			printk("%s=%s, ", avp_cursor->name, avp_cursor->value);
			avp_cursor = avp_cursor->next;
		}
		printk("Environment attributes: ");
		avp_cursor = r_cursor->env;
		while (avp_cursor != NULL) {
			printk("%s=%s, ", avp_cursor->name, avp_cursor->value);
			avp_cursor = avp_cursor->next;
		}
		if (r_cursor->op == ABAC_MODIFY)
			printk("MODIFY\n");
		else if (r_cursor->op == ABAC_READ)
			printk("READ\n");
		else
			printk("IGNORE\n");
		r_cursor = r_cursor->next;
	}
	printk("------------------------------------------------");
}
