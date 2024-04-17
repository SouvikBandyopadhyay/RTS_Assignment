// InitFile
// ReadBlock
// WriteBlock

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BLOCKSIZE 512
#define MAX_NO_BLOCKS 32544
#define OK 0
#define ERROR -1
#define FILENAME_SIZE 20
#define MODIFIED_TIME_SIZE 32
#define MAX_MOUNTS 10
#define MAX_OPEN_FILES 100
#define END_OF_FILE -5
#define EMPTY_BLOCK -2

struct OpenFileTableEntry
{
    int pointer;
    int index_file;
    char drive;
} OpenFileTable[MAX_OPEN_FILES];
int no_open_files;

struct SuperBlock
{
    int size_block;
    int num_blocks;
    int num_files;
    int num_used_block;
    int num_free_block;
    int file_descriptor_block;
    int no_file_descriptor;
    char *used_block_bit_pattern;
} SuperBlock;

struct FileDescriptorEntry
{
    char file_name[FILENAME_SIZE];
    int first_block_num;
    int size_file;
    char modified[MODIFIED_TIME_SIZE];
} *FileDescriptors;

struct DataBlock
{
    char *data;
    int next_block;
} *DataBlocks;

struct FileSystem
{
    char filename[100];
    char drive;
    struct SuperBlock SB;
    struct FileDescriptorEntry *FDs;
    struct DataBlock *DBs;
    struct FileSystem *next_FS;
} *FSs;
int No_FS_mntd = 0;

// Function to set a specific bit in the array
void set_bit(char *bit_array, int index)
{
    bit_array[index / 8] |= (1 << (index % 8));
}

// Function to clear a specific bit in the array
void clear_bit(char *bit_array, int index)
{
    bit_array[index / 8] &= ~(1 << (index % 8));
}

// Function to check if a specific bit is set
int check_bit(char *bit_array, int index)
{
    return (bit_array[index / 8] & (1 << (index % 8))) != 0;
}

// *********************DEVICE DRIVERS****************************************
int writeSuperBlock(char *filename, int block_no, int block_size, struct SuperBlock *buffer)
{
    int file = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (file == ERROR)
    {
        printf("Error in creating file \n");
        return ERROR;
    }
    lseek(file, block_size * block_no, SEEK_SET);

    write(file, buffer, sizeof(struct SuperBlock));
    write(file, buffer->used_block_bit_pattern, block_size - sizeof(struct SuperBlock));

    close(file);
}

int writeBlock(char *filename, int block_no, int block_size, void *buffer)
{
    int file = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (file == ERROR)
    {
        printf("Error in creating file \n");
        return ERROR;
    }
    lseek(file, block_size * block_no, SEEK_SET);

    int status = write(file, buffer, block_size);
    if (status == -1)
    {
        perror("Error writing to stdout");
    }

    close(file);
}

int writeDataBlock(char *filename, int block_no, int block_size, struct DataBlock *buffer)
{
    int file = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (file == ERROR)
    {
        printf("Error in creating file \n");
        return ERROR;
    }
    lseek(file, block_size * block_no, SEEK_SET);

    write(file, &buffer->next_block, sizeof(int));
    write(file, buffer->data, block_size - sizeof(int));

    close(file);
}

int readSuperBlock(char *filename, int block_no, int block_size, struct SuperBlock *buffer)
{
    int file = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (file == ERROR)
    {
        printf("Error in creating file \n");
        return ERROR;
    }
    lseek(file, block_size * block_no, SEEK_SET);
    read(file, buffer, sizeof(struct SuperBlock));
    buffer->used_block_bit_pattern = malloc(buffer->size_block - sizeof(struct SuperBlock));
    read(file, buffer->used_block_bit_pattern, block_size - sizeof(struct SuperBlock));
    close(file);
}

int readBlock(char *filename, int block_no, int block_size, void *buffer)
{
    int file = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (file == ERROR)
    {
        printf("Error in creating file \n");
        return ERROR;
    }
    lseek(file, block_size * block_no, SEEK_SET);
    read(file, buffer, block_size);
    close(file);
}

int readDataBlock(char *filename, int block_no, int block_size, struct DataBlock *buffer)
{
    int file = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (file == ERROR)
    {
        printf("Error in creating file \n");
        return ERROR;
    }
    lseek(file, block_size * block_no, SEEK_SET);
    read(file, &buffer->next_block, sizeof(int));
    read(file, buffer->data, block_size - sizeof(int));
    close(file);
}

