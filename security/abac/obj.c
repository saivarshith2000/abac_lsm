/*
	ABAC object attribute parsing methods
	Copyright (C) <2021>  Hariyala Omkara Naga Sai Varshith
*/

#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include "obj.h"

void destroy_obj_list(abac_obj *head) {
	abac_obj *cursor = head;
	abac_obj *to_free = NULL;
	while (cursor != NULL) {
		destroy_avp_list(cursor->attrs);
		to_free = cursor;
		cursor = cursor->next;
		if (to_free)kfree(to_free);
	}
}

static abac_obj *parse_line(char *buffer, int start, int end)
{
	/* Parse a single line of the buffer */
	int i;
	int delim = findidx(buffer, ':', start, end);
	if (delim - start >= PATH_MAX) {
		printk("buffer: %s, %d", buffer, delim);
		printk(KERN_ERR "File path exceeds PATH_MAX limit");
		return NULL;
	}
	abac_obj *o = kcalloc(1, sizeof(abac_obj), GFP_KERNEL);
	strncpy(o->path, buffer + start, delim - start);
	o->path[delim - start + 1] = '\0';
	o->attrs = parse_avp_section(buffer, delim + 1, end);
	o->next = NULL;
	return o;
}

abac_obj *parse_obj_attr(char *buffer, int length)
{
	/*
	 * Parse Object attributes buffer 
	 * Buffer format (For now all objects are files) 
	 * <object-path1>:<attr-name1>=<attr-value1>,<attr-name2>=<attr-value2> 
	 * <object-path2>:<attr-name3>=<attr-value3>,<attr-name4>=<attr-value4> 
	 */
	if (buffer == NULL || length < 1) {
		return NULL;
	}
	int start = 0;
	int end = 0;
	int i;
	abac_obj *cursor, *head;
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

void print_obj_attrs(abac_obj *u)
{
	abac_obj *cursor;
	avp *avp_cursor;
	cursor = u;
	while (cursor != NULL) {
		printk("%s", cursor->path);
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
