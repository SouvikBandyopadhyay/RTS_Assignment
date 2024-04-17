// ***************************************************************************************************
// * LATEST SOLUTION:
// * The recent bug was happening because manager thread was not joined in main,                     *
// * to solve the problem, I created and joined the manager thread after the producer and consumer.  *
// * more checks are addded in manager funtion to make threads join even after return.               *
// * Every time a thread is killed from manager, the thread's corrosponding condtitional varibale is *
// * woken up, and the condition is modified in such a way that a thread cannot consume or produce   *
// * an item after it has been killed, and must unlock it's mutex for fellow threads of same type    *
// * and the conditional variable and execute pthread_exit() to which manager and main has join      *
// * statements to capture                                                                           *
// ***************************************************************************************************
// _________________________________________________________________________________________________________
// 2 variables current_no_of_producer_threads and current_no_of_consumer_threads are maintained to keep track of how many producer and consumer thread to keep alive
// to kill a thread ... corrosponding variable value is reduced
// a thread will run if its corrosponding thread no. is less than the current (p/c)threadno of that type
// max no of threads is set to 5, initial m=2 and n=3
// some warnings are encoundered during compilation due to type casting although program runns fine
// ==================================================================================================
// ||   MANAGER:
// ||       Enter
// ||       1 to increase producers,
// ||       2 to decrease producers,
// ||       3 to increase consumers,
// ||       4 to decrease consumers,
// ||       0 to exit:
// ||
// ===================================================================================================
// ===============================================================================================
// The current execution works like this:

// 2 arrays of threads are created along with a manager thread

// the manager thread takes 4 inputs on input

// 1: create a P_thread pass P_thread_number as argument and increment the P_thread_number and add to P_thread_array

// 2: decrement the P_thread_number

// 3: create a C_thread pass C_thread_number as argument and increment the C_thread_number and add to C_thread_array

// 4: decrement the C_thread_number

// The producer and consumer threads gets argemtn makes it there thread_ID
// then in an infinite loop checks if there thread_ID is less than the current_number_of_thread_of_that_type
// If true dose its JOB*
// else returns
// Finally unreturned threads are joined