int initFile(char *filename, int block_size, int no_block)
{
    int file = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (file == ERROR)
    {
        printf("Error in creating file \n");
        return ERROR;
    }
    char *blocks = calloc(no_block, block_size);
    write(file, blocks, block_size * (no_block));
    free(blocks);
    close(file);
    return OK;
}

// *************************END OF DEVICE DRIVER************************

// *************************MY FS***************************************

int mymkfs(char *filename, int size_block, int no_block)
{
    // printf(" block size %d, allowd min %lu",size_block ,(sizeof(struct SuperBlock) > sizeof(struct FileDescriptorEntry)) ? sizeof(struct SuperBlock) : sizeof(struct FileDescriptorEntry));
    if ((no_block > MAX_NO_BLOCKS) || (no_block < 2) || (size_block < (((sizeof(struct SuperBlock) > sizeof(struct FileDescriptorEntry))) ? sizeof(struct SuperBlock) : sizeof(struct FileDescriptorEntry))))
    {
        printf("\nBlock limit exceded, with a min block size: %lu, max block size: %d, min no. of block: %d, max no. of blocks %d\n", (sizeof(struct SuperBlock) > sizeof(struct FileDescriptorEntry)) ? sizeof(struct SuperBlock) : sizeof(struct FileDescriptorEntry), BLOCKSIZE, 2, MAX_NO_BLOCKS);
        return ERROR;
    }

    SuperBlock.num_files = 0;
    SuperBlock.num_blocks = no_block;
    SuperBlock.size_block = size_block;
    SuperBlock.num_used_block = 2;
    SuperBlock.num_free_block = no_block - 2;
    SuperBlock.file_descriptor_block = 1;
    SuperBlock.no_file_descriptor = size_block / sizeof(struct FileDescriptorEntry);
    SuperBlock.used_block_bit_pattern = malloc(size_block - sizeof(struct SuperBlock));
    for (int i = 0; i < no_block; i++)
    {
        clear_bit(SuperBlock.used_block_bit_pattern, i);
    }
    set_bit(SuperBlock.used_block_bit_pattern, 0);
    set_bit(SuperBlock.used_block_bit_pattern, 1);

    FileDescriptors = malloc(size_block);
    printf("\nNo of FDs %d\n", SuperBlock.no_file_descriptor);
    for (int i = 0; i < SuperBlock.no_file_descriptor; i++)
    {
        strcpy(FileDescriptors[i].file_name, "Empty");
        FileDescriptors[i].first_block_num = -1;
        FileDescriptors[i].size_file = 0;
        strcpy(FileDescriptors[i].modified, "00:00");
    }

    DataBlocks = malloc(size_block * (no_block - 2));
    for (int i = 0; i < no_block - 2; i++)
    {
        DataBlocks[i].next_block = -2;
        DataBlocks[i].data = malloc(size_block - sizeof(int));
    }

    initFile(filename, size_block, no_block + 1);
    printf("SB. first int %d, second int %d", SuperBlock.size_block, SuperBlock.num_blocks);

    writeSuperBlock(filename, 0, SuperBlock.size_block, &SuperBlock);
    writeBlock(filename, 1, SuperBlock.size_block, FileDescriptors);


    for (int i = 0; i < no_block - 2; i++)
    {
        strcpy(DataBlocks[i].data, "Nothing");
        writeDataBlock(filename, i + 2, SuperBlock.size_block, &DataBlocks[i]);
        free(DataBlocks[i].data);
    }

    free(DataBlocks);
    free(SuperBlock.used_block_bit_pattern);

    return OK;
};

