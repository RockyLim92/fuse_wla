Readme.

Workload Analysis Filesystem is a user spcae file system that using linux fuse.

Before run this file system, please read this README file.

Here are few steps to mount WLA FS to your system.


At first, you should build the wlafs source code that fits your system.
You can configure yout Target Directory and Mount Point.
Target Directory is the Target of Analysis. you cand change target directory
by change the "run.sh". Next, you can also change the Mount Point name but you cannot change
the path of Mount Point. It must be next to source code files.

After you fhish the configuration things, Just type "sh make.sh" and "sh run.sh"
Mount Point directory would show you the things in target directory.
Don't be confused between target directory and mount point.
Both of them will show same contents. However, target Directory is origianl path and
it's mounted filesystem is native filesystem like Ext4, FAT.
Mount point is a artificial directory and it's file system is WLA FS.
So, if the Mount point is still not mounted with WLA FS, it shows nothing.

while the mount point is mounted with WLA FS, this file system analyze the Worklload.
when you unmount WLA FS by typing "sh unmount.sh", analysis.log file will be created.




2016, Rocky, Ajou University, lrocky1229@gmail.com
***************************************************************/

