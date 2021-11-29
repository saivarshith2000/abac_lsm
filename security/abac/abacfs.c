/*
	ABAC securityFS system calls
	Copyright (C) <2021>  Hariyala Omkara Naga Sai Varshith
*/

#include "abacfs.h"
#include <linux/init.h>
#include <linux/security.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

// const size_t MAX_FILE_SIZE = 65536; // 64Kb
const size_t MAX_FILE_SIZE = 16777216; // 16MB

struct dentry *abacfs;
struct dentry *policy_file;
struct dentry *user_attr_file;
struct dentry *obj_attr_file;
struct dentry *env_attr_file;

char *policy_buf = NULL;
char *user_attr_buf = NULL;
char *obj_attr_buf = NULL;
char *env_attr_buf = NULL;

abac_policy *policy = NULL;
abac_user *user_attr = NULL;
abac_obj *obj_attr = NULL;
avp *env_attr = NULL;

// method for opening policy file
static int abac_open(struct inode *i, struct file *f)
{
	// TODO: Add a check to let only the root access the policy file
	return 0;
}

// method for writing to policy file
static ssize_t policy_write(struct file *filp, const char __user *buffer,
			    size_t len, loff_t *off)
{
	if (len >= MAX_FILE_SIZE) {
		printk(KERN_INFO
		       "Write failed. Buffer too large %zu. Maximum Policy file size is %zu\n",
		       len, MAX_FILE_SIZE);
		return -EFAULT;
	}
	if (policy_buf) {
		destroy_policy(policy);
		kfree(policy_buf);
	}
	policy_buf = kmalloc(len, GFP_KERNEL);
	if (!policy_buf) {
		printk(KERN_INFO
		       "Write failed. Failed to allocate memory for policy buffer\n");
		return -EFAULT;
	}
	if (copy_from_user(policy_buf, buffer, len + 1)) {
		printk(KERN_INFO "Write to policy failed\n");
		return -EFAULT;
	}
	policy_buf[len] = '\0';
	printk("Policy written to buffer. Attempting to parse...");
	policy = parse_policy(policy_buf, len);
	// print_abac_policy(policy);
	printk("Policy loaded");
	return len;
}

// method for reading from policy file
static ssize_t policy_read(struct file *filp, char __user *buffer, size_t len,
			   loff_t *off)
{
	if (!policy_buf) {
		return -EFAULT;
	}
	/*
static int finished = 0;
if (finished) {
    finished = 0;
    return 0;
}
finished = 1;
*/

	int policy_buf_len = strlen(policy_buf);
	if (copy_to_user(buffer, policy_buf, policy_buf_len) != 0) {
		printk(KERN_INFO "Read from policy failed\n");
		return -EFAULT;
	}
	// Return the number of bytes read
	return policy_buf_len;
}

// method for writing to user_attrs file
static ssize_t user_attr_write(struct file *filp, const char __user *buffer,
			       size_t len, loff_t *off)
{
	if (len >= MAX_FILE_SIZE) {
		printk(KERN_INFO
		       "Write failed. Buffer too large %zu. Maximum file size is %zu\n",
		       len, MAX_FILE_SIZE);
		return -EFAULT;
	}
	if (user_attr_buf) {
		destroy_user_list(user_attr);
		kfree(user_attr_buf);
	}
	user_attr_buf = kmalloc(len, GFP_KERNEL);
	if (!user_attr_buf) {
		printk(KERN_INFO
		       "Write failed. Failed to allocate memory for user attributes buffer\n");
		return -EFAULT;
	}
	if (copy_from_user(user_attr_buf, buffer, len + 1)) {
		printk(KERN_INFO "Write to user_attrs failed\n");
		return -EFAULT;
	}
	user_attr_buf[len] = '\0';
	printk("User attributes written to buffer. Attempting to parse...");
	user_attr = parse_user_attr(user_attr_buf, len);
	//print_user_attrs(user_attr);
	printk("User attributes loaded");
	return len;
}

// method for writing to obj_attrs file
static ssize_t obj_attr_write(struct file *filp, const char __user *buffer,
			      size_t len, loff_t *off)
{
	if (len >= MAX_FILE_SIZE) {
		printk(KERN_INFO
		       "Write failed. Buffer too large %zu. Maximum file size is %zu\n",
		       len, MAX_FILE_SIZE);
		return -EFAULT;
	}
	if (obj_attr_buf) {
		destroy_obj_list(obj_attr);
		kfree(obj_attr_buf);
	}
	obj_attr_buf = kmalloc(len, GFP_KERNEL);
	if (!obj_attr_buf) {
		printk(KERN_INFO
		       "Write failed. Failed to allocate memory for object "
		       "attributes buffer\n");
		return -EFAULT;
	}
	if (copy_from_user(obj_attr_buf, buffer, len + 1)) {
		printk(KERN_INFO "Write to obj_attrs failed\n");
		return -EFAULT;
	}
	obj_attr_buf[len] = '\0';
	printk("Object attributes written to buffer. Attempting to parse...");
	obj_attr = parse_obj_attr(obj_attr_buf, len);
	//print_obj_attrs(obj_attr);
	printk("Object attributes loaded");
	return len;
}

