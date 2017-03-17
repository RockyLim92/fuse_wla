#include "params.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

#include "log.h"

/***************************************************************
fuse filesystem mount to the root directory, 

to change this root directory, make a full path,

2016, Rocky, Ajou University, lrocky1229@gmail.com
***************************************************************/
static void wla_fullpath(char fpath[PATH_MAX], const char *path)
{
    strcpy(fpath, WLA_DATA->rootdir);
    strncat(fpath, path, PATH_MAX);
}

/** Get file attributes.*/
int wla_getattr(const char *path, struct stat *statbuf)
{
    int retstat;
    char fpath[PATH_MAX];
    wla_fullpath(fpath, path);

    retstat = lstat(fpath, statbuf);

    if(retstat < 0)
        retstat = -errno;


    
    return retstat;
}

/** Read the target of a symbolic link. */
int wla_readlink(const char *path, char *link, size_t size)
{
    int retstat;
    char fpath[PATH_MAX];
    wla_fullpath(fpath, path);

    retstat = readlink(fpath, link, size - 1);
    if (retstat >= 0) {
	   link[retstat] = '\0';
	   retstat = 0;
    }
    
    return retstat;
}

/** Create a file node*/
int wla_mknod(const char *path, mode_t mode, dev_t dev)
{
    int retstat;
    char fpath[PATH_MAX];
    wla_fullpath(fpath, path);
  
    if(S_ISREG(mode)) {
	   retstat = open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);
	   if(retstat >= 0)
	       retstat = close(retstat);
    }
    else if(S_ISFIFO(mode))
	   retstat = mkfifo(fpath, mode);
	else
	   retstat = mknod(fpath, mode, dev);
    
    return retstat;
}

/** Create a directory */
int wla_mkdir(const char *path, mode_t mode)
{
    char fpath[PATH_MAX];
    wla_fullpath(fpath, path);

    return mkdir(fpath, mode);
}

/** Remove a file */
int wla_unlink(const char *path)
{
    char fpath[PATH_MAX];
    wla_fullpath(fpath, path);

    return unlink(fpath);
}

/** Remove a directory */
int wla_rmdir(const char *path)
{
    char fpath[PATH_MAX];
    wla_fullpath(fpath, path);

    return rmdir(fpath);
}

/** Create a symbolic link */
int wla_symlink(const char *path, const char *link)
{
    char flink[PATH_MAX];
    wla_fullpath(flink, link);

    return symlink(path, flink);
}

/** Rename a file */
int wla_rename(const char *path, const char *newpath)
{
    char fpath[PATH_MAX];
    char fnewpath[PATH_MAX];
    wla_fullpath(fpath, path);
    wla_fullpath(fnewpath, newpath);

    return rename(fpath, fnewpath);
}

/** Create a hard link to a file */
int wla_link(const char *path, const char *newpath)
{
    char fpath[PATH_MAX], fnewpath[PATH_MAX];
    wla_fullpath(fpath, path);
    wla_fullpath(fnewpath, newpath);

    return link(fpath, fnewpath);
}

/** Change the permission bits of a file */
int wla_chmod(const char *path, mode_t mode)
{
    char fpath[PATH_MAX];
    wla_fullpath(fpath, path);

    return chmod(fpath, mode);
}

/** Change the owner and group of a file */
int wla_chown(const char *path, uid_t uid, gid_t gid)
  
{
    char fpath[PATH_MAX];
    wla_fullpath(fpath, path);

    return chown(fpath, uid, gid);
}

/** Change the size of a file */
int wla_truncate(const char *path, off_t newsize)
{
    char fpath[PATH_MAX];
    wla_fullpath(fpath, path);

    return truncate(fpath, newsize);
}

/** Change the access and/or modification times of a file */
/* note -- I'll want to change this as soon as 2.6 is in debian testing */
int wla_utime(const char *path, struct utimbuf *ubuf)
{
    char fpath[PATH_MAX];
    wla_fullpath(fpath, path);

    return utime(fpath, ubuf);
}

