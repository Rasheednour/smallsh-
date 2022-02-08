// Name: Rasheed Mohammed
// CS 344 - Operating Systems - Winter 2022
// Assignment 3 - smallsh 
/* Description: A shell program written in C that provides a prompt for running shell commands, 
                handle blank lines and comment lines (begining with #), and provide expansion for 
                the variable $$.
                The program has three built in functions which handle the cd, exit, and status command
                internally, and will use the exec family functions to handle all other shell commands
                entered by the user. The program supports input and output redirection (using < and >)
                and support running commands in foreground and background processes. The program also 
                implements custom signal handlers for 2 signals, SIGINT, and SIGTSTP.
*/

//----------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

//----------------------------------------------------------------------

// a global variable to track if the TSTP signal was invoked or not.
static sig_atomic_t TSTP_flag = 0;

//----------------------------------------------------------------------

/*  a struct to track running background processes which stores 
    the process's pid and link to another process in a linked list.
*/  
struct backgroundPID
{
    // the pid of a background process
    pid_t pid;
    // a link to the next background process in the linked list
    struct backgroundPID *next;
};

//----------------------------------------------------------------------

/*  a struct to track the status code for each terminated foreground process,
    the struct stores an exit message and value, which will be updated whenever 
    a foreground process terminates.
*/
struct statusCode
{
    // an exit message
    char *message;
    // an exit value
    int code;
};

//----------------------------------------------------------------------

/*  
a struct to store a user entered command, the struct stores each part of a command
individually, the main command is stored alone, which the arguments to a command
are stored in a char array, the struct also keeps track of the count of arguments,
and file names used for input and output redirection.
*/
struct parsedCommand
{
    // store the main command
    char *command;
    // store the command arguments in a char array (including the main command)
    char **args;
    // store the count of arguments 
    int count;
    // store an input output symbol if entered by user (< or >)
    char *inOut;
    // store an input/output file name if a user chooses to redirect i/o
    char *inOutFile;
    // store another symbol by the user if the user chooses to redirect
    // both inputs and outputs
    char *out;
    // store the file name for the output redirection 
    char *outFile;
};

//----------------------------------------------------------------------

/*  
A signal handler function for SIGTSTP that sets a global flag TSTP_flag
to 1 if SIGTSTP was invoked, and ouputs a message, and resets the flag to 0
if SIGTSTP is invoked again, the flag value will be used in other functions
to determine whether to run child processes in the foreground or in the background.
*/

void handle_SIGTSTP(int signo)
{   
    // set flag to 0 if flag is 1 and output a message
    if (TSTP_flag == 1)
    {
        // when the flag is 0, foreground-only mode will be exited
        TSTP_flag = 0;
        char *newLine = "\n";
        char *message = "Exiting foreground-only mode\n";
        char *prompt = ": ";
        write(STDOUT_FILENO, newLine, 1);
        write(STDOUT_FILENO, message, 29);
        write(STDOUT_FILENO, prompt, 2);
    }
    // set flag to 1 if flag is 0 and output a message
    else
    {
        // when the flag is 1, foreground-only mode will be activated
        TSTP_flag = 1;
        char *newLine = "\n";
        char *message = "Entering foreground-only mode (& is now ignored)\n";
        char *prompt = ": ";
        write(STDOUT_FILENO, newLine, 1);
        write(STDOUT_FILENO, message, 49);
        write(STDOUT_FILENO, prompt, 2);
    }
}

//----------------------------------------------------------------------

/*  
An exit function that is activated when the user types "exit" in the command prompt,
the function takes a linked list that contains pids of processes running in the background
the function iterates through the pids of each process, and kills the one that are still active
before exiting the program. The function also frees memory occupied by the linked list of PIDs.
*/

void exitShell(struct backgroundPID *bgList)
{

    // iterate through the nodes in the list of PIDs
    while (bgList != NULL)
    {
        // if a node is still running (meaning its pid is not -5 which is the default value
        // set in this program for pids which have terminated) kill the node.
        if (bgList->pid != -5)
        {
            kill(bgList->pid, SIGKILL);
        }
        // let the current node point to the next
        bgList = bgList->next;

    }
}