// method for writing to env_attrs file
static ssize_t env_attr_write(struct file *filp, const char __user *buffer,
			      size_t len, loff_t *off)
{
	if (len >= MAX_FILE_SIZE) {
		printk(KERN_INFO
		       "Write failed. Buffer too large %zu. Maximum file size is %zu\n",
		       len, MAX_FILE_SIZE);
		return -EFAULT;
	}
	if (env_attr_buf) {
		destroy_avp_list(env_attr);
		kfree(env_attr_buf);
	}
	env_attr_buf = kmalloc(len, GFP_KERNEL);
	if (!env_attr_buf) {
		printk(KERN_INFO
		       "Write failed. Failed to allocate memory for environment "
		       "attributes buffer\n");
		return -EFAULT;
	}
	if (copy_from_user(env_attr_buf, buffer, len + 1)) {
		printk(KERN_INFO "Write to env_attrs failed\n");
		return -EFAULT;
	}
	env_attr_buf[len] = '\0';
	printk("Environment attributes written to buffer. Attempting to parse...");
	env_attr = parse_env_attr(env_attr_buf, len);
	//print_env_attrs(env_attr);
	printk("Environment attributes loaded");
	return len;
}

static const struct file_operations policy_fops = {
	.open = abac_open,
	.read = policy_read,
	.write = policy_write,
};

static const struct file_operations user_attr_fops = {
	.open = abac_open,
	.write = user_attr_write,
};

static const struct file_operations obj_attr_fops = {
	.open = abac_open,
	.write = obj_attr_write,
};

static const struct file_operations env_attr_fops = {
	.open = abac_open,
	.write = env_attr_write,
};

static void destroy_abac_fs(void)
{
	if (policy_file) {
		securityfs_remove(policy_file);
	}
	if (user_attr_file) {
		securityfs_remove(user_attr_file);
	}
	if (obj_attr_file) {
		securityfs_remove(obj_attr_file);
	}
	if (env_attr_file) {
		securityfs_remove(env_attr_file);
	}
	if (abacfs) {
		securityfs_remove(abacfs);
	}
}

/* create the abac filesystem */
static void abac_create_fs(void)
{
	// create the root 'abac' directory
	abacfs = securityfs_create_dir("abac", NULL);
	if (!abacfs) {
		printk(KERN_ERR "ABAC LSM: Failed to create abac securityfs "
				"/sys/kernel/security/abac/");
		destroy_abac_fs();
	}

	// create the policy file
	policy_file = securityfs_create_file("policy", 0666, abacfs, NULL,
					     &policy_fops);
	if (!policy_file) {
		printk(KERN_ERR
		       "ABAC LSM: Failed to create file /sys/kernel/security/abac/policy");
		destroy_abac_fs();
		return;
	}
	printk(KERN_INFO
	       "ABAC LSM: Created file /sys/kernel/security/abac/policy");

	// create the user attributes file
	user_attr_file = securityfs_create_file("user_attr", 0666, abacfs, NULL,
						&user_attr_fops);
	if (!user_attr_file) {
		printk(KERN_ERR
		       "ABAC LSM: Failed to create file /sys/kernel/security/abac/user_attr");
		destroy_abac_fs();
		return;
	}
	printk(KERN_INFO
	       "ABAC LSM: Created file /sys/kernel/security/abac/user_attr");

	// create the object attributes file
	obj_attr_file = securityfs_create_file("obj_attr", 0666, abacfs, NULL,
					       &obj_attr_fops);
	if (!obj_attr_file) {
		printk(KERN_ERR
		       "ABAC LSM: Failed to create file /sys/kernel/security/abac/obj_attr");
		destroy_abac_fs();
		return;
	}
	printk(KERN_INFO
	       "ABAC LSM: Created file /sys/kernel/security/abac/obj_attr");

	// create the environment attributes file
	env_attr_file = securityfs_create_file("env_attr", 0666, abacfs, NULL,
					       &env_attr_fops);
	if (!env_attr_file) {
		printk(KERN_ERR
		       "ABAC LSM: Failed to create file /sys/kernel/security/abac/env_attr");
		destroy_abac_fs();
		return;
	}
	printk(KERN_INFO
	       "ABAC LSM: Created file /sys/kernel/security/abac/env_attr");

	printk(KERN_INFO "ABAC LSM: Securityfs Initialized");
}

fs_initcall(abac_create_fs);
