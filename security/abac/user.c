/*
	ABAC user attribute parsing methods
	Copyright (C) <2021>  Hariyala Omkara Naga Sai Varshith
*/

#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include "user.h"

void destroy_user_list(abac_user *head) {
	abac_user *cursor = head;
	abac_user *to_free = NULL;
	while (cursor != NULL) {
		destroy_avp_list(cursor->attrs);
		to_free = cursor;
		cursor = cursor->next;
		if (to_free)kfree(to_free);
	}
}

static abac_user *parse_line(char *buffer, int start, int end)
{
	/* Parse a single line of the buffer */
	int i, uid_len;
	unsigned int uid;
	abac_user *u = kcalloc(1, sizeof(abac_user), GFP_KERNEL);
	int delim = findidx(buffer, ':', start, end);
	uid_len = delim - start + 1;
	char *uidbuf = kcalloc(delim - start + 1, sizeof(char), GFP_KERNEL);
	for (i = 0; i < uid_len; i++) {
		uidbuf[i] = buffer[start + i];
	}
	uidbuf[delim - start] = '\0';
	if (kstrtouint(uidbuf, 10, &uid) != 0) {
		printk("Invalid uidbuf %s\n", uidbuf);
		kfree(u);
		return NULL;
	}
	u->uid = uid;
	u->attrs = parse_avp_section(buffer, delim + 1, end);
	u->next = NULL;
	return u;
}

abac_user *parse_user_attr(char *buffer, int length)
{
	/*
	 * Parse user attributes buffer 
	 * Buffer format 
	 * <user-id1>:<attr-name1>=<attr-value1>,<attr-name2>=<attr-value2> 
	 * <user-id2>:<attr-name3>=<attr-value3>,<attr-name4>=<attr-value4> 
	 */
	if (buffer == NULL || length < 1) {
		return NULL;
	}
	int start = 0;
	int end = 0;
	int i;
	abac_user *cursor, *head;
	cursor = NULL;
	head = NULL;
	for (i = 0; i < length; i++) {
		if (buffer[i] == '\n') {
			end = i;
			if (cursor) {
				cursor->next = parse_line(buffer, start, end);
				cursor = cursor->next;
			} else {
				head = parse_line(buffer, start, end);
				cursor = head;
			}
			start = i + 1;
		}
	}
	return head;
}

void print_user_attrs(abac_user *u)
{
	abac_user *cursor;
	avp *avp_cursor;
	cursor = u;
	while (cursor != NULL) {
		printk("%d", cursor->uid);
		avp_cursor = cursor->attrs;
		while (avp_cursor != NULL) {
			printk("%s=%s", avp_cursor->name, avp_cursor->value);
			avp_cursor = avp_cursor->next;
		}
		cursor = cursor->next;
		printk("\n");
	}
	printk("\n");
}