//----------------------------------------------------------------------

/* 
A function that takes a user entered command and strips it from leading and trailing spaces
and returns the resulting string.
*/

char *removeSpaces(char *command)
{
    // create a variable to track the end of a command
    char *commandEnd;

    // iterate through the characters in the command and
    // move the command pointer if a space is encountered
    while (isspace((unsigned char) *command))
    {
        // move the command pointer forward
        command++;
    } 

    // if the command end up being empty, return it
    if (*command == 0)
    {
        // return the empty command
        return command;
    }

    // once leading spaces are cleared, set the commandEnd pointer to the 
    // last character in command
    commandEnd = command + strlen(command) - 1;

    // iterate through the command string using commandEnd as pointer
    // by moving backward whenever a space is encountered.
    while (commandEnd > command && isspace ((unsigned char) *commandEnd))
    {
        // move the commandEnd pointer when a space is encountered 
        commandEnd--;
    } 

    // at the end, when all spaces are ignored, set the commandEnd 
    // to a null terminator to finish the command string
    commandEnd[1] = '\0';

    // return the resulting string
    return command;
}

//----------------------------------------------------------------------

/* 
A function that changes the current working directory to the directory specified 
in the HOME environment variable.
*/

void homeDirectory()
{
    // get the directory at the HOME environment variable
    char *home = getenv("HOME");
    // change current directory to the directory specified above
    chdir(home);
}

//----------------------------------------------------------------------

/* 
A function that takes a linked list of pids (for background process), and a pid number
and creates a new node (backgroundPID struct) and sets the new pid number to that struct, 
and adds the new node/struct to the end of the linked list of background pids.
*/

void storeBgProcess(struct backgroundPID *bgList, pid_t bgPid)
{
    // first check if the linked list is empty
    // (by default, if the list is empty, the head node will have a -5 pid value)
    if (bgList->pid == -5)
    {
        // set the new given pid number to the head node
        bgList->pid = bgPid;
    }
    // if the list is not empty, create a new node, assign the new pid value to it
    // and add it to the tail of the linked list.
    else
    {
        // create a new struct to store the pid, and malloc space for it
        struct backgroundPID *pidNode = malloc(sizeof(struct backgroundPID));
        // assign the new pid number to the new node
        pidNode->pid = bgPid;
        // set the next node to be NULL
        pidNode->next = NULL;

        // iterate through the linked list of pids, and add the new node to the tail of the lsit
        while (bgList != NULL)
        {
            if (bgList->next == NULL)
            {
                bgList->next = pidNode;
                break;
            }

            bgList = bgList->next;
        }
    }
}

//----------------------------------------------------------------------

/* 
A function that takes the user enetered command, and tokenizes the command by 
stripping useful information from the command and storing it in a struct
The main command is separated, and the command arguments are added to a char array,
and the file names for input/out redirection (if exit) are also added to the struct.
The function then returns a parsedCommand struct which holds all this information
in separate variables.
*/

struct parsedCommand tokenizer(char *command)
{
    // create a new struct to store the command content
    struct parsedCommand commandLine;
    // create a char array to store the command arguments 
    char *args[512];
    // create a pointer to be used with the strtok_r function 
    char *ptr;
    
    // get the main command token
    char *token = strtok_r(command, " ", &ptr);
    // assign the main command to the struct's command variable
    commandLine.command = token;
    // set the count of arguments to 0
    commandLine.count = 0;
    // create a new variable and set it to "-", this will track if 
    // and input/output redirection symbol (< or >) was enetered by the user
    char *empty = "-";
    // set the struct variables (inOut and out) to "-", which means that no
    // i/o redirection symbols were enetered by the user
    commandLine.inOut = empty;
    commandLine.out = empty;