int printFS(char drive)
{
    printf("\n Searching for drive %c in FSS", drive);
    struct FileSystem *last_fs = FSs;
    for (int i = 0; last_fs != NULL; i++)
    {

        printf("\n Index %d", i);
        if (last_fs->drive == drive)
        {
            printf("\nFOUND %c:\n", drive);
            printf("\nSUPER BLOCK: \n num_files %d;\n num_blocks %d;\n size_block %d;\n num_used_block %d;\n num_free_block %d;\n file_descriptor_block %d;\n no_file_descriptor %d;\n", last_fs->SB.num_files, last_fs->SB.num_blocks, last_fs->SB.size_block, last_fs->SB.num_used_block, last_fs->SB.num_free_block, last_fs->SB.file_descriptor_block, last_fs->SB.no_file_descriptor);

            printf("\nFILE DESC :\n");
            for (int j = 0; j < last_fs->SB.no_file_descriptor; j++)
            {
                printf("Name :%s; First Block :%d, Size :%d, Modified :%s\n", last_fs->FDs[j].file_name, last_fs->FDs[j].first_block_num, last_fs->FDs[j].size_file, last_fs->FDs[j].modified);
            }

            printf("\nDATA BLOCKS :\n");
            for (int j = 0; j < last_fs->SB.num_blocks - 2; j++)
            {
                printf("Num :%d; Next Block :%d\n", j, last_fs->DBs[j].next_block);
            }

            break;
        }
        else
        {
            last_fs = last_fs->next_FS;
        }
    }
    return OK;
}

int mymount(char *filename, char drive)
{
    struct FileSystem *last_fs = FSs;

    // for (int i = 0; i < MAX_MOUNTS; i++)
    // {
    //     printf("\ndrive in fs %c\n", FSs[i].drive);
    //     if ((FSs[i].drive == '0') || (FSs[i].drive == '\0'))
    //     {
    // strcpy(FSs[i].filename, filename);

    int file = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    int fs_blocksize;
    read(file, &fs_blocksize, sizeof(int));
    int fs_no_blocks;
    read(file, &fs_no_blocks, sizeof(int));
    struct FileSystem *fs = malloc(fs_blocksize * fs_no_blocks);
    fs->drive = drive;
    strcpy(fs->filename, filename);
    printf("\n read block size in mount %d\n", fs_blocksize);
    fs->SB.used_block_bit_pattern = malloc(fs_blocksize - sizeof(struct SuperBlock));
    int status = readSuperBlock(filename, 0, fs_blocksize, &fs->SB);
    printf("\nDone Reading SB status %d\n", status);
    fs->FDs = malloc(fs->SB.size_block);
    status = readBlock(filename, fs->SB.file_descriptor_block, fs->SB.size_block, fs->FDs);
    printf("Done Reading FDs status %d\n", status);
    fs->DBs = malloc((fs->SB.size_block) * (fs->SB.num_blocks - 2));
    for (int j = 2; j < fs->SB.num_blocks; j++)
    {
        fs->DBs[j - 2].data = malloc((fs->SB.size_block - sizeof(int)));
        status = readDataBlock(filename, j, fs->SB.size_block, &fs->DBs[j - 2]);

        // printf("Done Reading DB %d status %d\n", j - 2, status);
    }
    //         break;
    //     }

    // }

    for (int i = 0; i < No_FS_mntd - 1; i++)
    {
        last_fs = last_fs->next_FS;
        printf("\n found latest fs %c", last_fs->drive);
    }
    if (No_FS_mntd == 0)
    {
        FSs = fs;
    }
    else
    {
        last_fs->next_FS = fs;
    }
    No_FS_mntd++;
    printFS(drive);
};

int myunmount(char drive)
{
    printf("\nUnmounting %c", drive);
    struct FileSystem *last_fs = FSs;
    struct FileSystem *prev_node = FSs;
    for (int i = 0; last_fs != NULL; i++)
    {

        printf("\n Index %d", i);
        if (last_fs->drive == drive)
        {
            if (i == 0)
            {
                FSs = FSs->next_FS;
            }
            else
            {
                prev_node->next_FS = last_fs->next_FS;
            }

            printf("\nFOUND %c:\n", drive);
            last_fs->drive = '0';
            free(last_fs->SB.used_block_bit_pattern);
            free(last_fs->FDs);
            for (int j = 0; j < last_fs->SB.num_blocks - 2; j++)
            {
                // free(last_fs->DBs[j].data);       //--------------- UNEXPECTED DOUBLE FREE
            }
            free(last_fs->DBs);
            free(last_fs);
            No_FS_mntd--;

            break;
        }
        else
        {
            prev_node = last_fs;
            last_fs = last_fs->next_FS;
        }
    }
    return OK;
};

