// The Application can run without manually creating Data and Index file, if files not found they will be created
// The application offeres 8 services
//
// 1: Reset File, Makes the current content of the file unreadable and prepares the files for a fresh start
//
// 2: Create Student: ttakes student details as input from user, Desc can be of variable length and stores in data file
//       and  provides a pointer to the datta in the index file.
//
// 3: Search by Index: calculates position of Data pointer in index file and reads the pointer,
//      then fetches the data from the data file.
//
// 4: Search by Roll: Uses Search by index to got through all data until appt roll no is found.
//
// 5: Delete Student: Coppies data of all student to a new file except the data that needs to be deleted, another
//      approach was used previously (commented ln.no.:573-610) where the pointer to the data was deleted and to maintain index
//      index consistency all the later index where shifted 1 step upwards but last index is not handled apptly.
//
// 6: Modify Student: Creates a new data struct and stores it in data file and pointer in index file is updated.(creates holes)
//
// 7: Compress file: creates a copy of all relivant data to new files and renamed, (works like delete remove holes).
//
// 8: Show all: Displayes all student recods by by traversing through all indexes.
//