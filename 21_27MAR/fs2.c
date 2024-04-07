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

int writeBlock(char *filename, int block_no, int block_size, void *buffer)
{
    int file = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (file == ERROR)
    {
        printf("Error in creating file \n");
        return ERROR;
    }
    lseek(file, block_size * block_no, SEEK_SET);
    write(file, buffer, block_size);
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
    SuperBlock.used_block_bit_pattern = malloc(size_block - sizeof(struct SuperBlock) - 8);
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
    writeBlock(filename, 0, SuperBlock.size_block, &SuperBlock);
    writeBlock(filename, 1, SuperBlock.size_block, FileDescriptors);
    for (int i = 2; i < no_block; i++)
    {
        writeBlock(filename, i, SuperBlock.size_block, &DataBlocks[(i - 2)]);
        free(DataBlocks[i - 2].data);
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
    struct FileSystem *fs = malloc(sizeof(char) * 101 + fs_blocksize * 2 + sizeof(struct DataBlock) * fs_no_blocks - 2);
    fs->drive = drive;
    strcpy(fs->filename, filename);
    printf("\n read block size in mount %d\n", fs_blocksize);
    fs->SB.used_block_bit_pattern = malloc(fs_blocksize - sizeof(struct SuperBlock) - 8);
    int status = readBlock(filename, 0, fs_blocksize, &fs->SB);
    printf("\nDone Reading SB status %d\n", status);
    fs->FDs = malloc(fs->SB.size_block);
    status = readBlock(filename, fs->SB.file_descriptor_block, fs->SB.size_block, fs->FDs);
    printf("Done Reading FDs status %d\n", status);
    fs->DBs = malloc((fs->SB.size_block) * (fs->SB.num_blocks - 2));
    for (int j = 2; j < fs->SB.num_blocks; j++)
    {
        fs->DBs[j - 2].data = malloc((fs->SB.size_block - sizeof(int)));
        status = readBlock(filename, j, fs->SB.size_block, &fs->DBs[j - 2]);

        printf("Done Reading DB %d status %d\n", j - 2, status);
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

int mylist(char drive)
{
    for (int i = 0; i < MAX_MOUNTS; i++)
    {
        if (FSs[i].drive == drive)
        {

            printf("\n%d Files :\n", FSs[i].SB.num_files);
            for (int j = 0; j < FSs[i].SB.no_file_descriptor; j++)
            {
                if (FSs[i].FDs[j].first_block_num != -1)
                {
                    printf("Name :%s; First Block :%d, Size :%d, Modified :%s\n", FSs[i].FDs[j].file_name, FSs[i].FDs[j].first_block_num, FSs[i].FDs[j].size_file, FSs[i].FDs[j].modified);
                }
            }
            break;
        }
    }
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
    writeBlock(fs->filename, 0, fs->SB.size_block, &fs->SB);
    writeBlock(fs->filename, 1, fs->SB.size_block, fs->FDs);
    for (int i = 0; i < fs->SB.num_blocks - 2; i++)
    {
        writeBlock(fs->filename, i + 2, fs->SB.size_block, &fs->DBs[i]);
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

int myopen(char *filename, char drive)
{
    int index_file = ERROR;
    struct FileSystem *fs = search_fs_mounted_drive(drive);
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
    printf("\nindex for oft %d", OFTindex);
    OpenFileTable[OFTindex].drive = fs->drive;
    OpenFileTable[OFTindex].index_file = index_file;
    OpenFileTable[OFTindex].pointer = 0;
    no_open_files++;

    printOFT();
    return (OFTindex);
};

int mywrite(int fileno, int size, char *buffer)
{
    // open file in drive
    struct FileSystem *fs = search_fs_mounted_drive(OpenFileTable[fileno].drive);

    int no_blocks_req = size / (fs->SB.size_block-sizeof(int));

    for (int i = 0; i < no_blocks_req; i++)
    {
        if (i==0)
        {
            fs->DBs[fs->FDs->first_block_num].data=malloc((fs->SB.size_block-sizeof(int)));
            for (int i = 0; i < (fs->SB.size_block-sizeof(int)); i++)
            {
                fs->DBs[fs->FDs->first_block_num].data=
            }
            
        }
        
        // int freedb=get_free_block(fs);
        // fs->DBs[freedb]
    }
    

    // no blocks req = buffer size % (blocksize of drive - size(int))
    // recrute blocks for filename - > block will point to a free block; get_free_block()
    // traverse through buffer , write data size amt of buffer in DB
    // next block
};

int myread(char *filename, char drive, int size, char *buffer){};

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

int mycopy(){};

int mycopyfromOS(){};

int mycopytoOS(){};

int main()
{

    for (int i = 0; i < MAX_OPEN_FILES; i++)
    {
        OpenFileTable[i].drive = '\0';
        OpenFileTable[i].index_file = -1;
        OpenFileTable[i].pointer = -1;
    }

    int choice;
    char filename[100], destination[100];
    int size, loop = 1;

    // for (int i = 0; i < MAX_MOUNTS; i++)
    // {
    //     FSs[i].drive='0';
    // }

    printOFT();

    mymkfs("file2", BLOCKSIZE, 10);
    mymkfs("file3", BLOCKSIZE * 2, 20);

    printf("\n AFTER MAKE");
    // printOFT();

    mymount("file2", 'c');

    printf("\n AFTER 1st MOUNT");
    // printOFT();
    mymount("file3", 'd');
    mymount("file3", 'e');
    mymount("file3", 'f');

    print_mounted_FSs();
    myunmount('e');
    mymount("file2", 'g');
    print_mounted_FSs();
    // return 0;
    mymount("file3", 'h');

    // printf("\n AFTER 2nd MOUNT");
    // printOFT();

    // mylist('d');
    // printf("\n AFTER 1st list");
    // printOFT();
    mylist('c');

    // printf("\n AFTER list");
    // printOFT();

    // myunmount('c');

    mylist('d');

    int file = myopen("new file", 'd');
    printf("\nopen file1 : %d", file);
    sleep(2);

    int file2 = myopen("new file2", 'd');
    printf("\nopen file2 : %d", file);

    file = myopen("new in c", 'c');
    printf("\nopen file2 : %d", file);

    printOFT();

    myclose(file2);

    file = myopen("sec in c", 'c');
    printf("\nopen file2 : %d", file);

    printOFT();

    mylist('d');

    mylist('c');
    // myunmount('d');
    // printf("\n lising d\n");
    // mylist('d');
    // printf("\n mounting d\n");

    // mymount("file3",'d');
    // printFS('d');

    return 0;

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
}

// ***********************END OF MY APP************