    // now tokenize the command arguments part
    while (token != NULL)
    {
        // if an i/o redirection symbol is entered, change the variables
        // in the struct to reflect this
        if ((strcmp(token, ">") == 0) || (strcmp(token, "<") == 0))
        {
            // first see if this was the first time a i/o redirection 
            // symbol is encountered, if so, set the inOut variable to reflect this
            if (strcmp(commandLine.inOut, "-") == 0)
            {
                // get another token which will have the file name to be used
                // in i/o redirection 
                commandLine.inOut = token;
                commandLine.inOutFile = strtok_r(NULL, " ", &ptr);
            }
            else
            {
                // if this is the second time an i/o redirection symbol is 
                // encountered, reflect this information in the out variable in the 
                // struct
                if (strcmp(commandLine.out, "-") == 0)
                {
                    commandLine.out = token;
                    // get the file name for the second i/o redirection
                    commandLine.outFile = strtok_r(NULL, " ", &ptr);
                }
            }
        }
        // otherwise, add the arguments to the list of argument
        else
        {
            // if a $$ sign is encountered on its own, expand it to the pid of the current process (smallsh)
            if (strcmp(token, "$$") == 0)
            {
                // get the current pid (for smallsh)
                int shellPID = getpid();
                // create a string to store the process number
                char strPID[6];
                // convert the pid number to a string and store it in the char variable above
                sprintf(strPID, "%d", shellPID);
                // increment the argument array counter 
                // and store the current argument token in the array args
                args[commandLine.count++] = strPID;

            }
            // if a "$$" symbol is found inside any argument, expand it to the 
            // pid of the current process (smallsh) and attach it to the string
            // at the same location
            else if(strstr(token, "$$"))
            {
                // create a new pointer and let it point to the current token
                // which has the "$$" substring in it
                char *newToken = token;
                // create a new pointer for another strtok_r function
                char *nptr;
                // move the old token to the next word in the argument list
                token = strtok_r(NULL, " ", &ptr);
                // now tokenize the current token, and get the string part
                // which doesn't have the "$$" in it and store this part in newStr
                char *newStr = strtok_r(newToken, "$$", &nptr);
                // create a char variable to store the combined string + pid
                char finalStr[255];
                // first copy the sliced string part to finalStr 
                strcpy(finalStr, newStr);
                // get the current pid (for smallsh), and convert it to a string
                int shellPID = getpid();
                char strPID[6];
                sprintf(strPID, "%d", shellPID);
                // add the stringified pid to the finalStr
                strcat(finalStr, strPID);
                // add the finished string to the array of arguments
                args[commandLine.count++] = finalStr;
            }
            // for all other argument types, simply add them to the args array
            else
            {
                args[commandLine.count++] = token;
            }
        }
        // move to the next argument in the command line
        token = strtok_r(NULL, " ", &ptr);
    }

    // once we have all arguments in the command line separated and
    // added to the args array, add a NULL value at the end of the args array
    // this is done so the array will be used properly by the execvp() function 
    // when the command is executed.
    args[commandLine.count++] = NULL;

    // now once the args array is filled, malloc some space in the uninitialized args 
    // variable inside the commandLine struct
    commandLine.args = malloc(commandLine.count * sizeof *commandLine.args);
    // copy the contents of args to commandLine.args
    memcpy(commandLine.args, args, commandLine.count * sizeof *commandLine.args);

    // the commandLine struct is now filled, return it.
    return commandLine;
    
}

//----------------------------------------------------------------------

/* 
A function that takes a linked list of background process PIDs, and iterates through the
nodes in the list, waiting (WNOHANG) for each pid and checking if a process is terminated,
if so, the function prints messages reflecting this.
*/

void checkBgProcesses(struct backgroundPID *bgList)
{
    // iterate through the linked list of background pids 
    while (bgList != NULL)
    {
        // if a node doesn't have a pid value of -5 (meaning its a valid background process)
        // wait on it (WNOHANG) and print a message if it is terminated.
        if (bgList->pid != -5)
        {
            // set variables to store the process exit status
            int childExitStatus = -5;

            // get the process pid from the struct
            pid_t childPid = bgList->pid;

            // wait on the process (WNOHANG)
            pid_t actualPid = waitpid(childPid, &childExitStatus, WNOHANG);

            // if the process terminated, print a message with the exit code

            // if the process terminated normally
            if (WIFEXITED(childExitStatus))
            {
                // print a message, and get the exit status
                printf("background pid %d is done: exit value %d\n", childPid, WEXITSTATUS(childExitStatus));
                // flush the standard output
                fflush(stdout);
                // set the pid for the terminated process to -5 (meaning it is now terminated and will be
                // ignored in future checks)
                bgList->pid = -5;
            }
            // if the process terminated abnormally
            else if (actualPid == -1) 
            {
                // print a message, and get the exit status
                printf("background pid %d is done: terminated by signal %d\n", childPid, WTERMSIG(childExitStatus));
                fflush(stdout);
                // set the pid for the terminated process to -5 (meaning it is now terminated and will be
                // ignored in future checks)
                bgList->pid = -5;
            }
        }

        // move to the next node in the list
        bgList = bgList->next;
    }
}