struct FileSystem *search_fs_mounted_drive(char drive)
{
    struct FileSystem *last_fs = FSs;
    for (int i = 0; last_fs != NULL; i++)
    {

        if (last_fs->drive == drive)
        {
            return last_fs;
        }

        last_fs = last_fs->next_FS;
    }
    last_fs = NULL;
    return last_fs;
}

int mylist(char drive)
{
    struct FileSystem *fs = search_fs_mounted_drive(drive);
    // for (int i = 0; i < MAX_MOUNTS; i++)
    // {
    //     if (FSs[i].drive == drive)
    //     {
    if (fs == NULL)
    {
        return ERROR;
    }

    printf("\n%d Files :\n", fs->SB.num_files);
    for (int j = 0; j < fs->SB.no_file_descriptor; j++)
    {
        if (fs->FDs[j].first_block_num != -1)
        {
            printf("Name :%s; First Block :%d, Size :%d, Modified :%s\n", fs->FDs[j].file_name, fs->FDs[j].first_block_num, fs->FDs[j].size_file, fs->FDs[j].modified);
        }
    }
    //         break;
    //     }
    // }
    return OK;
};

int get_free_block(struct FileSystem *fs)
{
    for (int i = 0; i < fs->SB.num_blocks - 2; i++)
    {
        printf("\n checking bit %d : %d", i, check_bit(fs->SB.used_block_bit_pattern, i));
        // if(check_bit( fs->SB.used_block_bit_pattern, i) == 0){
        if (fs->DBs[i].next_block == EMPTY_BLOCK)
        {
            set_bit(fs->SB.used_block_bit_pattern, i + 2);
            fs->DBs[i].next_block = END_OF_FILE;
            return i;
        }
    }
    return ERROR;
}

char *getCurrentTimeAsString(char *epochString)
{
    // Get current time
    time_t currentTime;
    time(&currentTime);

    // Convert current time to epoch format

    snprintf(epochString, 32, "%ld", (long)currentTime);

    // Print epoch time string
    printf("Epoch Time: %s\n", epochString);

    return epochString;
}

int update_file(struct FileSystem *fs)
{
    writeSuperBlock(fs->filename, 0, fs->SB.size_block, &fs->SB);
    writeBlock(fs->filename, 1, fs->SB.size_block, fs->FDs);
    for (int i = 0; i < fs->SB.num_blocks - 2; i++)
    {
        writeDataBlock(fs->filename, i + 2, fs->SB.size_block, &fs->DBs[i]);
    }
    return OK;
}

int create_file(char *filename, struct FileSystem *fs)
{
    if (fs->SB.num_files < fs->SB.no_file_descriptor)
    {
        for (int i = 0; i < fs->SB.no_file_descriptor; i++)
        {
            if (fs->FDs[i].first_block_num == -1)
            {
                strcpy(fs->FDs[i].file_name, filename);
                printf("\nname : %s", fs->FDs[i].file_name);
                fs->FDs[i].size_file = 0;
                fs->FDs[i].first_block_num = get_free_block(fs);
                getCurrentTimeAsString(fs->FDs[i].modified);
                // strcpy( fs->FDs[i].modified, getCurrentTimeAsString());

                fs->SB.num_free_block--;
                fs->SB.num_used_block++;
                fs->SB.num_files++;

                update_file(fs);
                return i;
            }
        }
    }
}

int get_free_OFTE()
{
    for (int i = 0; i < MAX_OPEN_FILES; i++)
    {
        if (OpenFileTable[i].drive == '\0')
        {
            return i;
        }
    }
    return ERROR;
}

int printOFT()
{
    printf("\nOpen File Table:\n");
    for (int i = 0; i < MAX_OPEN_FILES; i++)
    {
        if (OpenFileTable[i].drive != '\0')
        {
            printf("%d -> File index :%d, Drive: %c, Pointer :%d\n", i, OpenFileTable[i].index_file, OpenFileTable[i].drive, OpenFileTable[i].pointer);
        }
        else
        {
            // printf("\n%d -> FileNo :%d, filename index :%d, drive index :%d, Pointer :%d", i, OpenFileTable[i].file_no, OpenFileTable[i].index_file, OpenFileTable[i].index_FS, OpenFileTable[i].pointer);
        }
    }
}

