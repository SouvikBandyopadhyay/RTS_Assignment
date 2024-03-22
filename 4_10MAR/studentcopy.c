
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

struct student
{
    int roll;
    char fname[50];
    char mname[50];
    char sname[50];
    char desc[2000];
};

struct student create_student()
{
    struct student s;

    printf("\nEnter roll number: ");
    scanf("%d", &s.roll);

    printf("Enter first name: ");
    scanf("%s", s.fname);

    printf("Enter middle name: ");
    scanf("%s", s.mname);

    printf("Enter surname: ");
    scanf("%s", s.sname);
    getchar();
    printf("Enter description: ");
    scanf("%[^\n]s", s.desc);

    return s;
}

void add_student(int slot, struct student s, FILE *data_file, FILE *index_file)
{
    int first_int = slot - 1;
    if (first_int < 0)
    {
        lseek(index_file->_fileno, 0, SEEK_SET);
        read(index_file->_fileno, &first_int, sizeof(int));
    }

    off_t offset = lseek(data_file->_fileno, 0, SEEK_END);
    lseek(index_file->_fileno, sizeof(int) + (first_int * sizeof(off_t)), SEEK_SET);
    write(index_file->_fileno, &offset, sizeof(off_t));

    lseek(data_file->_fileno, 0, SEEK_END);
    write(data_file->_fileno, &s.roll, sizeof(s.roll));
    write(data_file->_fileno, s.fname, sizeof(char) * 50);
    write(data_file->_fileno, s.mname, sizeof(char) * 50);
    write(data_file->_fileno, s.sname, sizeof(char) * 50);
    write(data_file->_fileno, s.desc, sizeof(char) * 2000);
    write(data_file->_fileno, "\n", sizeof(char));
    write(data_file->_fileno, "\0", sizeof(char));

    if (slot < 0)
    {
        first_int = first_int + 1;
        lseek(index_file->_fileno, 0, SEEK_SET);
        write(index_file->_fileno, &first_int, sizeof(first_int));
        lseek(data_file->_fileno, 0, SEEK_SET);
        write(data_file->_fileno, &first_int, sizeof(first_int));
    }

    lseek(index_file->_fileno, 0, SEEK_SET);
    read(index_file->_fileno, &first_int, sizeof(int));
}

struct student *search_student_by_index(struct student *s, int i, FILE *data_file, FILE *index_file)
{
    off_t offset = (off_t)(sizeof(int) + (i - 1) * sizeof(off_t));
    lseek(index_file->_fileno, offset, SEEK_SET);
    off_t pointer_to_block;
    read(index_file->_fileno, &pointer_to_block, sizeof(off_t));

    lseek(data_file->_fileno, pointer_to_block, SEEK_SET);
    int roll;
    read(data_file->_fileno, &s->roll, sizeof(int));
    read(data_file->_fileno, s->fname, sizeof(char) * 50);
    read(data_file->_fileno, s->mname, sizeof(char) * 50);
    read(data_file->_fileno, s->sname, sizeof(char) * 50);
    read(data_file->_fileno, s->desc, sizeof(char) * 2000);
    return s;
}

void search_student_by_roll(int roll, FILE *data_file, FILE *index_file)
{
    struct student s1;
    lseek(index_file->_fileno, 0, SEEK_SET);
    int no_of_students;
    read(index_file->_fileno, &no_of_students, sizeof(int));
    for (int i = 1; i <= no_of_students; i++)
    {
        struct student s1;

        search_student_by_index(&s1, i, data_file, index_file);
        if (s1.roll == roll)
        {
            printf("\nStudent Record by Roll: \n");
            printf("Roll: %d\n", s1.roll);
            printf("First Name: %s\n", s1.fname);
            printf("Middle Name: %s\n", s1.mname);
            printf("Surname: %s\n", s1.sname);
            printf("Description: %s\n", s1.desc);
            break;
        }
    }
}

void modify_student_by_roll(int choice, int roll, FILE *data_file, FILE *index_file)
{
    struct student s1;
    lseek(index_file->_fileno, 0, SEEK_SET);
    int no_of_students;
    read(index_file->_fileno, &no_of_students, sizeof(int));
    for (int i = 1; i <= no_of_students; i++)
    {
        struct student s1;

        search_student_by_index(&s1, i, data_file, index_file);
        if (s1.roll == roll)
        {
            int choice;
            printf("Enter your choice \n 1 - roll,\n 2 - fname,\n 3 - mname,\n 4 - sname,\n or 5 - desc\n ");
            scanf("%d", &choice);

            switch (choice)
            {
            case 1:
            {
                int num;
                printf("Enter an integer: ");
                scanf("%d", &num);
                printf("You entered: %d\n", num);
                s1.roll = num;
                break;
            }
            case 2:
            {
                char str[50];
                printf("Enter a string (up to 50 characters): ");
                scanf(" %s", str);
                printf("You entered: %s\n", str);
                strcpy(s1.fname, str);
                break;
            }
            case 3:
            {
                char str[50];
                printf("Enter a string (up to 50 characters): ");
                scanf(" %s", str);
                printf("You entered: %s\n", str);
                strcpy(s1.mname, str);
                break;
            }
            case 4:
            {
                char str[50];
                printf("Enter a string (up to 50 characters): ");
                scanf(" %s", str);
                printf("You entered: %s\n", str);
                strcpy(s1.sname, str);
                break;
            }
            case 5:
            {
                int in;
                printf("\nenter desc: ");
                scanf(" %[^\n]s", s1.desc);
                break;
            }
            default:
                printf("Invalid choice\n");
            }
            break;
            add_student(i, s1, data_file, index_file);
        }
    }
}

