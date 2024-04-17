#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

/* funtion takes file path and name reads current permissssions
 bitwise ads negetive of read permission value of group and others
 ṣets new permission and checks new permissions
 */
void securefile(char *file_path)
{

    // variable to store filepath/name

    char file[2048];
    strcpy(file, file_path);

    // fetching old permissions

    struct stat old_status;
    if (stat(file, &old_status) == -1)
    {
        perror("Error in getting file old_status");
        exit(EXIT_FAILURE);
    }
    printf(" Old Permissions: ");
    printf((old_status.st_mode & S_IRUSR) ? "r" : "-");
    printf((old_status.st_mode & S_IWUSR) ? "w" : "-");
    printf((old_status.st_mode & S_IXUSR) ? "x" : "-");
    printf((old_status.st_mode & S_IRGRP) ? "r" : "-");
    printf((old_status.st_mode & S_IWGRP) ? "w" : "-");
    printf((old_status.st_mode & S_IXGRP) ? "x" : "-");
    printf((old_status.st_mode & S_IROTH) ? "r" : "-");
    printf((old_status.st_mode & S_IWOTH) ? "w" : "-");
    printf((old_status.st_mode & S_IXOTH) ? "x" : "-");

    // Withdrawing Group Read Permission

    old_status.st_mode = old_status.st_mode & ~(S_IRGRP);

    // Withdrawing Other Read Permission

    old_status.st_mode = old_status.st_mode & ~(S_IROTH);

    // setting new permisssions

    chmod(file, old_status.st_mode);

    // variale to store new mode

    struct stat new_status = old_status;

    // chechking new status

    if (stat(file, &new_status) == -1)
    {
        perror("Error in getting file status");
        exit(EXIT_FAILURE);
    }

    printf(" .... NEW Permissions: ");
    printf((new_status.st_mode & S_IRUSR) ? "r" : "-");
    printf((new_status.st_mode & S_IWUSR) ? "w" : "-");
    printf((new_status.st_mode & S_IXUSR) ? "x" : "-");
    printf((new_status.st_mode & S_IRGRP) ? "r" : "-");
    printf((new_status.st_mode & S_IWGRP) ? "w" : "-");
    printf((new_status.st_mode & S_IXGRP) ? "x" : "-");
    printf((new_status.st_mode & S_IROTH) ? "r" : "-");
    printf((new_status.st_mode & S_IWOTH) ? "w" : "-");
    printf((new_status.st_mode & S_IXOTH) ? "x" : "-");
    printf("\n");
    return;
}

/* funtion takes dir name, extension and depth,
 depth indicates the dept at which current file exist from base dir for visualisation purpose
 checks items in dir, if item is file, gets item's extension by pointing to last occourance of "." in name +1,
 if item's extension matches parameter extension, changes the security permissions of the file,
 if item is folder makes a recursive call on the same funtion with the new folder name,
 same extension and increases depth by \t, except for self and parent folder, finally closes dir after operation
  */
void list_files(const char *dir_name, const char *extension, char *depth)
{
    DIR *dir;
    struct dirent *item_in_dir;

    // variable to store "filepath/name"

    char file_path[2048];
    strcpy(file_path, dir_name);

    // Open folder Dir_name in dir

    dir = opendir(dir_name);
    if (dir == NULL)
    {
        perror("Unable to open directory");
        exit(EXIT_FAILURE);
    }

    // loop through items in folder dir_name

    while ((item_in_dir = readdir(dir)) != NULL)
    {

        // variable to store  dir_name/itme_name

        char file_path2[2048];
        strcpy(file_path2, file_path);
        strcat(file_path2, "/");
        strcat(file_path2, item_in_dir->d_name);

        // checks if itmme is folder but not self or parent

        if ((item_in_dir->d_type == 4) && strcmp(item_in_dir->d_name, ".") != 0 && strcmp(item_in_dir->d_name, "..") != 0)
        {
            char depth2[2048] = "";
            strcpy(depth2, depth);
            strcat(depth2, "\t");
            printf("OPENING %s\n", file_path2);

            // recurse with item folder

            list_files(file_path2, extension, depth2);
        }

        // checks if item is file

        else if (item_in_dir->d_type == 8)
        {

            // varibale to store item's externtion

            const char *file_extension = strrchr(item_in_dir->d_name, '.');

            // checks itm's extension with extension

            if ((file_extension != NULL) && (strcmp(file_extension + 1, extension) == 0))
            {
                printf("%sSECURING .... %s ....", depth, item_in_dir->d_name);

                // if extension matches --> secure file

                securefile(file_path2);
            }

            // if extension dont match

            else
            {
                printf("%s%s \n", depth, item_in_dir->d_name);
            }
        }
    }

    closedir(dir);
}

/* main funtion splits the command line arguments to Dir and extension and
 searches for files with matching extension in Dir and subfolders
 */
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <directory_name> <extension>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char *dir_name = argv[1];
    char *extension = argv[2];
    list_files(dir_name, extension, "");
}