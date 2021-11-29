/*
	ABAC environmental attribute parsing methods
	Copyright (C) <2021>  Hariyala Omkara Naga Sai Varshith
*/

#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include "env.h"

avp *parse_env_attr(char *buffer, int length)
{
	/*
     * Parse environment attributes buffer
     * Buffer format
     * <env_attr_1>=<val1>
     * <env_attr_2>=<val1>
     */
	if (buffer == NULL || length < 1) {
		return NULL;
	}
	int i, end, start;
	start = 0;
	avp *head, *cursor;
	head = NULL;
	cursor = NULL;
	for (i = 0; i < length; i++) {
		if (buffer[i] == '\n') {
			end = i;
			if (cursor) {
				cursor->next = parse_avp(buffer, start, end);
				cursor = cursor->next;
			} else {
				head = parse_avp(buffer, start, end);
				cursor = head;
			}
			start = i + 1;
		}
	}
	return head;
}

void print_env_attrs(avp *head)
{
	if (head == NULL) {
		return;
	}
	avp *cursor;
	cursor = head;
	printk("Environment Attributes");
	while (cursor != NULL) {
		printk("%s=%s", cursor->name, cursor->value);
		cursor = cursor->next;
	}
}
