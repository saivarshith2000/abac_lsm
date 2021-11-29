/*
	ABAC access resolution methods
	Copyright (C) <2021>  Hariyala Omkara Naga Sai Varshith
*/

#include "resolve.h"
#include "policy.h"
#include "avp.h"
#include "user.h"
#include "obj.h"
#include "abacfs.h"
#include <linux/string.h>
#include <linux/kernel.h>

static avp *get_user_avp(unsigned int UID, abac_user *head)
{
	abac_user *cursor = head;
	while (cursor != NULL) {
		if (cursor->uid == UID) {
			return cursor->attrs;
		}
		cursor = cursor->next;
	}
	return NULL;
}

static avp *get_obj_avp(char *path, abac_obj *head)
{
	abac_obj *cursor = head;
	while (cursor != NULL) {
		if (strcmp(path, cursor->path) == 0) {
			return cursor->attrs;
		}
		cursor = cursor->next;
	}
	return NULL;
}

static bool compare_avp_list(avp *r_avp, avp *a_avp)
{
	avp *r_cursor = r_avp;
	avp *a_cursor = a_avp;
	bool match_found;
	while (r_cursor != NULL) {
		a_cursor = a_avp;
		match_found = false;
		while (a_cursor != NULL) {
			// check if any of the avps in access request have the same attribute name
			// if a matching attribute is found, check if the value matches
			if (strcmp(r_cursor->name, a_cursor->name) == 0 &&
			    strcmp(r_cursor->value, a_cursor->value) == 0) {
				match_found = true;
				goto nextavp;
			}
			a_cursor = a_cursor->next;
		}
	nextavp:
		if (match_found) {
			r_cursor = r_cursor->next;
		} else {
			return false;
		}
	}
	return true;
}

static bool compare_op(enum abac_op r, enum abac_op a)
{
	// compare operations for rule (r) and access request (a)
	// if rule says MODIFY, then user can both READ and MODIFY
	// if rule says READ, then user can ONLY READ
	//printk("rule: %s access: %s", abac_op_str(r), abac_op_str(a));
	if (r == a || (r == ABAC_MODIFY && a == ABAC_READ)) {
		return true;
	}
	return false;
}

bool abac_resolve(unsigned int UID, char *path, int mask)
{
	enum abac_op op = convert_to_abac_op(mask);
	//printk(KERN_INFO "[ABAC LSM] User %d requested %s for file %s", UID,path, abac_op_str(op));
	if (op == ABAC_IGNORE) {
		return true;
	}
	//printk("Operation is recognized");
	avp *uattr = get_user_avp(UID, user_attr);
	if (!uattr) {
		return false;
	}
	//printk("User attrs found");
	avp *oattr = get_obj_avp(path, obj_attr);
	if (!oattr) {
		// If an object doesn't have attributes, it isn't covered by the policy
		return true;
	}
	//printk("Object attrs found");
	abac_rule *r = policy->rules;
	while (r != NULL) {
		//printk("Checking rule: %d", r->id);
		// check if access mode is matching
		if (!compare_op(r->op, op)) {
			//printk("OP not matching");
			goto next_rule;
		}
		// check if user attributes are matching
		if (!compare_avp_list(r->user, uattr)) {
			//printk("User attr not matching");
			goto next_rule;
		}
		// check if object attributes are matching
		if (!compare_avp_list(r->object, oattr)) {
			//printk("Object attr not matching");
			goto next_rule;
		}
		// check if there are any environment attribute pairs
		// both in the rule and kernel
		if (r->env && env_attr) {
			if (!compare_avp_list(r->env, env_attr)) {
				//printk("Environment attr not matching");
				goto next_rule;
			}
		}
		return true;
	next_rule:
		r = r->next;
	}
	//printk("No rules matching");
	return false;
}
