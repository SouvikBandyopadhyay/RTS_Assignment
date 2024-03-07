Even though I was unable to reproduce the initial bug, from the same code, 
I only changed line number 157, and 189, from 1 to 0 to make the minimum producer and consumer = 0.

Previously the bug was once the number of producer was reduced to 0,
the producers created thereafter had negetive `ThreadID` (A variable used to check life span of thread). 

But a new bug is encountered, Trying to set the number of threads of all type to 0, results
in unexplained exiting of the program after trying to increase the numbers back up again.
(Sometimes happening by reducing the number of any one type of threadds to 0).


Example input to produce new bug:
<*> --> press enter wait for few seconds

Eg1:  4 <*> 4 <*> 4 <*> 4 <*> 2 <*> 2 <*> 2 <*> 2 <*>

step 1: Reduce the number of thread of all type to 0.

step 2: increase it back up.

step 3: Auto Termination of program (unexplained).

In the Final solution I will change some variable names for better readability, 
and try to fix the problem. 

=============================================================================================== 
The current execution workds like this:

2 arrays of threads are created along with a manager thread

the manager thread takes 4 inputs on input

1: create a P_thread pass P_thread_number as argument and increment the P_thread_number and add to P_thread_array

2: decrement the P_thread_number

3: create a C_thread pass C_thread_number as argument and increment the C_thread_number and add to C_thread_array

4: decrement the C_thread_number

The producer and consumer threads gets argemtn makes it there thread_ID
then in an infinite loop checks if there thread_ID is less than the current_number_of_thread_of_that_type
If true dose its JOB*
else returns