int print_mounted_FSs()
{
    printf("\nMounted Files");
    struct FileSystem *last_fs = FSs;
    for (int i = 0; last_fs != NULL; i++)
    {

        printf("\n @%d FileName: %s, Drive: %c", i, last_fs->filename, last_fs->drive);

        last_fs = last_fs->next_FS;
    }
    printf("\n");
    return OK;
}

int myopen(char *filename, char drive)
{
    int index_file = ERROR;
    struct FileSystem *fs = search_fs_mounted_drive(drive);
    if (fs == NULL)
    {
        return ERROR;
    }

    // for (int i = 0; i < MAX_MOUNTS; i++)
    // {
    //     if (FSs[i].drive == drive)
    //     {
    for (int j = 0; j < fs->SB.num_files; j++)
    {
        if (strcmp(fs->FDs[j].file_name, filename) == 0)
        {
            index_file = j;
            break;
        }
    }
    //     }
    // }
    if (fs == (struct FileSystem *)NULL)
    {
        return ERROR;
    }

    if ((index_file == ERROR))
    {
        index_file = create_file(filename, fs);
    }
    int OFTindex = get_free_OFTE();
    // printf("\nindex for oft %d", OFTindex);
    OpenFileTable[OFTindex].drive = fs->drive;
    OpenFileTable[OFTindex].index_file = index_file;
    OpenFileTable[OFTindex].pointer = 0;
    no_open_files++;

    // printOFT();
    return (OFTindex);
};

int mywrite(int fileno, int size, char *buffer)
{
    int k = 0;
    // open file in drive
    struct FileSystem *fs = search_fs_mounted_drive(OpenFileTable[fileno].drive);
    if (fs == NULL)
    {
        return ERROR;
    }

    int no_blocks_req = size / (fs->SB.size_block - sizeof(int));
    if (size % (fs->SB.size_block - sizeof(int)) != 0)
    {
        no_blocks_req++;
    }
    // printf("\nNo of blocks req %d datasize %lu size %d\n",no_blocks_req, (fs->SB.size_block-sizeof(int)), size);
    int head_block = fs->FDs[OpenFileTable[fileno].index_file].first_block_num;

    for (int i = 0; i < no_blocks_req; i++)
    {
        // make a db except for i==0
        if (i != 0)
        {

            if (fs->DBs[head_block].next_block == END_OF_FILE)
            {
                fs->DBs[head_block].next_block = get_free_block(fs);
            }
            head_block = fs->DBs[head_block].next_block;
        }

        // write on the data block
        fs->DBs[head_block].data = malloc(fs->SB.size_block - sizeof(int));
        for (int i = 0; (i < (fs->SB.size_block - sizeof(int))) && (k < size); i++)
        {

            fs->DBs[head_block].data[i] = buffer[k];
            // printf("\nWriting char %c %d at drive %c",buffer[k],k,fs->drive);
            k++;
        }

        // if (i==0)
        // {
        // fs->DBs[fs->FDs->first_block_num].data=malloc((fs->SB.size_block-sizeof(int)));

        // int freedb=get_free_block(fs);

        // }

        // fs->DBs[freedb]
    }

    // no blocks req = buffer size % (blocksize of drive - size(int))
    // recrute blocks for filename - > block will point to a free block; get_free_block()
    // traverse through buffer , write data size amt of buffer in DB
    // next block
    fs->FDs[OpenFileTable[fileno].index_file].size_file = size;
    // printf("\n size written %d, current pointer val %d\n" , OpenFileTable[fileno].pointer, size);
    printOFT();
    update_file(fs);
    return OK;
};

int get_file_size(int fileno)
{
    struct FileSystem *fs = search_fs_mounted_drive(OpenFileTable[fileno].drive);
    if (fs == NULL)
    {
        return ERROR;
    }
    return fs->FDs[OpenFileTable[fileno].index_file].size_file;
    // int k=1;
    // // open file in drive
    // struct FileSystem *fs = search_fs_mounted_drive(OpenFileTable[fileno].drive);

    // int head_block = fs->FDs[OpenFileTable[fileno].index_file].first_block_num;

    // for (; head_block != END_OF_FILE;)
    // {
    //     head_block= fs->DBs[head_block].next_block;
    //     k=k+fs->DBs[head_block].data;
    // }
    // return k;
};