/** File open operation*/
int wla_open(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    int fd;
    char fpath[PATH_MAX];
    wla_fullpath(fpath, path);

    // if the open call succeeds, my retstat is the file descriptor,
    // else it's -errno.  I'm making sure that in that case the saved
    // file descriptor is exactly -1.
    fd = open(fpath, fi->flags);
    if (fd < 0)
        retstat = -errno;
    
    fi->fh = fd;

    
    return retstat;
}

/***************************************************************
The following operation is read operation

in this read operation

the workload file system trace the path, size, and offset

using these info, the workload file system calculate

the patter of workload and transaction


2016, Rocky, Ajou University, lrocky1229@gmail.com
***************************************************************/
/** Read data from an open file*/
int wla_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) 
{

    /*
    maximum 131072byte site.

    */

    int retstat = 0;



    /*
    record the log of operation if the starting point of the sequence
    i.e record the operation parameters when the offset is 0;    */
    if(offset == 0){

        readCount = readCount +1;

        log_print_time();
        log_msg("read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld)\n",
            path, buf, size, offset);
    }

    readByte = readByte + size ;
    
    //log_msg("read(size=%d, offset=%lld)\n",size, offset);
    
    return pread(fi->fh, buf, size, offset);
}
/***************************************************************
The following operation is write operation

in this write operation

the workload file system trace the path, size, and offset

using these info, the workload file system calculate

the patter of workload and transaction


2016, Rocky, Ajou University, lrocky1229@gmail.com
***************************************************************/
/** Write data to an open file*/
int wla_write(const char *path, const char *buf, size_t size, off_t offset,
	     struct fuse_file_info *fi)
{
    int retstat = 0;


    /*
    when the write operation comes from system,
    size is less than 4096byte(maximum 4096)

    2016, Rocky, Ajou University, lrocky1229@gmail.com
    */

    if(offset == 0){

        writeCount = writeCount +1;

        log_print_time();
        log_msg("write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld)\n",
            path, buf, size, offset);
    }

    writeByte = writeByte + size;

    //log_msg("write(size=%d, offset=%lld)\n",size, offset);

    return pwrite(fi->fh, buf, size, offset);

}

/** Get file system statistics*/
int wla_statfs(const char *path, struct statvfs *statv)
{
    int retstat = 0;
    char fpath[PATH_MAX];
    wla_fullpath(fpath, path);
    
    // get stats for underlying filesystem
    retstat = statvfs(fpath, statv);

    return retstat;
}

/** Possibly flush cached data*/
int wla_flush(const char *path, struct fuse_file_info *fi)
{

    /*
        

    NO OPERATION HERE


    */

    return 0;
}

/** Release an open file*/
int wla_release(const char *path, struct fuse_file_info *fi)
{
    return close(fi->fh);
}

/** Synchronize file contents*/
int wla_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{

#ifdef HAVE_FDATASYNC
    if (datasync)
	   return fdatasync(fi->fh);
    else
#endif	
	   return fsync(fi->fh);
}




/***************************************************************
The following operation is for extended attribute of file

actually, it is rerely used in the system

this file system does not implement these operation


2016, Rocky, Ajou University, lrocky1229@gmail.com
***************************************************************/
#ifdef HAVE_SYS_XATTR_H
/** Set extended attributes */
int wla_setxattr(const char *path, const char *name, const char *value, size_t size, int flags)
{
    //NO OPERATION
}

/** Get extended attributes */
int wla_getxattr(const char *path, const char *name, char *value, size_t size)
{
    //NO OPERATION
}

/** List extended attributes */
int wla_listxattr(const char *path, char *list, size_t size)
{
    //NO OPERATION
}

/** Remove extended attributes */
int wla_removexattr(const char *path, const char *name)
{
    //NO OPERATION
}
#endif


/** Open directory*/
int wla_opendir(const char *path, struct fuse_file_info *fi)
{
    DIR *dp;
    int retstat = 0;
    char fpath[PATH_MAX];
    wla_fullpath(fpath, path);

    // since opendir returns a pointer, takes some custom handling of
    // return status.
    dp = opendir(fpath);

    if (dp == NULL)
	   retstat = -errno;
    
    fi->fh = (intptr_t) dp;
    
    
    return retstat;
}

/** Read directory*/
int wla_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
	       struct fuse_file_info *fi)
{
    int retstat = 0;
    DIR *dp;
    struct dirent *de;

