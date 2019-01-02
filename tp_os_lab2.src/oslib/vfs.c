#include <stdlib.h>
#include <string.h>
#include <vfs.h>
#include <fat.h>

/***************************************************************************
 * File name manipulation utilities
 ***************************************************************************/
// -------------------- dirname ------------------------------------------
// returns parent's name (everything before the last '/')
char *dirname(char *path)
{
	char *dname = strdup(path);
	char *slash = NULL;
	char *p = dname;

	while (*p)
	{
		if (*p == '/')
			slash = p;
		p++;
	}

	if (slash)
		*slash = 0;
	if (*dname == 0)
		strcpy(dname, "/");
	return dname;
}

// -------------------- basename ------------------------------------------
// returns the filename (everything after the last '/')
char *basename(char *path)
{
	char *p = path;
	char *slash = NULL;
	while (*p)
	{
		if (*p == '/')
			slash = p;
		p++;
	}
	if (slash)
		return slash + 1;
	else
		return path;
}

/***************************************************************************
 * Opened file descriptor array with Device associated to file descriptor
 ***************************************************************************/
FileObject *opened_fds[MAX_OPENED_FDS];

/***************************************************************************
 * Registered device table
 ***************************************************************************/
extern Device *device_table[];
extern Device dev_fs;
extern Device dev_test;

/* dev_lookup
 *   search for a device represented by its name in the device table
 */
static Device *dev_lookup(char *path)
{
	char *dir = dirname(path);

	if (!strcmp(dir, "/dev"))
	{
		free(dir);
		int i = 0;
		char *devname = basename(path);

		Device *dev = device_table[0];
		while (dev)
		{
			if (!strcmp(devname, dev->name))
				return dev;
			dev = device_table[++i];
		}
	}
	else
		free(dir);
	return NULL;
}

/***************************************************************************
 * FAT Object
 ***************************************************************************/

int mount()
{
	return dev_fs.init(&dev_fs);
}

/***************************************************************************
 * Generic device functions
 ***************************************************************************/

/* open
 *   returns a file descriptor for path name
 */
int open(char *path, int flags)
{
	// Recherche de l'index disponnible dans device table
	int i = 0;
	while (1)
	{
		if (opened_fds[i] == NULL)
			break;

		i++;
		if (MAX_OPENED_FDS == i)
			return -1;
	}

	// Allocation et initialisation du FileObject
	FileObject *fo = malloc(sizeof(FileObject));
	if (fo == NULL)
		return -1;

	fo->name = basename(path);
	fo->flags = flags;
	fo->offset = 0;

	fo->file = NULL;
	fo->dir = NULL;

	if (strcmp(path, "/dev") == 0)
	{
		fo->flags |= F_IS_DEVDIR;
		opened_fds[i] = fo;
		return i;
	}

	fo->dev = dev_lookup(path);
	if (fo->dev == NULL)
	{
		fo->name = path;
		fo->dev = &dev_fs;
	}

	if (fo->dev->open != NULL)
		if (fo->dev->open(fo) == 0)
		{
			free(fo);
			return -1;
		}

	opened_fds[i] = fo;

	return i;
}

/* close
 *   close the file descriptor
 */
int close(int fd)
{
	FileObject *fo = opened_fds[fd];
	if (fo->dev->close(fo) == 0)
		return -1;

	free(fo);

	opened_fds[fd] = NULL;

	return 0;
}

/* read
 *   read len bytes from fd to buf, returns actually read bytes
 */
int read(int fd, void *buf, size_t len)
{
	if (opened_fds[fd]->dev->read == NULL)
		return -1;

	return opened_fds[fd]->dev->read(opened_fds[fd], buf, len);
}

/* write
 *   write len bytes from buf to fd, returns actually written bytes
 */
int write(int fd, void *buf, size_t len)
{
	if (opened_fds[fd]->dev->write == NULL)
		return -1;

	return opened_fds[fd]->dev->write(opened_fds[fd], buf, len);
}

/* ioctl
 *   set/get parameter for fd
 */
int ioctl(int fd, int op, void **data)
{
	if (opened_fds[fd]->dev->ioctl == NULL)
		return -1;

	return opened_fds[fd]->dev->ioctl(opened_fds[fd], op, data);
}

/* lseek
 *   set the offset in fd
 */
int lseek(int fd, unsigned int offset)
{
	FileObject *f = opened_fds[fd];
	if (f)
	{
		f->offset = offset;
		return 0;
	}
	return -1;
}

/***************************************************************************
 * Directory handling functions
 ***************************************************************************/
int dev_fs_next_dir(DIR *dir);

int readdir(int fd, DIR **dir)
{
	FileObject *f = opened_fds[fd];

	if (f)
	{
		if (f->flags & F_IS_ROOTDIR)
		{
			f->flags &= ~F_IS_ROOTDIR;
			strcpy(f->dir->entry, "dev");
			f->dir->entry_isdir = 1;
			f->dir->entry_size = 0;
			*dir = f->dir;
			return 0;
		}
		else if (f->flags & F_IS_DEVDIR)
		{
			if (device_table[f->offset])
			{
				strcpy(f->dir->entry, device_table[f->offset]->name);
				f->dir->entry_isdir = 0;
				f->dir->entry_size = 0;
				*dir = f->dir;
				f->offset++;
				return 0;
			}
		}
		else if ((f->flags & F_IS_DIR) && dev_fs_next_dir(f->dir))
		{
			*dir = f->dir;
			return 0;
		}
	}
	return -1;
}
