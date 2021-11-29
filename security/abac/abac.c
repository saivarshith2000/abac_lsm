/*
	ABAC LSM hooks
	Copyright (C) <2021>  Hariyala Omkara Naga Sai Varshith
*/

#include <linux/limits.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/binfmts.h>
#include <linux/xattr.h>
#include <linux/kernel_read_file.h>
#include <linux/lsm_hooks.h>
#include <linux/dcache.h>
#include <linux/cred.h>
#include "abacfs.h"
#include "resolve.h"

static const char *secured_dir = "/home/secured/";

// Check if path is secured
char *is_secured(char *secured_path, char *accessed_path)
{
	if (strncmp(secured_path, accessed_path, strlen(secured_dir)) == 0) {
		return true;
	}
	return false;
}

// get full filename
char *get_full_name(struct file *file, char *buf, int buflen)
{
	struct dentry *dentry = file->f_path.dentry;
	char *ret = dentry_path_raw(dentry, buf, buflen);
	return ret;
}

// File read/write hook
static int abac_file_permission(struct file *file, int mask)
{
	// if the user is root, don't evaluate
	unsigned int UID = current_uid().val;
	if (UID < 1000) {
		return 0;
	}
	char *path = NULL;
	struct dentry *dentry = file->f_path.dentry;
	char *buff = kmalloc(PATH_MAX, GFP_KERNEL);
	path = dentry_path_raw(dentry, buff, PATH_MAX);
	// if the path is not secured, don't evaluate
	if (!is_secured(secured_dir, path)) {
		return 0;
	}
	// if the policy is not yet initalized, DENY permission
	if (policy == NULL || user_attr == NULL || obj_attr == NULL) {
		return -EPERM;
	}
	if (abac_resolve(UID, path, mask)) {
		return 0;
	}
	return -EPERM;
}

// The hooks we wish to be installed.
static struct security_hook_list abac_hooks[] __lsm_ro_after_init = {
	LSM_HOOK_INIT(file_permission, abac_file_permission),
};

// Initialize our module.
static int __init abac_init(void)
{
	security_add_hooks(abac_hooks, ARRAY_SIZE(abac_hooks), "abac");
	printk(KERN_INFO
	       "ABAC LSM: Initialized.\n Files in %s are protected by ABAC policy defined in rules.\n",
	       secured_dir);
	return 0;
}

DEFINE_LSM(abac) = {
	.init = abac_init,
	.name = "abac",
};