int myread(int fileno, int size, char *buffer)
{
    int k = 0;
    // open file in drive
    struct FileSystem *fs = search_fs_mounted_drive(OpenFileTable[fileno].drive);
    if (fs == NULL)
    {
        return ERROR;
    }

    int head_block = fs->FDs[OpenFileTable[fileno].index_file].first_block_num;

    for (int i = 0; head_block != END_OF_FILE; i++)
    {
        if (i != 0)
        {
            head_block = fs->DBs[head_block].next_block;
        }

        // read the data block
        for (int i = 0; (i < (fs->SB.size_block - sizeof(int))) && (k < size); i++)
        {
            buffer[k] = fs->DBs[head_block].data[i];
            printf("char:%c k:%d i:%d\n", buffer[k], k, i);
            k++;
        }
    }
    return OK;
};

int myclose(int i)
{
    if (i > MAX_OPEN_FILES)
    {
        return ERROR;
    }

    OpenFileTable[i].drive = '\0';
    return OK;
}

// **********************END OF MyFS******************

// **********************My APP***********************

int mycopy(char drivefrom, char *filefrom, char driveto, char *fileto)
{
    struct FileSystem *fsfrom = search_fs_mounted_drive(drivefrom);
    struct FileSystem *fsto = search_fs_mounted_drive(driveto);
    if ((fsfrom == NULL) || (fsto == NULL))
    {
        return ERROR;
    }
    int fromfile = myopen(filefrom, drivefrom);
    int tofile = myopen(fileto, driveto);
    int size = get_file_size(fromfile);
    // printf("\nread file size to be coppeid %d\n",size);
    char *buff = malloc(size);
    myread(fromfile, size, buff);
    // printf("\nwrititng to D %c\n",OpenFileTable[tofile].drive);
    mywrite(tofile, size, buff);
    myclose(fromfile);
    myclose(tofile);
    update_file(fsto);
    return OK;
};

int mycopyfromOS(char *source, char *destination_filename, char destination_drive)
{
    int fd = open(source, O_RDONLY);

    off_t file_size = lseek(fd, 0, SEEK_END);

    char *buffer = (char *)malloc(file_size);

    lseek(fd, 0, SEEK_SET);
    read(fd, buffer, file_size);
    close(fd);
    int file = myopen(destination_filename, destination_drive);
    mywrite(file, file_size, buffer);
    myclose(file);
};

