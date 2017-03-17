#usage

# ./wlafs [FUSE and mount options] targetDirectory mountPoint\n"
# mountPoint must be emtpy directory and normally Accessible.
# If you want get Fuse Mount Options,
# refer the fuse man page ($man fuse) of http://manpages.ubuntu.com/manpages/precise/man8/mount.fuse.8.html

# Example) ./wlafs /home/user/Document /home/user/myMountPoint


./wlafs /home/parallels/ /home/parallels/workload_filesystem/src/mirror/