    // once again, no need for fullpath -- but note that I need to cast fi->fh
    dp = (DIR *) (uintptr_t) fi->fh;
    // Every directory contains at least two entries: . and ..  If my
    // first call to the system readdir() returns NULL I've got an
    // error; near as I can tell, that's the only condition under
    // which I can get an error from readdir()
    de = readdir(dp);


    if (de == 0) {
	   retstat = -errno;
    }

    // This will copy the entire directory into the buffer.  The loop exits
    // when either the system readdir() returns NULL, or filler()
    // returns something non-zero.  The first case just means I've
    // read the whole directory; the second means the buffer is full.
    do{
	   if(filler(buf, de->d_name, NULL, 0) != 0){
	       return -ENOMEM;
	   }
    }while((de = readdir(dp)) != NULL);
    
    return retstat;
}

/** Release directory*/
int wla_releasedir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    
    closedir((DIR *) (uintptr_t) fi->fh);
    
    return retstat;
}


/***************************************************************
I dont know when this operation is called 

but in general use of filesystem or file operation

is is rarely used, so I left this operation unimplemented

2016, Rocky, Ajou University, lrocky1229@gmail.com
***************************************************************/
/** Synchronize directory contents*/
int wla_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi)
{
    int retstat = 0;
    
    return retstat;
}

/**
 * Initialize filesystem
 *
 * The return value will passed in the private_data field of
 * fuse_context to all file operations and as a parameter to the
 * destroy() method.
 *
 * Introduced in version 2.3
 * Changed in version 2.6
 */
// Undocumented but extraordinarily useful fact:  the fuse_context is
// set up before this function is called, and
// fuse_get_context()->private_data returns the user_data passed to
// fuse_main().  Really seems like either it should be a third
// parameter coming in here, or else the fact should be documented
// (and this might as well return void, as it did in older versions of
// FUSE).
void *wla_init(struct fuse_conn_info *conn)
{
    //log_fuse_context(fuse_get_context());
    fuse_get_context();
    return WLA_DATA;
}

/**Clean up filesystem*/
void wla_destroy(void *userdata)
{
    log_msg("############################################################\n\r\n\r");
    log_msg("\nWLA FileSustem SUMMARY\n");

    log_msg("\n\rTotal Operation Count : %ld\n\r",readCount + writeCount);
    log_msg("Total Read Count : %ld\n\r",readCount);
    log_msg("Total Write Count : %ld\n\r",writeCount);
    log_msg("Total Transaction Byte : %lld\n\r",readByte + writeByte);
    log_msg("Total Read Byte : %lld\n\r",readByte);
    log_msg("Total Write Byte : %lld\n\r", writeByte);

    log_msg("Average Read Request Size : %f\n\r", (double)readByte/readCount);
    log_msg("Average Write Request Size : %f\n\r", (double)writeByte/writeCount );

    log_msg("ReadPercentage : %f\n\r", ((double)readByte)/(readByte+writeByte)*100);
    log_msg("\n\nWLA Filesystem Version 1.0\nAjou Univresity. HEERAK LIM(Rocky), lrocky1229@gmail.com\n\n");


    log_msg("***********************************************************************************\n");
    log_msg("readCount and writeCount do not mean just the count of read ans write function call\n");
    log_msg("becasue system request one big file operation, fuse do it by deviding several chunk\n");
    log_msg("the deviding byte size is following\n");
    log_msg("read = 131072byte (32* 4096byte)\n");
    log_msg("wrtie = 4096byte\n");
    log_msg("2016, Rocky, Ajou University, lrocky1229@gmail.com\n");
    log_msg("************************************************************************************\n");



}

/**Check file access permissions*/
int wla_access(const char *path, int mask)
{
    int retstat = 0;
    char fpath[PATH_MAX];
    wla_fullpath(fpath, path);
    
    retstat = access(fpath, mask);
    
    if (retstat < 0)
        retstat = -errno;    

    return retstat;
}

/**Change the size of an open file*/
int wla_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi)
{
    int retstat = 0;
   
    
    retstat = ftruncate(fi->fh, offset);
    if (retstat < 0)
        retstat =  -errno;  

    return retstat;
}