void deleteStudent_by_roll(int roll, FILE *data_file, FILE *index_file)
{
    lseek(index_file->_fileno, 0, SEEK_SET);
    int no_of_students;
    read(index_file->_fileno, &no_of_students, sizeof(int));
    int flag = 0;

    FILE *tempindex = fopen("temp1.index", "w");
    if (tempindex == NULL)
    {
        printf("Error opening temp1.index file.\n");
        return;
    }
    FILE *tempdata = fopen("temp2.data", "w");
    if (tempdata == NULL)
    {
        printf("Error opening temp2.data file.\n");
        return;
    }
    close(tempindex->_fileno);
    close(tempdata->_fileno);
    tempindex = fopen("temp1.index", "r+");
    if (tempindex == NULL)
    {
        printf("Error opening temp1.index file.\n");
        return;
    }
    tempdata = fopen("temp2.data", "r+");
    if (tempdata == NULL)
    {
        printf("Error opening temp2.data file.\n");
        return;
    }
    for (int i = 1; i <= no_of_students; i++)
    {

        struct student s1;

        search_student_by_index(&s1, i, data_file, index_file);
        if (s1.roll != roll)
        {
            add_student(-1, s1, tempdata, tempindex);
        }
    }
    close(data_file->_fileno);
    close(index_file->_fileno);

    remove("student.index");
    remove("student.data");

    char old_name1[] = "temp1.index";
    char new_name1[] = "student.index";

    if (rename(old_name1, new_name1) == 0)
    {
        printf("File renamed successfully.\n");
    }
    else
    {
        perror("Error renaming file");
    }
    char old_name2[] = "temp2.data";
    char new_name2[] = "student.data";

    if (rename(old_name2, new_name2) == 0)
    {
        printf("File renamed successfully.\n");
    }
    else
    {
        perror("Error renaming file");
    }
    lseek(tempindex->_fileno, 0, SEEK_SET);
    read(tempindex->_fileno, &no_of_students, sizeof(int));
    lseek(tempdata->_fileno, 0, SEEK_SET);
    read(tempdata->_fileno, &no_of_students, sizeof(int));
    close(tempindex->_fileno);
    close(tempdata->_fileno);
}

int main()
{
    int choice;
    FILE *index_file = fopen("student.index", "r+");
    if (index_file == NULL)
    {
        return 0;
    }
    FILE *data_file = fopen("student.data", "r+");
    if (data_file == NULL)
    {
        return 0;
    }

    int index = 0;
    lseek(index_file->_fileno, 0, SEEK_SET);
    write(index_file->_fileno, &index, sizeof(index));
    lseek(data_file->_fileno, 0, SEEK_SET);
    write(data_file->_fileno, &index, sizeof(index));
    
    while (1)
    {
        printf("\n");
        printf("Enter your choice (1-7):\n");
        printf("1. Create student\n");
        printf("2. Search student by index\n");
        printf("3. Search student by roll\n");
        printf("4. Delete student\n");
        printf("5. Modify student\n");
        printf("Enter 0 to exit\n");
        scanf("%d", &choice);
        int search_roll;
        switch (choice)
        {
        case 1:
            struct student s = create_student();
            add_student(-1, s, data_file, index_file);
            break;
        case 2:
            struct student s1;
            int search_index;

            printf("\nSearch by index: ");
            scanf("%d", &search_index);
            
            search_student_by_index(&s1, search_index, data_file, index_file);

            printf("Roll: %d\n", s1.roll);
            printf("First Name: %s\n", s1.fname);
            printf("Middle Name: %s\n", s1.mname);
            printf("Surname: %s\n", s1.sname);
            printf("Description: %s\n", s1.desc);
            
            break;
        case 3:
            printf("\nSearch by Roll: ");
            scanf("%d", &search_roll);
            search_student_by_roll(search_roll, data_file, index_file);

            break;
        case 4:
            printf("\nDelete by Roll: ");
            scanf("%d", &search_roll);
            deleteStudent_by_roll(search_roll, data_file, index_file);
            index_file = fopen("student.index", "r+");
            if (index_file == NULL)
            {
                printf("Error opening student.index file.\n");
                return 0;
            }
            data_file = fopen("student.data", "r+");
            if (data_file == NULL)
            {
                printf("Error opening student.data file.\n");
                return 0;
            }
            lseek(index_file->_fileno, 0, SEEK_SET);
            write(index_file->_fileno, &index, sizeof(index));
            lseek(data_file->_fileno, 0, SEEK_SET);
            write(data_file->_fileno, &index, sizeof(index));
            break;
        case 5:
            printf("\nModify by Roll: ");
            scanf("%d", &search_roll);
            modify_student_by_roll(choice, search_roll, data_file, index_file);
            break;
        case 0:
            printf("Exiting program.\n");
            close(data_file->_fileno);
            close(index_file->_fileno);
            return 0;
        default:
            printf("\nInvalid choice. Please enter a number between 0 and 5.\n");
        }
    }
    close(data_file->_fileno);
    close(index_file->_fileno);
    return 0;
}