int mycopytoOS(char *destination, char *source_filename, char source_drive)
{

    int file = myopen(source_filename, source_drive);
    int size = get_file_size(file);
    char *buffer = malloc(size);
    myread(file, size, buffer);
    myclose(file);

    int fd = open(destination, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    write(fd, buffer, size);
    close(fd);
};

int main()
{
    int choice;
    char filename[100], drive, drivefrom, driveto, filefrom[100], fileto[100];
    int size_block, no_block, fileno, size;
    char buffer[100];

    while (1)
    {
        printf("\nMenu:\n========File System======\n");
        printf("11. Create File System (mymkfs) inputs: filename\n");
        printf("12. Mount File System (mymount) inputs: filename, drive\n");
        printf("13. Print File System Metadata (printFS) inputs: drive\n");
        printf("14. Print Mounted File Systems (print_mounted_FSs)\n");
        printf("15. Unmount File System (myunmount) inputs: drive\n\n");
        printf("========Files in mounted FS========\n");
        printf("21. Open File (myopen) inputs: filename, drive\n");
        printf("22. Print Open File Table (printOFT)\n");
        printf("23. Write to File (mywrite) inputs: filenumber, size, content\n");
        printf("24. Get File Size (get_file_size) inputs: filenumber\n");
        printf("25. Read from File (myread) inputs: filenumber, size\n");
        printf("26. Close File (myclose) inputs: filenumber\n");
        printf("27. List Files (mylist) inputs: drive\n\n");
        printf("========Copy funtions========\n");
        printf("31. Copy File within File System (mycopy) inputs: filename, drive, filename, drive\n");
        printf("32. Copy File from OS to File System (mycopyfromOS) inputs: OSfilename, filename, drive\n");
        printf("33. Copy File from File System to OS (mycopytoOS) inputs: OSfilename, filename, drive\n");
        printf("00. Exit\n");

        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 11:
            printf("Enter filename, size of block (min 512), and number of blocks(min 20): ");
            scanf("%s %d %d", filename, &size_block, &no_block);
            mymkfs(filename, size_block, no_block);
            break;
        case 12:
            printf("Enter filename and drive letter: ");
            scanf("%s %c", filename, &drive);
            mymount(filename, drive);
            break;
        case 13:
            printf("Enter drive letter: ");
            scanf(" %c", &drive);
            printFS(drive);
            break;
        case 15:
            printf("Enter drive letter: ");
            scanf(" %c", &drive);
            myunmount(drive);
            break;
        case 27:
            printf("Enter drive letter: ");
            scanf(" %c", &drive);
            mylist(drive);
            break;
        case 22:
            printOFT();
            break;
        case 14:
            print_mounted_FSs();
            break;
        case 21:
            printf("Enter filename and drive letter: ");
            scanf("%s %c", filename, &drive);
            int file = myopen(filename, drive);
            printf("\nOpened File : %d", file);
            break;
        case 23:
            printf("Enter file number, size, and data: ");
            scanf("%d %d %s", &fileno, &size, buffer);
            mywrite(fileno, size, buffer);
            break;
        case 24:
            printf("Enter file number: ");
            scanf("%d", &fileno);
            printf("File size: %d\n", get_file_size(fileno));
            break;
        case 25:
            printf("Enter file number and size: ");
            scanf("%d %d", &fileno, &size);
            char * bufferread = malloc(size);
            myread(fileno, size, bufferread);
            printf("Data read: %s\n", bufferread);
            break;
        case 26:
            printf("Enter file number: ");
            scanf("%d", &fileno);
            myclose(fileno);
            break;
        case 31:
            printf("Enter source drive, filename, destination drive, and destination filename: ");
            scanf(" %c %s %c %s", &drivefrom, filefrom, &driveto, fileto);
            mycopy(drivefrom, filefrom, driveto, fileto);
            break;
        case 32:
            printf("Enter source filename, destination filename, and destination drive: ");
            scanf("%s %s %c", filename, fileto, &drive);
            mycopyfromOS(filename, fileto, drive);
            break;
        case 33:
            printf("Enter destination filename, source filename, and source drive: ");
            scanf("%s %s %c", filename, filefrom, &drive);
            mycopytoOS(filename, filefrom, drive);
            break;
        case 00:
            printf("Exiting...");
            return 0;
        default:
            printf("Invalid choice!\n");
            break;
        }
    }

    return 0;
}
// int main()
// {

//     for (int i = 0; i < MAX_OPEN_FILES; i++)
//     {
//         OpenFileTable[i].drive = '\0';
//         OpenFileTable[i].index_file = -1;
//         OpenFileTable[i].pointer = -1;
//     }

//     int choice;
//     char filename[100], destination[100];
//     int size, loop = 1;

//     // for (int i = 0; i < MAX_MOUNTS; i++)
//     // {
//     //     FSs[i].drive='0';
//     // }

//     printOFT();

//     mymkfs("file2", BLOCKSIZE, 10);
//     mymkfs("file3", BLOCKSIZE * 2, 20);

//     printf("\n AFTER MAKE");
//     // printOFT();

//     mymount("file2", 'c');

//     printf("\n AFTER 1st MOUNT");
//     // printOFT();
//     mymount("file3", 'd');
//     mymount("file3", 'e');
//     mymount("file3", 'f');

//     print_mounted_FSs();
//     myunmount('e');
//     mymount("file2", 'g');
//     print_mounted_FSs();
//     // return 0;
//     mymount("file3", 'h');

//     // printf("\n AFTER 2nd MOUNT");
//     // printOFT();

//     // mylist('d');
//     // printf("\n AFTER 1st list");
//     // printOFT();
//     mylist('c');

//     // printf("\n AFTER list");
//     // printOFT();

//     // myunmount('c');

//     mylist('d');

//     int file = myopen("new file", 'd');
//     printf("\nopen file1 : %d", file);
//     sleep(2);

//     int file2 = myopen("new file2", 'd');
//     printf("\nopen file2 : %d", file);

//     file = myopen("new in c", 'c');
//     printf("\nopen file2 : %d", file);

//     printOFT();

//     myclose(file2);

//     file = myopen("sec in c", 'c');
//     printf("\nopen file2 : %d", file);

