#ifndef FS_VFS_H
#define FS_VFS_H

#include <stdint.h>
#include <stddef.h>
#include <include/ctype.h>
#include <include/list.h>
#include <kernel/locking/semaphore.h>

// file
#define S_IFMT 00170000
#define S_IFIFO 0010000
#define S_IFSOCK 0140000
#define S_IFLNK 0120000
#define S_IFREG 0100000
#define S_IFBLK 0060000
#define S_IFDIR 0040000
#define S_IFCHR 0020000

#define S_ISLNK(m) (((m)&S_IFMT) == S_IFLNK)
#define S_ISREG(m) (((m)&S_IFMT) == S_IFREG)
#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)
#define S_ISCHR(m) (((m)&S_IFMT) == S_IFCHR)
#define S_ISBLK(m) (((m)&S_IFMT) == S_IFBLK)
#define S_ISFIFO(m) (((m)&S_IFMT) == S_IFIFO)
#define S_ISSOCK(m) (((m)&S_IFMT) == S_IFSOCK)

#define MAX_SUB_DENTRIES 8

typedef struct kstat
{
  unsigned long ino;
  dev_t dev;
  umode_t mode;
  unsigned int nlink;
  uid_t uid;
  gid_t gid;
  dev_t rdev;
  loff_t size;
  struct timespec atime;
  struct timespec mtime;
  struct timespec ctime;
  unsigned long blksize;
  unsigned long blocks;
} kstat;

typedef struct iattr
{
  unsigned int ia_valid;
  umode_t ia_mode;
  uid_t ia_uid;
  gid_t ia_gid;
  loff_t ia_size;
  struct timespec ia_atime;
  struct timespec ia_mtime;
  struct timespec ia_ctime;
  unsigned int ia_attr_flags;
} iattr;

typedef struct vfs_file_system_type
{
  const char *name;
  struct vfs_superblock *(*mount)(struct vfs_file_system_type *, char *, char *);
  void (*unmount)(struct vfs_superblock *);
  struct vfs_file_system_type *next;
} vfs_file_system_type;

typedef struct vfs_mount
{
  struct vfs_dentry *mnt_mountpoint;
  struct vfs_dentry *mnt_root;
  struct vfs_superblock *mnt_sb;
  char *mnt_devname;
  struct vfs_mount *next;
} vfs_mount;

typedef struct vfs_superblock
{
  unsigned long s_blocksize;
  dev_t s_dev;
  unsigned char s_blocksize_bits;
  struct vfs_file_system_type *s_type;
  struct vfs_super_operations *s_op;
  unsigned long s_magic;
  struct vfs_dentry *s_root;
  char *mnt_devname;
  void *s_fs_info;
} vfs_superblock;

typedef struct vfs_super_operations
{
  struct vfs_inode *(*alloc_inode)(struct vfs_superblock *sb);
  void (*read_inode)(struct vfs_inode *);
  void (*write_inode)(struct vfs_inode *);
  void (*write_super)(struct vfs_superblock *);
} vfs_super_operations;

typedef struct vfs_inode
{
  unsigned long i_ino;
  umode_t i_mode;
  unsigned int i_nlink;
  uid_t i_uid;
  gid_t i_gid;
  dev_t i_rdev;
  struct timespec i_atime;
  struct timespec i_mtime;
  struct timespec i_ctime;
  uint32_t i_blocks;
  unsigned long i_blksize;
  uint32_t i_flags;
  uint32_t i_size;
  struct semaphore i_sem;
  struct pipe *i_pipe;
  struct vfs_inode_operations *i_op;
  struct vfs_file_operations *i_fop;
  struct vfs_superblock *i_sb;
  void *i_fs_info;
} vfs_inode;

typedef struct vfs_inode_operations
{
  struct vfs_inode *(*create)(struct vfs_inode *, char *, mode_t mode);
  struct vfs_inode *(*lookup)(struct vfs_inode *, char *);
  int (*mknod)(struct vfs_inode *, char *, int, dev_t);
  void (*truncate)(struct vfs_inode *);
  int (*setattr)(struct vfs_dentry *, struct iattr *);
  int (*getattr)(struct vfs_mount *mnt, struct vfs_dentry *, struct kstat *);
} vfs_inode_operations;

typedef struct vfs_dentry
{
  struct vfs_inode *d_inode;
  struct vfs_dentry *d_parent;
  char *d_name;
  struct vfs_superblock *d_sb;
  struct list_head d_subdirs;
  struct list_head d_sibling;
} vfs_dentry;

typedef struct vfs_file
{
  struct vfs_dentry *f_dentry;
  struct vfs_mount *f_vfsmnt;
  struct vfs_file_operations *f_op;
  long f_count;
  unsigned int f_flags;
  mode_t f_mode;
  loff_t f_pos;
} vfs_file;

typedef struct vfs_file_operations
{
  loff_t (*llseek)(struct vfs_file *file, loff_t ppos);
  ssize_t (*read)(struct vfs_file *file, char *buf, size_t count, loff_t ppos);
  ssize_t (*write)(struct vfs_file *file, const char *buf, size_t count, loff_t ppos);
  int (*open)(struct vfs_inode *, struct vfs_file *);
  int (*release)(struct vfs_inode *, struct vfs_file *);
} vfs_file_operations;

typedef struct nameidata
{
  struct vfs_dentry *dentry;
  struct vfs_mount *mnt;
} nameidata;

int register_filesystem(vfs_file_system_type *fs);
int unregister_filesystem(vfs_file_system_type *fs);
int find_unused_fd_slot();
vfs_mount *lookup_mnt(vfs_dentry *d);
void vfs_init(vfs_file_system_type *fs, char *dev_name);
vfs_inode *init_inode();
void init_special_inode(vfs_inode *inode, umode_t mode, dev_t dev);

// open.c
vfs_dentry *alloc_dentry(vfs_dentry *parent, char *name);
long vfs_open(const char *path);
long vfs_close(uint32_t fd);
void vfs_stat(const char *path, kstat *stat);
void vfs_fstat(uint32_t fd, kstat *stat);
int vfs_mknod(const char *path, int mode, dev_t dev);

// read_write.c
char *vfs_read(const char *path);
ssize_t vfs_fread(uint32_t fd, char *buf, size_t count);
int vfs_write(const char *path, const char *buf, size_t count);
ssize_t vfs_fwrite(uint32_t fd, const char *buf, size_t count);
loff_t vfs_flseek(uint32_t fd, loff_t offset);

#endif