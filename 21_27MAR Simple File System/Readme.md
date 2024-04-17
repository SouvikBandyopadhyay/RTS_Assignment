In the previous version some of the funtions where ineffective and incomplete, this new version has all funtions working

difference form previos verison:

+WriteSuperBlock() funtion to exclusively write super block
+WriteDataBlock() funtion to write Data block
+ReadSuperBlock() funtion to write superblock
+ReadDataBlock() funtion to write data block

modified funtions:
mycopy()
mycopyfromOS()
mycopytoOS()

Pending Work:
Everytime a file in the filesystem is written on, it overwrites existing information, the concept of file cursor pointer is still not working, as the logic's implimentation is difficult for me to figureout. 

Working Principle:

The application provides

FS functionalities like creating a file system, mounting a filesystem to a drive, printing the meta data about a mounted file system, printing all available mounted file systems, and unmounting file systems.

Using the device driver the application provides my file operations that allow us to open a file in a mounted file system , print open file table, write to a file that is previously opened, get the file size of a opened file, read from an opened file, and close an opened file, finally list all available files in a drive.

Using this file operations, the application provides some copy functions like copping a file from file system to file system, copying a file from operating system to file system, copying of file from file system to operating system.