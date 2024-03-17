#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_DESC 2000

// Define a struct for student record
struct student {
    int roll;               // storing the roll number of the student
    char fname[50];         // storing the first name of the student
    char mname[50];         // storing the middle name of the student
    char sname[50];         // storing the surname of the student
    char *desc;             // storing description of the student
};

// const char* FORMAT_OUT = "(%d, %s, %s, %s, %s)\n"; 

// Function to create a student struct with user input
struct student create_student() {
    struct student s;

    // Input roll number
    printf("\nEnter roll number: ");
    scanf("%d", &s.roll);

    // Input first name
    printf("Enter first name: ");
    scanf("%s", s.fname);

    // Input middle name
    printf("Enter middle name: ");
    scanf("%s", s.mname);

    // Input surname
    printf("Enter surname: ");
    scanf("%s", s.sname);
    getchar();
    // Input description
    printf("Enter description: ");
    char tempDesc[MAX_DESC];
    scanf("%[^\n]s",tempDesc);
    strcat(tempDesc,"\0");
    // Dynamically allocate memory for description based on input size
    s.desc = (char *)malloc((strlen(tempDesc) + 1) * sizeof(char));
    strcpy(s.desc, tempDesc);

    return s;
}


void add_student(int slot, struct student s,FILE *data_file, FILE *index_file){
    int first_int=slot-1;
    if (first_int<0)
    {
        // read current no of students
        lseek(index_file->_fileno, 0, SEEK_SET);
        read(index_file->_fileno, &first_int, sizeof(int));
        printf("first int:%d",first_int);
    }
    

    // after the last entry in index file, filesize of .data file is added
    off_t offset=lseek(data_file->_fileno, 0, SEEK_END);
    lseek(index_file->_fileno,sizeof(int)+(first_int*sizeof(off_t)),SEEK_SET);
    write(index_file->_fileno,&offset, sizeof(off_t));
    printf("\n calculated offset %ld\n",offset);

    // at the end of data file add a student info
    lseek(data_file->_fileno, 0, SEEK_END);
    write(data_file->_fileno,&s.roll, sizeof(s.roll));
    write(data_file->_fileno,s.fname, sizeof(char)*50);
    write(data_file->_fileno,s.mname, sizeof(char)*50);
    write(data_file->_fileno,s.sname, sizeof(char)*50);
    write(data_file->_fileno,s.desc, sizeof(char)*strlen(s.desc));
    write(data_file->_fileno,"\n", sizeof(char));
    write(data_file->_fileno,"\0", sizeof(char));


    int item;
    lseek(index_file->_fileno, sizeof(int)+sizeof(off_t)*first_int, SEEK_SET);
    read(index_file->_fileno, &item, sizeof(int));
    printf("\nindex entry:%d",item);
    lseek(data_file->_fileno, offset, SEEK_SET);
    read(data_file->_fileno, &item, sizeof(int));
    printf("\nroll:%d",item);
    char tempstr[MAX_DESC];
    read(data_file->_fileno, &tempstr, sizeof(char)*50);
    printf("\nfname:%s",tempstr);
    read(data_file->_fileno, &tempstr, sizeof(char)*50);
    printf("\nmname:%s",tempstr);
    read(data_file->_fileno, &tempstr, sizeof(char)*50);
    printf("\nsname:%s",tempstr);

    int j=0;
    char ch=fgetc(data_file);
    while ((ch != '\n') && (ch != EOF) && (ch != '\0')) {
        tempstr[j]=ch;
        j++;
        // printf("\n char from file :%c", ch); // Print character or do whatever processing you need
        ch=fgetc(data_file);
    }
    tempstr[j]='\0';

    printf("\ndesc:%s",tempstr);

    if (slot<0)
    {
        first_int=first_int+1;
        lseek(index_file->_fileno, 0, SEEK_SET);
        write(index_file->_fileno,&first_int, sizeof(first_int));
        lseek(data_file->_fileno, 0, SEEK_SET);
        write(data_file->_fileno,&first_int, sizeof(first_int));
        printf("\n slot by default %d\n",slot);
    }
    
    lseek(index_file->_fileno, 0, SEEK_SET);
    read(index_file->_fileno, &first_int, sizeof(int));
    printf("\nafter update first int:%d",first_int);
    

}