//----------------------------------------------------------------------

/* 
A function that takes a file name in the current directory, and redirects standard
input to that file.
*/

void redirectInput(char *fileName)
{
    // open the file in read only mode
    int sourceFD = open(fileName, O_RDONLY);

    // if file opening failed, print an error message and set exit status to 1
    if (sourceFD == -1)
    {
        printf("cannot open %s for input\n", fileName);
        fflush(stdout);
        exit(1);
    }

    // redirect standard input to the opened file using the dup2() function
    int result = dup2(sourceFD, 0);

    // if redirection fails, print an error message and set exit status to 1
    if (result == -1)
    {
        printf("stdin redirection failed\n");
        fflush(stdout);
        exit(1);
    }
}

//----------------------------------------------------------------------

/* 
A function that takes a file name, and redirects standard
output to that file, the function creates the file if it doesn't exist,
or truncates it otherwise.
*/

void redirectOutput(char *fileName)
{
    // open/create the file in write only mode
    int targetFD = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    // if opening/creation failed, print a message and set exit status to 1
    if (targetFD == -1)
    {
        printf("cannot open %s for output\n", fileName);
        fflush(stdout);
        exit(1);
    }
    // redirect standard output to the newly created/opened file using the dup2() function
    int result = dup2(targetFD, 1);
    // if redirection failed, print a message and set the exit status to 1
    if (result == -1)
    {
        printf("stdout redirection failed\n");
        fflush(stdout);
        exit(1);
    }
}

//----------------------------------------------------------------------

/* 
A function that takes the following parameters:
    - command: the user enetered command 
    - execMode: the execusion mode (0 for foreground, and 1 for background)
    - exitStatus: a statusCode struct thatupdates and tracks the exit status for each terminated foreground process
    - bgList: a backgroundPID struct to track and store background process PIDs
    - ignore_action: a sigaction struct with a built-in signal handler that is set to ignore signals.
    - default_action: a sigaction struct with a built-in signal handler that is set to the default behaviour for a signal.

The function uses the above information to fork a child process, which will use the execvp()
function to execute the command either in the foreground or the background depending on the value of execMode,
and redirect i/o if specified, and print exit status for foreground process, and store background process pids for later monitoring.
*/

