/*
	ABAC AVP parsing methods
	Copyright (C) <2021>  Hariyala Omkara Naga Sai Varshith
*/

#include <linux/string.h>
#include <linux/slab.h>
#include "avp.h"

void destroy_avp_list(avp *head) 
{
	// Free the avp linked list given by head
	avp* cursor = head;
	avp* to_free = NULL;
	while (cursor != NULL) {
		kfree(cursor->name);
		kfree(cursor->value);
		to_free = cursor;
		cursor = cursor->next;
		if (to_free) kfree(to_free);
	}
}

int findidx(char *buffer, char ch, int start, int end)
{
	// Find the index of first occurance of ch in buffer between start and end (excluding end)
	int i = start;
	for (i = start; i < end; i++) {
		if (buffer[i] == ch)
			return i;
	}
	return -1;
}

avp *parse_avp(char *buffer, int start, int end)
{
	// Parse a single attribute-value pair separated by =
	// Example: Designation=Professor
	int i;
	int pair_delim = findidx(buffer, '=', start, end);
	char *name = kcalloc(pair_delim - start, sizeof(char), GFP_KERNEL);
	int j = 0;
	for (i = start; i < pair_delim; i++) {
		name[j++] = buffer[i];
	}
	name[j] = '\0';
	char *value = kcalloc(end - pair_delim, sizeof(char), GFP_KERNEL);
	j = 0;
	for (i = pair_delim + 1; i < end; i++) {
		value[j++] = buffer[i];
	}
	value[j] = '\0';
	avp *p = kcalloc(1, sizeof(avp), GFP_KERNEL);
	p->name = name;
	p->value = value;
	p->next = NULL;
	return p;
}

avp *parse_avp_section(char *buffer, int start, int end)
{
	// Parse the string between | delimiters
	// Example: Designation=Professor,Department=CSE
	int _start = start;
	int delim = findidx(buffer, ',', _start, end);
	avp *cursor, *head;
	cursor = NULL;
	head = NULL;
	while (delim != -1 && _start < end) {
		if (cursor) {
			cursor->next = parse_avp(buffer, _start, delim);
			cursor = cursor->next;
		} else {
			head = parse_avp(buffer, _start, delim);
			cursor = head;
		}
		_start = delim + 1;
		delim = findidx(buffer, ',', _start, end);
	}
	// if there was more than one avp, then there will be one more avp between the last , and |
	if (cursor) {
		cursor->next = parse_avp(buffer, _start, end);
	} else {
		head = parse_avp(buffer, _start, end);
		cursor = head;
	}
	return head;
}
