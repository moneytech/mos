#include "char_dev.h"

#include <include/errno.h>

struct list_head devlist;

struct char_device *get_chrdev(dev_t dev)
{
	struct char_device *iter = NULL;
	list_for_each_entry(iter, &devlist, sibling)
	{
		if (iter->dev == dev)
			return iter;
	};
	return NULL;
}

int register_chrdev(struct char_device *new_cdev)
{
	struct char_device *exist = get_chrdev(new_cdev->dev);
	if (exist == NULL)
	{
		list_add_tail(&new_cdev->sibling, &devlist);
		return 0;
	}
	else
		return -EEXIST;
}

int unregister_chrdev(dev_t dev)
{
	struct char_device *cdev = get_chrdev(dev);
	if (cdev)
	{
		list_del(&cdev->sibling);
		return 0;
	}
	else
		return -ENODEV;
}

int chrdev_open(struct vfs_inode *inode, struct vfs_file *filp)
{
	struct char_device *cdev = get_chrdev(inode->i_rdev);
	if (cdev == NULL)
		return -ENODEV;

	if (cdev->f_ops->open)
		return cdev->f_ops->open(inode, filp);

	return -EINVAL;
}

ssize_t chrdev_read(struct vfs_file *file, char *buf, size_t count, loff_t ppos)
{
	struct char_device *cdev = get_chrdev(file->f_dentry->d_inode->i_rdev);
	if (cdev == NULL)
		return -ENODEV;

	if (cdev->f_ops->read)
		return cdev->f_ops->read(file, buf, count, ppos);

	return -EINVAL;
}

unsigned int chrdev_poll(struct vfs_file *file, struct poll_table *pt)
{
	struct char_device *cdev = get_chrdev(file->f_dentry->d_inode->i_rdev);
	if (cdev == NULL)
		return -ENODEV;

	if (cdev->f_ops->poll)
		return cdev->f_ops->poll(file, pt);

	return -EINVAL;
}

int chrdev_release(struct vfs_inode *inode, struct vfs_file *file)
{
	struct char_device *cdev = get_chrdev(inode->i_rdev);
	if (cdev == NULL)
		return -ENODEV;

	if (cdev->f_ops->release)
		return cdev->f_ops->release(inode, file);

	return -EINVAL;
}

struct vfs_file_operations def_chr_fops = {
	.open = chrdev_open,
	.read = chrdev_read,
	.poll = chrdev_poll,
	.release = chrdev_release,
};

void chrdev_init()
{
	INIT_LIST_HEAD(&devlist);
}