void executeCommand(char *command, int execMode, struct statusCode *exitStatus, struct backgroundPID *bgList, struct sigaction ignore_action, struct sigaction default_action)
{
    // tokenize the user command line using the tokenizer() function and 
    // store information in a struct
    struct parsedCommand commandLine = tokenizer(command);

    // create variables to store the child process pid, and exit status
    pid_t childPid = -5;
    int childExitStatus = -5;

    // create a new child process by using the fork() function
    childPid = fork();

    // check the return values of the fork function
    switch (childPid) {

        // if fork value = -1, the fork is unsuccessful, print a message and exit.
        case -1: { perror("Fork unsuccessful!\n"); exit(1); break; }

        // if fork value is 0, proceed with executing the command in the child process
        case 0: {

            // if execMode is 0, set the SIGINT signal to default behaviour 
            // meaning that foreground processes can be interrupted if SIGINT is invoked by the user
            if (execMode == 0)
            {
                sigaction(SIGINT, &default_action, NULL);
            }

            // for foreground and background processes, ignore the SIGTSTP signal if invoked by the user
            sigaction(SIGTSTP, &ignore_action, NULL);
            
            // check if i/o redirection is specified in the command line
            if (strcmp(commandLine.inOut, "-") != 0)
            {
                // redirect input if the "<" symbol is found
                if (strcmp(commandLine.inOut, "<") == 0)
                {
                    // redirect stdin to commandLine.inOutFile
                    redirectInput(commandLine.inOutFile);
                }
                // redirect output if the ">" symbol is found in the command
                else
                {
                    // redirect stdout to commandLine->outFile
                    redirectOutput(commandLine.inOutFile);
                }
            }

            // redirect output if another symbol is found in the command line
            if (strcmp(commandLine.out, ">") == 0)
            {
                redirectOutput(commandLine.outFile);
            }

            // execute the command using execvp() by linking the command and the array of command arguments
            execvp(commandLine.command, commandLine.args);
            // print an error message if the command is not executed successfully (i.e not found in the PATH variable)
            printf("%s: command not found\n", commandLine.command);
            // flush standard ouput
            fflush(stdout);
            // exit and set status code to 2
            exit(2);
            break;
        }

        // parent process 
        default: {

            // wait for and monitor foreground processes if the execMode is 0
            if (execMode == 0)
            {
                // wait for the current foreground process
                waitpid(childPid, &childExitStatus, 0);

                // when the current child process is terminated, get the status code

                // if the process exits normally
                if (WIFEXITED(childExitStatus))
                {
                    // store the exit status in the exitStatus struct
                    char *msg;
                    msg = "exit value";
                    exitStatus->message = msg;
                    exitStatus->code = WEXITSTATUS(childExitStatus);

                }
                // if the process exited abnormally
                else 
                {
                    // store the exit status in the exitStatus struct and 
                    // print a message with the signal termination value
                    char *msg;
                    msg = "terminated by signal";
                    exitStatus->message = msg;
                    exitStatus->code = WTERMSIG(childExitStatus);
                    printf("terminated by signal %d\n", exitStatus->code);
                    fflush(stdout);
                }
                break;
            }

            // for background child processes, print a message with the pid
            // of the new background process, and save the child pid in the linked
            // list of background pids, and return to the command prompt
            else 
            {
                printf("background pid is %d\n", childPid);
                fflush(stdout);
                waitpid(childPid, &childExitStatus, WNOHANG);
                storeBgProcess(bgList, childPid);
                break;
            }
        }
    }
    // free the argument array memory inside the commandLine struct to prepare for another 
    // command by the user
    free(commandLine.args);
}

//----------------------------------------------------------------------

/* 
A function that takes a path, and changes the current directory to that path
*/

void changeDirectory(char *path) 
{
    // create pointer to be used with strtok_r()
    char *ptr;

    // first token is cd
    char *token = strtok_r(path, " /", &ptr);

    // second token is start of path
    token = strtok_r(NULL, " /", &ptr);

    // tokenize the provided filepath
    while (token != NULL)
    {
        // modify $$ here

        // attempt to open the directory in the path
        DIR* dir = opendir(token);

        // if opening is successful, change current directory
        // to that directory
        if (dir) {
            closedir(dir);
            chdir(token);
            token = strtok_r(NULL, " /", &ptr);
        }
        // if opening is unsuccessful, print an error message and return
        else
        {   
            printf("Directory not found\n");
            break;
        }
    }
}

//----------------------------------------------------------------------

/* 
A command prompt function that performs the following:
    - set signal handling for SIGINT and SIGTSTP
    - set up structs to track background processes, and exit status for foreground processes
    - set up a flag to track foreground and background execution of child processes
    - enter a command prompt loop that allows the user to enter commands.
    - analyzes and stores the user command, and calls other functions to execute the user input.
    - keeps promting the user input of commands until the user types "exit".
    - frees all dynamically allocated memory prior to exiting the program.

*/