struct student *search_student_by_index(struct student *s,int i, FILE *data_file, FILE *index_file){
    lseek(index_file->_fileno,0,SEEK_SET);
    int no_of_stud;
    read(index_file->_fileno, &no_of_stud, sizeof(int));
    printf("\namong %d students in storage, searching for %d\n",no_of_stud,i);
    if (i>no_of_stud)
    {
        printf("\nNot suffucient students\n");
        return s;
    }
    
    // get offset of block from index file
    off_t offset=(off_t)(sizeof(int)+(i-1)*sizeof(off_t));
    lseek(index_file->_fileno,offset,SEEK_SET);
    off_t pointer_to_block;
    read(index_file->_fileno, &pointer_to_block, sizeof(off_t));
    printf("\n pointer val %ld\n",pointer_to_block);

    // get block from data file
    printf("\n end of data file = %ld.. trying to go : %ld\n",lseek(data_file->_fileno,0,SEEK_END),pointer_to_block);
    //
    lseek(data_file->_fileno,pointer_to_block,SEEK_SET);
    printf("shifted pointer");
    int roll;
    read(data_file->_fileno,&s->roll,sizeof(int));
    read(data_file->_fileno,s->fname,sizeof(char)*50);
    read(data_file->_fileno,s->mname,sizeof(char)*50);
    read(data_file->_fileno,s->sname,sizeof(char)*50);
    // off_t currpointer=lseek(data_file->_fileno,0,SEEK_CUR);
    // off_t fileend=lseek(data_file->_fileno,0,SEEK_END);
    // lseek(data_file->_fileno,currpointer,SEEK_SET);
    // off_t maxlen=fileend-currpointer;
    // printf("\nmaxlen=%ld\n",maxlen);

    char desc[MAX_DESC];
    int j=0;
    char ch[1];
    read(data_file->_fileno,ch,sizeof(char));
    // printf("\nfirst char from file :%c", ch[0]); // Print character or do whatever processing you need
    while ((ch[0] != '\n') && (ch[0] != EOF) && (ch[0] != '\0') && (j<15)) {
        desc[j]=ch[0];
        j++;
        // printf("\n char from file :%c", ch[0]); // Print character or do whatever processing you need
        read(data_file->_fileno,ch,sizeof(char));
    }
    desc[j]='\0';

    printf("\ndesc :%s of len:%ld\n",desc,strlen(desc));
    s->desc=(char *)malloc((strlen(desc) + 1) * sizeof(char));
    strcpy(s->desc,desc);
    // if (fgets(desc, MAX_DESC, data_file) != NULL) {
        
    //         printf("\ndesc: %s\n",desc);
    // } else {
    //     printf("Error reading from file\n");
    // }

    // int j=0;
    // char ch=fgetc(data_file);
    // while ((ch != '\n') && (ch != EOF) && (ch != '\0')) {
    //     desc[j]=ch;
    //     j++;
    //     printf("%c", ch); // Print character or do whatever processing you need
    //     ch=fgetc(data_file);
    // }
    // s->desc = (char *)malloc((j + 1) * sizeof(char));
    // strcpy(s->desc,desc);
    // printf("\n desc in s %s\n",s->desc);

    // if (fgets(desc, maxlen, data_file) != NULL) {
        
    //         printf("\n desc %s\n",desc);
    //         s->desc = (char *)malloc((strlen(desc) + 1) * sizeof(char));
    //         strcpy(s->desc,desc);
    // } else {
    //     printf("Error reading from file\n");
    // }
    // printf("\n desc in s %s\n",s->desc);
    return s;
} 

