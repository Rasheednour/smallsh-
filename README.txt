A shell program written in C that provides a prompt for running shell commands, 
handle blank lines and comment lines (begining with #), and provide expansion for the variable $$.
The program has three built in functions which handle the cd, exit, and status command internally, 
and will use the exec family functions to handle all other shell commands entered by the user. 
The program supports input and output redirection (using < and >) and support running commands in
foreground and background processes. The program also implements custom signal handlers for 2 signals, SIGINT, and SIGTSTP.

----------------------------------------------------------------------------------

To compile and run the program, use the following commands:

gcc --std=gnu99 -o smallsh main.c
./smallsh

Or alternatevely, you can utilize the Makefile by running the following commands to compile and run the program:

make
./smallsh

Optional: To clean the created executable file, run the following command:

make clean
