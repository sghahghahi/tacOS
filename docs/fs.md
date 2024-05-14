1) `/dev/nvme0n1p1`
2) `brw-rw---- 1 root disk 259, 1 Apr 22 19:23 /dev/nvme0n1p1`
<br>– The group is `disk` and the file name is yellow

3) 105 MB; took 0.086 seconds
4) 25584 inodes created
5) 88%
6) Names:
<br>– `.`
<br>– `..`
<br>– `lost+found`

7) Output of `ls -l`:
```bash
total 4
-rw-r--r-- 1 sghahghahi2 students  0 May  9 20:55 fileA
-rw-rw-rw- 1 sghahghahi2 students  0 May  9 20:55 fileB
---------- 1 sghahghahi2 students  0 May  9 20:55 fileC
-rw------- 1 sghahghahi2 students 36 May  9 20:56 whoami.txt
```
`umask` masks (subtracts) the bits from the default permissions `666`
Ex: `umask 000` on a file with permissions `666` (default permissions) basically means `666 - 000 = 666`

8) `umask 662` --- default permissions are `666`, so `666 - 662 = 004`. `004` matches the expected permission of read for other users.
9) 18387833643
10) First, `ls` opens the directory. Then, it loops through each `struct dirent` in the directory and prints accordingly.
Examples of inode contents include: the number of links, size, file type, and address
