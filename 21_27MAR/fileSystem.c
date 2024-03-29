/*
Data Structures:
    SuperBlock{
        num_files
        num_blocks
        size_block
    }
    File Descriptor entry{
        file_name[20]
        first_block_num[3]
        size_file[4]
        modified[32]
    }
    Block{
        data[512]
        next_block[3]
    }
Funtionalities with Meta Data:
    Myputfs: filename , fs
        puts currect fs to relivant file        Done...
    UI-Mymkfs: filename
        creates file system and puts on relivant file       Done...
    UI-Myreadfs: filename
        reads entire file sytem, returnes metadata      Done...
    UI-Mymount: filename , mount_loc
        reads entire file sytem, meta data + data and puts in mount location    Done...
    UI-Mydir: filename
        print all file descriptors      Done...
    Myprintfs: filename
        prints current file system meta data    Done...

Funtionalities with Data:
    Myopen: filename
        searches for filename in descriptors, if present return file descriptor entry index,
        if not present, create new file and returnfile descriptor entry index
    Myread: descriptor_index

    Mywrite: descriptor_index

    CopytoOS: filename, destination
        copy file from our filesystem to OS/destination
    CopyfromOS: filename, destination
        copy file from OS/destination to our filesystem
    Mylseek: descriptor_index, position
        calc block_num & position % size_block



*/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#define FILENAME_SIZE 20
#define BLOCK_DATA_SIZE 512
#define MAX_FD_ENTRY 10
#define MODIFIED_TIME_SIZE 32
#define OK 0
#define ERROR -1

struct SuperBlock
{
    int num_files;
    int num_blocks;
    int size_block;
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
    char data[BLOCK_DATA_SIZE];
    int next_block;
} *DataBlocks;

void getCurrentTimeAsString(char *timeString) {
    time_t currentTime;
    struct tm *localTime;

    // Get the current time
    currentTime = time(NULL);
    if (currentTime == ((time_t)-1)) {
        perror("Time retrieval failed");
        exit(EXIT_FAILURE);
    }

    // Convert the current time to local time
    localTime = localtime(&currentTime);
    if (localTime == NULL) {
        perror("Local time conversion failed");
        exit(EXIT_FAILURE);
    }

    // Format the time as a string
    if (strftime(timeString, MODIFIED_TIME_SIZE, "%Y-%m-%d %H:%M:%S", localTime) == 0) {
        perror("String formatting failed");
        exit(EXIT_FAILURE);
    }

}

int myreadfs(char *filename)
{
    FILE *file = fopen(filename, "r+");
    if (file == NULL)
    {
        printf("Error reading file");
        return ERROR;
    }
    fread(&SuperBlock, sizeof(struct SuperBlock), 1, file);

    FileDescriptors = malloc(sizeof(struct FileDescriptorEntry) * MAX_FD_ENTRY);
    DataBlocks = malloc(sizeof(struct DataBlock) * SuperBlock.num_blocks);

    fread(FileDescriptors, sizeof(struct FileDescriptorEntry), MAX_FD_ENTRY, file);
    fread(DataBlocks, sizeof(struct DataBlock), SuperBlock.num_blocks, file);

    fclose(file);

    return OK;
}

int myputfs(char *filename)
{
    FILE *file = fopen(filename, "w+");
    if (file == NULL)
    {
        printf("Error in creating file \n");
        return ERROR;
    }

    fwrite(&SuperBlock, sizeof(SuperBlock), 1, file);
    fwrite(FileDescriptors, sizeof(struct FileDescriptorEntry), MAX_FD_ENTRY, file);
    fwrite(DataBlocks, sizeof(struct DataBlock), SuperBlock.num_blocks, file);

    fclose(file);

    return OK;
}

int mymount(char *filename, char *destination)
{
    if (myreadfs(filename) == ERROR)
    {
        printf("/nerror in fetching from file/n");
        return ERROR;
    }
    if (myputfs(destination) == ERROR)
    {
        printf("/nerror in putting to dest/n");
        return ERROR;
    }
    free((void *)FileDescriptors);
    free((void *)DataBlocks);

    return OK;
}

void mydir(char *filename)
{
    FILE *file = fopen(filename, "r+");
    if (file == NULL)
    {
        printf("Error reading file");
        return;
    }
    struct SuperBlock SB;
    fread(&SB, sizeof(struct SuperBlock), 1, file);
    struct FileDescriptorEntry *fds = malloc(sizeof(struct FileDescriptorEntry) * SB.num_files);
    fread(fds, sizeof(struct FileDescriptorEntry), SB.num_files, file);
    printf("\n%d Files in %s \n", SB.num_files, filename);
    for (int i = 0; i < SB.num_files; i++)
    {
        printf("\tFile name: %s, First Block num: %d, File Size: %d, Modified: %s\n", fds[i].file_name, fds[i].first_block_num, fds[i].size_file, fds[i].modified);
    }
    free((void *)fds);

    fclose(file);

    return;
}

void printfs()
{
    printf(" \n Super Block: \n \t Num_files : %d \n \t Num_blocks: %d \n \t size_blocks: %d \n ", SuperBlock.num_files, SuperBlock.num_blocks, SuperBlock.size_block);
    printf("\n File Descriptors \n");
    for (int i = 0; i < MAX_FD_ENTRY; i++)
    {
        printf("\t[%d] File name: %s, First Block num: %d, File Size: %d, Modified: %s\n", i, FileDescriptors[i].file_name, FileDescriptors[i].first_block_num, FileDescriptors[i].size_file, FileDescriptors[i].modified);
    }
    printf("\n Data Blocks \n");
    for (int i = 0; i < SuperBlock.num_blocks; i++)
    {
        printf("\tBlock Num: %d, Next Block num: %d\n", i, DataBlocks[i].next_block);
    }

    return;
}