//     // mywrite(file, 2000, "41215860590629816389251980957455255730155710435674684626245666084393793616482987125324178198059320781825461589955444130108871711927477012400730559127449290285139855855795741060689401197377918864540098853447199157431791899507065683757154258998938140405895442669615827408814362516128167755458426971247008678091628782691016953845351119520754601206232099582197190925708421858807503773995899584581864638961939622477014779811883140516122979636669553682940016484008483370906069593700237526658072176275846341948371509190795692277659646377584693928105463848459215741845100493135662293787255817249876094923789273113624767657818458038770168114897848335522321815677830230689592923307874512937445357737238814332961021551806340754766338655836538164430385084475305451913088948628482208681810003270486886440424488448428564585575857398296725650017114239976849692278893331428800129029854516648938422816956838712763917721234581950963319757487498296193720592300683865159145788995898018972468634234891264079627904792617054970587570257734904016095172320094865139389144955970801046564962155296896400386146515352805936982575675257512834233883150839809517598882121891835736471228302344470500643214679841126004640759081645074984770684974180204141279108181984872433577580835894157718737059012217674185080130046358657091299039276314791972875557110906549769833019250505897781548691406714481324008898276124299090818069963947300724406256886094552595392412206067897483596603626388854074128366778128106142689927030614246287278703886770120199428169746534890342020933135838826237936129647990617973849501976780923133281386103031960632468544310343515365768259710941290186844455857151044998155187152302267558175698186094378655963802978736460233484318082146982355082026218014791520985605556148882724225307825462489850231205527409435454200908196394858699328799780089975091299802658522015017371193941540300582076074307880120290463510117743980558464569351693059687192417606907795898531636558344188650084593334106922209654039565254212701597929");
//     mywrite(file, 20, "12345678900987654321");
//     myclose(file);
//     file = myopen("sec in c", 'c');
//     printf("\nopen file: %d", file);

//     char buff[20];
//     myread(file, 20, buff);
//     printf("\nRead from file %s\n", buff);
//     // printFS('c');
//     mycopy('c', "sec in c", 'd', "Anew File");

//     file = myopen("Anew File", 'd');

//     printOFT();
//     myread(file, 20, buff);

//     printf("\nRead from d's file %s\n", buff);

//     mycopytoOS("aa.txt", "Anew File", 'd');

//     int f = open("aa.txt", O_RDONLY);
//     char buff2[20];
//     read(file, buff2, 20);
//     printf("\nreading from OS %s\n", buff2);

//     mycopyfromOS("aa.txt", "X File", 'c');

//     file = myopen("X File", 'c');

//     printOFT();
//     myread(file, 20, buff2);

//     printf("\nRead from my copy file %s\n", buff);
//     // myunmount('d');
//     // printf("\n lising d\n");
//     // mylist('d');
//     // printf("\n mounting d\n");

//     // mymount("file3",'d');
//     // printFS('d');

//     return 0;

// while(loop){
//     printf("Choose a function to call:\n");
//     printf("1. Create a FS\n");
//     printf("2. Mount a FS\n");
//     printf("3. List all files in a FS\n");
//     printf("4. Copy File\n");
//     printf("5. Copy to OS\n");
//     printf("6. Copy from OS\n");
//     printf("6. exit\n");

//     scanf("%d", &choice);
//     switch (choice) {
//         case 1:
//             printf("Enter filename: ");
//             scanf("%s", filename);
//             myreadfs(filename);
//             break;
//         case 2:
//             printf("Enter filename: ");
//             scanf("%s", filename);
//             printf("Enter destination: ");
//             scanf("%s", destination);
//             mymount(filename, destination);
//             break;
//         case 3:
//             printf("Enter filename: ");
//             scanf("%s", filename);
//             mydir(filename);
//             break;
//         case 4:
//             printfs();
//             break;
//         case 5:
//             printf("Enter size: ");
//             scanf("%d", &size);
//             printf("Enter filename: ");
//             scanf("%s", filename);
//             mymkfs(size, filename);
//             break;
//         case 6:
//             printf("Enter source: ");
//             scanf("%s", filename);
//             printf("Enter filename: ");
//             scanf("%s", destination);
//             int fileno = myopen(filename, destination);
//             printf("\n File No : %d\n",fileno);
//             break;
//         case 10:
//             loop = 0;
//             break;
//         default:
//             printf("Invalid choice!\n");
//             break;
//     }
// }
// return 0;
// }

// ***********************END OF MY APP************