void search_student_by_roll(int roll, FILE *data_file, FILE *index_file){
    struct student s1;
    lseek(index_file->_fileno,0,SEEK_SET);
    int no_of_students;
    read(index_file->_fileno,&no_of_students,sizeof(int));
    for (int i = 1; i <= no_of_students; i++)
    {
                struct student s1;
                
                search_student_by_index(&s1,i,data_file,index_file);
                if (s1.roll==roll)
                {
                    // Display the student record 
                    printf("\n**************************************\n\n");
                    printf("\nStudent Record by Roll: \n");
                    printf("Roll: %d\n", s1.roll);
                    printf("First Name: %s\n", s1.fname);
                    printf("Middle Name: %s\n", s1.mname);
                    printf("Surname: %s\n", s1.sname);
                    printf("Description: %s\n", s1.desc);
                    printf("INDEX: %d\n", i);
                    printf("\n\n**************************************\n");
                    free(s1.desc);
                    break;
                }
    }
}

// int main() {


//     // Open student.index file
//     FILE *index_file = fopen("student.index", "r+");
//     if (index_file == NULL) {
//         printf("Error opening student.index file.\n");
//         return 0;
//     }
//     // Open student.data file for appending
//     FILE *data_file = fopen("student.data", "r+");
//     if (data_file == NULL) {
//         printf("Error opening student.data file.\n");
//         return 0;
//     }
//     int index=0;
//      // Update integer in files
//     lseek(index_file->_fileno, 0, SEEK_SET);
//     write(index_file->_fileno,&index, sizeof(index));
//     lseek(data_file->_fileno, 0, SEEK_SET);
//     write(data_file->_fileno,&index, sizeof(index));

//     int first_int;
//     lseek(index_file->_fileno, 0, SEEK_SET);
//     lseek(data_file->_fileno, 0, SEEK_SET);
//     read(index_file->_fileno, &first_int, sizeof(int));
//     printf("\ngot index %d\n",first_int);
//     read(data_file->_fileno, &first_int, sizeof(int));
//     printf("\ngot index %d\n",first_int);



//     // Create a student struct using the create_student() function
//     struct student s = create_student();
//     add_student(s,data_file,index_file);
//     free(s.desc);
//     // s = create_student();
//     // add_student(s,data_file,index_file);
//     // free(s.desc);
//     struct student s1;
//     int search_index,search_roll;

//     printf("\nSearch by index");
//     scanf("%d",&search_index);
//     search_student_by_index(&s1,search_index,data_file,index_file);

//     // Display the student record
//     printf("\nStudent Record by index:\n");
//     printf("Roll: %d\n", s1.roll);
//     printf("First Name: %s\n", s1.fname);
//     printf("Middle Name: %s\n", s1.mname);
//     printf("Surname: %s\n", s1.sname);
//     printf("Description: %s\n", s1.desc);

//     // printf("\nSearch by Roll");
//     // scanf("%d",&search_roll);
//     // struct student s2=search_student_by_roll(search_roll,data_file,index_file);
    
//     // printf("\nStudent Record by roll:\n");
//     // printf("Roll: %d\n", s2.roll);
//     // printf("First Name: %s\n", s2.fname);
//     // printf("Middle Name: %s\n", s2.mname);
//     // printf("Surname: %s\n", s2.sname);
//     // printf("Description: %s\n", s2.desc);


//     close(data_file->_fileno);
//     close(index_file->_fileno);
//     // Free dynamically allocated memory for description
    

//     return 0;
// }
void initialise_file(FILE *data_file, FILE *index_file){
    int index=0;
     // Update integer in files
    lseek(index_file->_fileno, 0, SEEK_SET);
    write(index_file->_fileno,&index, sizeof(index));
    lseek(data_file->_fileno, 0, SEEK_SET);
    write(data_file->_fileno,&index, sizeof(index));

    int first_int;
    lseek(index_file->_fileno, 0, SEEK_SET);
    lseek(data_file->_fileno, 0, SEEK_SET);
    read(index_file->_fileno, &first_int, sizeof(int));
    printf("\ngot index %d\n",first_int);
    read(data_file->_fileno, &first_int, sizeof(int));
    printf("\ngot index %d\n",first_int);

}