int mymkfs(int size, char *filename)
{
    // printf("size of super block %lu,\n size of FD %lu,\n size of datab %lu",sizeof(SuperBlock),sizeof(FileDescriptorEntry),sizeof(DataBlock)) ;
    int datablock_size = (int)(sizeof(struct DataBlock));
    int metadata_size = (int)(sizeof(SuperBlock) + sizeof(struct FileDescriptorEntry) * MAX_FD_ENTRY);
    if (size < (metadata_size + datablock_size))
    {
        printf("Size of file: %d, Minimum size of file %d", size, metadata_size + datablock_size);
        return ERROR;
    }
    SuperBlock.num_files = 0;
    SuperBlock.num_blocks = (size - metadata_size) / datablock_size;
    SuperBlock.size_block = datablock_size;

    FileDescriptors = malloc(sizeof(struct FileDescriptorEntry) * MAX_FD_ENTRY);
    for (int i = 0; i < MAX_FD_ENTRY; i++)
    {
        strcpy(FileDescriptors[i].file_name, "Empty");
        FileDescriptors[i].first_block_num = -1;
        FileDescriptors[i].size_file = 0;
        strcpy(FileDescriptors[i].modified, "00:00");
    }

    DataBlocks = malloc(sizeof(struct DataBlock) * SuperBlock.num_blocks);
    for (int i = 0; i < SuperBlock.num_blocks; i++)
    {
        strcpy(DataBlocks[i].data, "NULL");
        DataBlocks[i].next_block = -1;
    }

    if (myputfs(filename) == ERROR)
    {
        printf("\n Error in creating and writing file system\n");
    }

    printfs();

    free((void *)FileDescriptors);
    free((void *)DataBlocks);

    return OK;
}

int myopen(char *source, char *filename)
{
    if (myreadfs(source) == ERROR)
    {
        printf("/nerror in fetching from file/n");
        return ERROR;
    }

    int index = ERROR;
    // Search filename in FD -> if found set index
    for (int i = 0; i < SuperBlock.num_files; i++)
    {
        // printf("\n comp %s %s %d",filename,FileDescriptors[i].filename, strcmp(filename, FileDescriptors[i].file_name));
        if (strcmp(filename, FileDescriptors[i].file_name) == 0)
        {
            index = i;
            break;
        }
    }

    // else:
    if (index == ERROR)
    {
        // if not found & FD not full -> clame a new FDe store index  / else retrun error
        if (SuperBlock.num_files < MAX_FD_ENTRY)
        {
            for (int i = 0; i < MAX_FD_ENTRY; i++)
            {
                if (FileDescriptors[i].first_block_num == -1)
                {
                    // search for empty data block -> connect FDe to DB / else return error
                    for (int j = 0; j < SuperBlock.num_blocks; j++)
                    {
                        if (DataBlocks[j].next_block == -1)
                        {
                            strcpy(FileDescriptors[i].file_name, filename);
                            getCurrentTimeAsString(FileDescriptors[i].modified);
                            FileDescriptors[i].first_block_num = j;
                            DataBlocks[j].next_block = -2;
                            SuperBlock.num_files ++;
                            index = i;
                            break;
                        }
                    }
                    break;
                }
            }

        }
    }

    if (myputfs(source) == ERROR)
    {
        printf("/nerror in putting to dest/n");
        return ERROR;
    }
    free((void *)FileDescriptors);
    free((void *)DataBlocks);

    return index;
}

// int main()
// {
//     // int i = mymkfs ( 10000, "file1" ) ;
//     // printf ( "\nmkfs status %d" , i ) ;

//     // mydir("file1");
//     // mymount("file1", "aa/bb/cc");
//     // int fileno = myopen ("file1","abcd1");
//     // myreadfs("file1");
//     // printfs();
//     // printf("\n file no : %d",fileno);
//     // free((void *)FileDescriptors);
//     // free((void *)DataBlocks);
// }

int main() {
    int choice;
    char filename[100], destination[100];
    int size,loop=1;

    while(loop){
        printf("Choose a function to call:\n");
        printf("1. myreadfs\n");
        printf("2. mymount\n");
        printf("3. mydir\n");
        printf("4. printfs\n");
        printf("5. mymkfs\n");
        printf("6. myopen\n");

        scanf("%d", &choice);
        switch (choice) {
            case 1:
                printf("Enter filename: ");
                scanf("%s", filename);
                myreadfs(filename);
                break;
            case 2:
                printf("Enter filename: ");
                scanf("%s", filename);
                printf("Enter destination: ");
                scanf("%s", destination);
                mymount(filename, destination);
                break;
            case 3:
                printf("Enter filename: ");
                scanf("%s", filename);
                mydir(filename);
                break;
            case 4:
                printfs();
                break;
            case 5:
                printf("Enter size: ");
                scanf("%d", &size);
                printf("Enter filename: ");
                scanf("%s", filename);
                mymkfs(size, filename);
                break;
            case 6:
                printf("Enter source: ");
                scanf("%s", filename);
                printf("Enter filename: ");
                scanf("%s", destination);
                int fileno = myopen(filename, destination);
                printf("\n File No : %d\n",fileno);
                break;
            case 10:
                loop = 0;
                break;
            default:
                printf("Invalid choice!\n");
                break;
        }
    }   
    return 0;

}