/**
 * Get attributes from an open file
 *
 * This method is called instead of the getattr() method if the
 * file information is available.
 *
 * Currently this is only called after the create() method if that
 * is implemented (see above).  Later it may be called for
 * invocations of fstat() too.
 *
 * Introduced in version 2.5
 */
int wla_fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi)
{
    int retstat = 0;

    // On FreeBSD, trying to do anything with the mountpoint ends up
    // opening it, and then using the FD for an fgetattr.  So in the
    // special case of a path of "/", I need to do a getattr on the
    // underlying root directory instead of doing the fgetattr().
    if (!strcmp(path, "/"))
	   return wla_getattr(path, statbuf);
    
    retstat = fstat(fi->fh, statbuf);
    if (retstat < 0)
	   retstat = -errno;
    
    return retstat;
}

struct fuse_operations wla_oper = {
  .getattr = wla_getattr,
  .readlink = wla_readlink,
  // no .getdir -- that's deprecated
  .getdir = NULL,
  .mknod = wla_mknod,
  .mkdir = wla_mkdir,
  .unlink = wla_unlink,
  .rmdir = wla_rmdir,
  .symlink = wla_symlink,
  .rename = wla_rename,
  .link = wla_link,
  .chmod = wla_chmod,
  .chown = wla_chown,
  .truncate = wla_truncate,
  .utime = wla_utime,
  .open = wla_open,
  .read = wla_read,
  .write = wla_write,
  .statfs = wla_statfs,
  .flush = wla_flush,
  .release = wla_release,
  .fsync = wla_fsync,
  
#ifdef HAVE_SYS_XATTR_H
  .setxattr = wla_setxattr,
  .getxattr = wla_getxattr,
  .listxattr = wla_listxattr,
  .removexattr = wla_removexattr,
#endif
  
  .opendir = wla_opendir,
  .readdir = wla_readdir,
  .releasedir = wla_releasedir,
  .fsyncdir = wla_fsyncdir,
  .init = wla_init,
  .destroy = wla_destroy,
  .access = wla_access,
  .ftruncate = wla_ftruncate,
  .fgetattr = wla_fgetattr
};

void wla_usage()
{
    fprintf(stderr, "usage:  wlafs [FUSE and mount options] rootDir mountPoint\n");
    abort();
}

int main(int argc, char *argv[])
{
    int fuse_stat;
    struct wla_state *wla_data;

    // wlafs doesn't do any access checking on its own (the comment
    // blocks in fuse.h mention some of the functions that need
    // accesses checked -- but note there are other functions, like
    // chown(), that also need checking!).  Since running wlafs as root
    // will therefore open Metrodome-sized holes in the system
    // security, we'll check if root is trying to mount the filesystem
    // and refuse if it is.  The somewhat smaller hole of an ordinary
    // user doing it with the allow_other flag is still there because
    // I don't want to parse the options string.
    if ((getuid() == 0) || (geteuid() == 0)) {
	   fprintf(stderr, "Running WLA FS as root opens unnacceptable security holes\n");
	   return 1;
    }

    // See which version of fuse we're running
    fprintf(stderr, "Fuse library version %d.%d\n", FUSE_MAJOR_VERSION, FUSE_MINOR_VERSION);
    
    // Perform some sanity checking on the command line:  make sure
    // there are enough arguments, and that neither of the last two
    // start with a hyphen (this will break if you actually have a
    // rootpoint or mountpoint whose name starts with a hyphen, but so
    // will a zillion other programs)
    if ((argc < 3) || (argv[argc-2][0] == '-') || (argv[argc-1][0] == '-'))
	   wla_usage();

    wla_data = malloc(sizeof(struct wla_state));
    if (wla_data == NULL) {
	   perror("main calloc");
	   abort();
    }

    // Pull the rootdir out of the argument list and save it in my
    // internal data
    wla_data->rootdir = realpath(argv[argc-2], NULL);
    argv[argc-2] = argv[argc-1];
    argv[argc-1] = NULL;
    argc--;
    
    wla_data->logfile = log_open();
    
    // turn over control to fuse
    fprintf(stderr, "about to call fuse_main\n");
    fuse_stat = fuse_main(argc, argv, &wla_oper, wla_data);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);
    
    return fuse_stat;
}