void modify_student_by_roll(int choice, int roll,FILE *data_file, FILE *index_file){
    struct student s1;
    lseek(index_file->_fileno,0,SEEK_SET);
    int no_of_students;
    read(index_file->_fileno,&no_of_students,sizeof(int));
    for (int i = 1; i <= no_of_students; i++)
    {
                struct student s1;
                
                search_student_by_index(&s1,i,data_file,index_file);
                if (s1.roll==roll)
                {
                    int choice;
                    printf("Enter your choice \n 1 - roll,\n 2 - fname,\n 3 - mname,\n 4 - sname,\n or 5 - desc\n ");
                    scanf("%d", &choice);

                    switch(choice) {
                        case 1: {
                            int num;
                            printf("Enter an integer: ");
                            scanf("%d", &num);
                            printf("You entered: %d\n", num);
                            s1.roll=num;
                            break;
                        }
                        case 2: {
                            char str[50];
                            printf("Enter a string (up to 50 characters): ");
                            scanf(" %s", str); // Read up to 49 characters, avoiding buffer overflow
                            printf("You entered: %s\n", str);
                            strcpy(s1.fname,str); 
                            break;
                        }
                        case 3: {
                            char str[50];
                            printf("Enter a string (up to 50 characters): ");
                            scanf(" %s", str); // Read up to 49 characters, avoiding buffer overflow
                            printf("You entered: %s\n", str);
                            strcpy(s1.mname,str); 
                            break;
                        }
                        case 4: {
                            char str[50];
                            printf("Enter a string (up to 50 characters): ");
                            scanf(" %s", str); // Read up to 49 characters, avoiding buffer overflow
                            printf("You entered: %s\n", str);
                            strcpy(s1.sname,str); 
                            break;
                        }
                        case 5: {
                            int in;
                            printf("\nenter desc: ");
                            char tempDesc[MAX_DESC];
                            scanf(" %[^\n]s",tempDesc);
                            strcat(tempDesc,"\0");
                            // Dynamically allocate memory for description based on input size
                            s1.desc = (char *)malloc((strlen(tempDesc) + 1) * sizeof(char));
                            strcpy(s1.desc,tempDesc); 
                            break;
                        }
                        default:
                            printf("Invalid choice\n");
                    }
                    // Display the student record 
                    printf("\nStudent Record by Roll: \n");
                    printf("Roll: %d\n", s1.roll);
                    printf("First Name: %s\n", s1.fname);
                    printf("Middle Name: %s\n", s1.mname);
                    printf("Surname: %s\n", s1.sname);
                    printf("Description: %s\n", s1.desc);
                    add_student(i,s1,data_file,index_file);
                    free(s1.desc);
                    break;
                }
    }
}


void deleteStudent_by_roll(int roll,FILE *data_file, FILE *index_file)
{
    lseek(index_file->_fileno,0,SEEK_SET);
    int no_of_students;
    read(index_file->_fileno,&no_of_students,sizeof(int));
    printf("\nbefore deleting %d students\n",no_of_students);
    int flag=0;

    // Open student.index file
    FILE *tempindex = fopen("temp1.index", "w");
    if (tempindex == NULL) {
        printf("Error opening temp1.index file.\n");
        return ;
    }
    // Open student.data file for appending
    FILE *tempdata = fopen("temp2.data", "w");
    if (tempdata == NULL) {
        printf("Error opening temp2.data file.\n");
        return ;
    }
    close(tempindex->_fileno);
    close(tempdata->_fileno);
    tempindex = fopen("temp1.index", "r+");
    if (tempindex == NULL) {
        printf("Error opening temp1.index file.\n");
        return ;
    }
    // Open student.data file for appending
    tempdata = fopen("temp2.data", "r+");
    if (tempdata == NULL) {
        printf("Error opening temp2.data file.\n");
        return ;
    }

    initialise_file(tempdata,tempindex);
    for (int i = 1; i <= no_of_students; i++)
    {

        struct student s1;
        
        search_student_by_index(&s1,i,data_file,index_file);
        if (s1.roll!=roll){
            add_student(-1,s1,tempdata,tempindex);
        }
    }
    close(data_file->_fileno);
    close(index_file->_fileno);

    remove("student.index");
    remove("student.data");

    char old_name1[] = "temp1.index";
    char new_name1[] = "student.index";

    if (rename(old_name1, new_name1) == 0) {
        printf("File renamed successfully.\n");
    } else {
        perror("Error renaming file");
    }
    char old_name2[] = "temp2.data";
    char new_name2[] = "student.data";

    if (rename(old_name2, new_name2) == 0) {
        printf("File renamed successfully.\n");
    } else {
        perror("Error renaming file");
    }
    lseek(tempindex->_fileno,0,SEEK_SET);
    read(tempindex->_fileno,&no_of_students,sizeof(int));
    lseek(tempdata->_fileno,0,SEEK_SET);
    read(tempdata->_fileno,&no_of_students,sizeof(int));
    printf("\nAfter deleting %d students\n",no_of_students); 
    close(tempindex->_fileno);
    close(tempdata->_fileno);
}