void commandPrompt()
{

//----------------------Implement signal handlers for SIGINT and SIGTSTP--------------

    // create a sigaction struct to handle SIGTSTP
    struct sigaction SIGTSTP_action = {{0}};
    // create a sigaction struct to ignore signals
    struct sigaction ignore_action = {{0}};
    // create a sigaction struct to use the default behavior of signals
    struct sigaction default_action = {{0}};

    SIGTSTP_action.sa_handler = handle_SIGTSTP;
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGTSTP_action.sa_flags = SA_RESTART;

    ignore_action.sa_handler = SIG_IGN;
    default_action.sa_handler = SIG_DFL;

    sigaction(SIGINT, &ignore_action, NULL);
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

//----------------------Initiate struct to track execution status--------------

    // initiate statusCode struct and set its message to "exit value 0"
    // this statusCode struct will be later passed to other functions when executing commands using exec()
    // and its value will be updated regulary with each terminated process so that it always holds the 
    // exit status for the last terminated process.
    struct statusCode *exitStatus = malloc(sizeof(struct statusCode));
    char *msg = "exit value";
    int value = 0;
    exitStatus->message = msg;
    exitStatus->code = value;

//----------------------Initiate a struct to store background PIDs--------

    struct backgroundPID *bgList = malloc(sizeof(struct backgroundPID));
    bgList->pid = -5;
    bgList->next = NULL;

//----------------------Initiate a variable to store execution mode--------

    // "1" is for background mode, and "0" is for foreground mode.
    int execMode = 0;

//----------------------------Enter command prompt loop------------------------

    while(1)
    {

    //---------------------show prompt and get user input---------------------

        // background processes must be checked here before user is handed control
        checkBgProcesses(bgList);
        // create a char to store the user input
        char command[2048];
        // prompt user for input
        printf(": ");
        fflush(stdout);
        // get user input and store it in command
        fgets(command, 2048, stdin);
        // remove new line at the end of user input
        command[strlen(command) - 1] = 0;

    //--------------remove trailing and leading spaces from command-----------

        // call function to remove the leading and trailing empty space -if exit- in the user entered command
        // store the new command in a new variable
        char *newCommand = removeSpaces(command);

    //--------------------------------Set execution mode--------------------------------------------

        // check if last letters in a command is " &"
        int len = strlen(newCommand);
        if (len > 1)
        {
            const char *lastTwo = &newCommand[len - 2];
            if (strcmp(lastTwo, " &") == 0)
            {
                // remove the & from string as we don't need it anymore
                newCommand[strlen(newCommand) - 2] = '\0';
                // check to see if the TSTP signal is invoked
                if (TSTP_flag != 1)
                {
                    // if not, set the execision mode to background 
                    execMode = 1;
                }
            }
            // otherwise set the execution mode to foreground
            else
            {
                execMode = 0;
            }
        }

    //------------------------------Analyze command to determine which function to use-----------------------------------------

        // if the user enteres "exit", call the exitShell() function
        if (strncmp(newCommand, "exit", 4) == 0)
        {
            // call the exit function and add the bgList of background pids as parameter
            exitShell(bgList);
            // break from the command prompt loop and return to main
            break;
        }
        // ignore user input input if they provided empty input or a comment (starting with a #)
        else if ((strcmp(newCommand, "") == 0) || (newCommand[0] == '#')) 
        {
            continue;
        }

        // call the homeDirectory() function if the user entered cd
        else if (strcmp(newCommand, "cd") == 0)
        {
            homeDirectory();
        }

        // call the changeDirectory() function if the user entered cd with arguments
        else if (strncmp(newCommand, "cd ", 3) == 0)
        {
            changeDirectory(newCommand);
        }
        // print the status of the last temrminated foreground process if the user entered "status"
        // the last status is stored in the exitStatus struct
        else if (strncmp(newCommand, "status", 6) == 0)
        {
            printf("%s %d\n", exitStatus->message, exitStatus->code);
        }

        // for all other non built-in commands, call the executeCommand() function to handle these commands
        else
        {
            executeCommand(newCommand, execMode, exitStatus, bgList, ignore_action, default_action);
        }
        
    }
    
    // user is exiting
    // free the memory dynamically allocated for the exitStatus struct
    free(exitStatus);
    // struct backgroundPID *tmp;
    // while (bgList != NULL)
    // {
    //     tmp = bgList;
    //     bgList = bgList->next;
    //     free(tmp);
    // }
}

//----------------------------------------------------------------------

/*
The main function which calls the commandPrompt function to start the program. 
*/

int main(int argc, char* argv[])
{
    // call the commandPrompt function to start smallsh
    commandPrompt();

    // exit the program when the user decides to exit
    return EXIT_SUCCESS;
};