void compress_files(FILE *data_file, FILE *index_file){
    lseek(index_file->_fileno,0,SEEK_SET);
    int no_of_students;
    read(index_file->_fileno,&no_of_students,sizeof(int));
    int flag=0;

    // Open student.index file
    FILE *tempindex = fopen("temp1.index", "w");
    if (tempindex == NULL) {
        printf("Error opening temp1.index file.\n");
        return ;
    }
    // Open student.data file for appending
    FILE *tempdata = fopen("temp2.data", "w");
    if (tempdata == NULL) {
        printf("Error opening temp2.data file.\n");
        return ;
    }
    close(tempindex->_fileno);
    close(tempdata->_fileno);
    tempindex = fopen("temp1.index", "r+");
    if (tempindex == NULL) {
        printf("Error opening temp1.index file.\n");
        return ;
    }
    // Open student.data file for appending
    tempdata = fopen("temp2.data", "r+");
    if (tempdata == NULL) {
        printf("Error opening temp2.data file.\n");
        return ;
    }

    initialise_file(tempdata,tempindex);
    for (int i = 1; i <= no_of_students; i++)
    {

        struct student s1;
        
        search_student_by_index(&s1,i,data_file,index_file);
        add_student(-1,s1,tempdata,tempindex);
    }
    close(data_file->_fileno);
    close(index_file->_fileno);

    remove("student.index");
    remove("student.data");

    char old_name1[] = "temp1.index";
    char new_name1[] = "student.index";

    if (rename(old_name1, new_name1) == 0) {
        printf("File renamed successfully.\n");
    } else {
        perror("Error renaming file");
    }
    char old_name2[] = "temp2.data";
    char new_name2[] = "student.data";

    if (rename(old_name2, new_name2) == 0) {
        printf("File renamed successfully.\n");
    } else {
        perror("Error renaming file");
    }
    lseek(index_file->_fileno,0,SEEK_SET);
    read(index_file->_fileno,&no_of_students,sizeof(int));
    close(tempindex->_fileno);
    close(tempdata->_fileno);
}

// void deleteStudent_by_roll(int roll,FILE *data_file, FILE *index_file)
// {
//     lseek(index_file->_fileno,0,SEEK_SET);
//     int no_of_students;
//     read(index_file->_fileno,&no_of_students,sizeof(int));
//     printf("\nbefore deleting %d students\n",no_of_students);
//     int flag=0;
//     for (int i = 1; i <= no_of_students; i++)
//     {
//                 if (flag)
//                 {
//                     off_t todel_offset=sizeof(int)+((i)*(sizeof(off_t)));
//                     lseek(index_file->_fileno,todel_offset,SEEK_SET);
//                     off_t next_offset;
//                     read(index_file->_fileno,&next_offset,sizeof(off_t));
                    
//                     off_t toshift_offset=sizeof(int)+((i-1)*(sizeof(off_t)));
//                     lseek(index_file->_fileno,todel_offset,SEEK_SET);
//                     write(index_file->_fileno,&next_offset,sizeof(off_t));
//                 }
//                 else{

//                     struct student s1;
                    
//                     search_student_by_index(&s1,i,data_file,index_file);
//                     if (s1.roll==roll)
//                     {
//                         flag=1;
//                         no_of_students=no_of_students-1;
//                         lseek(index_file->_fileno,0,SEEK_SET);
//                         write(index_file->_fileno,&no_of_students,sizeof(int));
//                     }
//                 }
//     }
//     lseek(index_file->_fileno,0,SEEK_SET);
//     read(index_file->_fileno,&no_of_students,sizeof(int));
//     printf("\nAfter deleting %d students\n",no_of_students); 
// }


int main() {
    int choice;
    FILE *index_file = fopen("student.index", "r+");
    if (index_file == NULL) {
        printf("Error opening student.index file.\n");
        return 0;
    }
    // Open student.data file for appending
    FILE *data_file = fopen("student.data", "r+");
    if (data_file == NULL) {
        printf("Error opening student.data file.\n");
        return 0;
    }
    
    while (1) {
        printf("\n");
        printf("Enter your choice (1-7):\n");
        printf("1. Initialise file\n");
        printf("2. Create student\n");
        printf("3. Search student by index\n");
        printf("4. Search student by roll\n");
        printf("5. Delete student\n");
        printf("6. Modify student\n");
        printf("7. Compress file\n");
        printf("Enter 0 to exit\n");
        scanf("%d", &choice);
        int search_roll;
        switch (choice) {
            case 1:
                initialise_file(data_file,index_file);
                break;
            case 2:
                struct student s = create_student();
                add_student(-1,s,data_file,index_file);
                free(s.desc);
                break;
            case 3:
                struct student s1;
                int search_index;

                printf("\nSearch by index: ");
                scanf("%d",&search_index);
                lseek(index_file->_fileno,0,SEEK_SET);
                int no_of_stud;
                read(index_file->_fileno, &no_of_stud, sizeof(int));
                printf("\namong %d students in storage, searching for %d\n",no_of_stud,search_index);
                if (search_index>no_of_stud)
                {
                    printf("\nNot suffucient students\n");
                }
                else{
                    search_student_by_index(&s1,search_index,data_file,index_file);

                    // Display the student record 
                    printf("\n*************************************\n\nStudent Record by index: \n");
                    printf("Roll: %d\n", s1.roll);
                    printf("First Name: %s\n", s1.fname);
                    printf("Middle Name: %s\n", s1.mname);
                    printf("Surname: %s\n", s1.sname);
                    printf("Description: %s\n", s1.desc);
                    printf("\n\n**************************************\n");
                    free(s1.desc);
                }
                break;
            case 4:
                printf("\nSearch by Roll: ");
                scanf("%d",&search_roll);
                search_student_by_roll(search_roll,data_file,index_file);
                
                break;
            case 5:
                printf("\nDelete by Roll: ");
                scanf("%d",&search_roll);
                deleteStudent_by_roll(search_roll,data_file,index_file);
                    // Open student.index file
                index_file = fopen("student.index", "r+");
                if (index_file == NULL) {
                    printf("Error opening student.index file.\n");
                    return 0;
                }
                // Open student.data file for appending
                data_file = fopen("student.data", "r+");
                if (data_file == NULL) {
                    printf("Error opening student.data file.\n");
                    return 0;
                }
                break;
            case 6:
            
                printf("\nModify by Roll: ");
                scanf("%d",&search_roll);
                modify_student_by_roll(choice, search_roll, data_file,index_file);
                break;
            case 7:
                compress_files(data_file,index_file);
                break;
            case 0:
                printf("Exiting program.\n");
                close(data_file->_fileno);
                close(index_file->_fileno);
                return 0;
            default:
                printf("\nInvalid choice. Please enter a number between 1 and 8.\n");
        }
    }
    close(data_file->_fileno);
    close(index_file->_fileno);
    return 0;
}