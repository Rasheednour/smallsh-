#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>


// FIX & SIGN AT END OF BUILT IN FUNCTIONS
// FIX CD WHEN ONE OF THE FILES IN PATH DOES NOT EXIST
// UPDATE white space stripper to remove extra spaces between command and arguments
// IMPLEMENT A TOKENIZER TO RETURN A LIST OF COMMAND AND ITS ARGUMENTS along with & if exists

struct backgroundPID
{
    int pid;
    struct backgroundPID *next;
};

struct statusCode
{
    char *message;
    int code;
};

struct parsedCommand
{
    char *command;
    char **args;
    char *count;
    char inOut[1];
    char *file1;
    char out[1];
    char *file2;
};

void exitShell()
{
    // take an array of pids for background processes and kill them
    printf("Ending processes and exiting shell\n");
}

char *removeSpaces(char *command)
{
    char *commandEnd;

    while (isspace((unsigned char) *command))
    {
        command++;
    } 

    if (*command == 0)
    {
        return command;
    }

    commandEnd = command + strlen(command) - 1;

    while (commandEnd > command && isspace ((unsigned char) *commandEnd))
    {
        commandEnd--;
    } 

    commandEnd[1] = '\0';

    return command;
}

void homeDirectory()
{
    printf("sleeping for 5 seconds..\n");
    sleep(5);
    char *home = getenv("HOME");
    chdir(home);
    printf("Changed directory to %s\n", home);
}

void printPath()
{
    char *path = getenv("PATH");
    printf("The path is: %s\n", path);
}

void executeVP(char** commandList)
{
    // execute parsed list of command and arguments
}

void executeLP(char *command)
{
    // execute a single command
}

void storeBgProcess(struct backgroundPID *bgList)
{
    // add new background PID to the tail of linked lists of background processes
}

void executeCommand(char *command, int execMode, struct statusCode *exitStatus, struct backgroundPID *bgList)
{
    // printf("Command is: %s\n", command);
    printf("parent PID is: %d\n", getpid());
    printf("backgorund process number: %d\n", bgList->pid);
    printf("execMode is: %d\n", execMode);
    printf("command is: %s\n", command);
    // printf("forking child..\n");

    // pid_t spawnPid = -5;
    // int childExitStatus = -5;

    // spawnPid = fork();

    // switch (spawnPid) {
    //     case -1: { perror("Fork unsuccessful!\n"); exit(1); break; }
    //     case 0: {
    //         printf("New child forked with pid %d and parnet's pid is %d\n", getpid(), getppid());
    //         sleep(1);
    //         printf("Child with pid %d now executing command: %s\n", getpid(), command);
    //         sleep(2);
    //         execlp(command, command, NULL);
    //         perror("child exec failure!\n");
    //         exit(2); break;
    //     }
    //     default: {
    //         printf("parent with pid %d now sleeping for 2 secs\n", getpid());
    //         sleep(2);
    //         printf("parent with pid %d now waiting for child with pid %d to terminate\n", getpid(), spawnPid);
    //         pid_t actualPid = waitpid(spawnPid, &childExitStatus, 0);
    //         printf("parent with pid %d: child with pid %d terminated, now exiting!\n", getpid(), actualPid);
    //         exit(0); break;
    //     }
    // }



    // get main command
    // 

    
}

void openInputFile()
{

}

void openOutputFile()
{

}

void changeDirectory(char *path) // dont change directories if any of them is invalid
{
    printf("Now changing directory\n");

    char *ptr;
    // first token is cd
    char *token = strtok_r(path, " /", &ptr);
    // second token is start of path
    token = strtok_r(NULL, " /", &ptr);

    while (token != NULL)
    {
        DIR* dir = opendir(token);

        if (dir) {
            closedir(dir);
            chdir(token);
            token = strtok_r(NULL, " /", &ptr);
        }
        else
        {   
            break;
        }
    }
}


void tokenizer(char *command)
{

}

void commandPrompt()
{

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
    bgList->pid = -1;
    bgList->next = NULL;

//----------------------Initiate a variable to store execution mode--------

    // "1" is for background mode, and "0" is for foreground mode.
    int execMode = 0;

//----------------------------Enter command prompt loop------------------------

    while(1)
    {

    //---------------------show prompt and get user input---------------------

        // background processes must be checked here before user is handed control
        char command[2048];
        printf(": ");
        // get user input
        fgets(command, 2048, stdin);
        // remove new line at the end of user input
        command[strlen(command) - 1] = 0;

    // --------------------check signal for foreground mode only-------------
    //--------------remove trailing and leading spaces from command-----------

        // call function to remove the leading and trailing empty space -if exit- in the user entered command
        // store the new command in a new variable
        char *newCommand = removeSpaces(command);

    //--------------------------------Set execution mode--------------------------------------------

        // check if last letters in command is " &"
        int len = strlen(newCommand);
        if (len > 1)
        {
            const char *lastTwo = &newCommand[len - 2];
            if (strcmp(lastTwo, " &") == 0)
            {
                // remove the & from string as we don't need it anymore
                newCommand[strlen(newCommand) - 2] = '\0';
                execMode = 1;
            }
            else
            {
                execMode = 0;
            }
        }

    //----------------------------------------------------------------------------------------------

        if (strncmp(newCommand, "exit", 4) == 0)
        {
            exitShell();
            break;
        }
        else if ((strcmp(newCommand, "") == 0) || (newCommand[0] == '#')) 
        {
            continue;
        }

        else if (strcmp(newCommand, "cd") == 0)
        {
            homeDirectory();
        }

        else if (strncmp(newCommand, "cd ", 3) == 0)
        {
            changeDirectory(newCommand);
        }
        else if (strncmp(newCommand, "status", 6) == 0)
        {
            printf("%s %d\n", exitStatus->message, exitStatus->code);
        }

        else
        {
            executeCommand(newCommand, execMode, exitStatus, bgList);
        }
        
    }
    
    // user is exiting
    // kill background processes inside of exit function while passing it PID list
    // free status code and background PID list
    free(exitStatus);
    struct backgroundPID *tmp;
    while (bgList != NULL)
    {
        tmp = bgList;
        bgList = bgList->next;
        free(tmp);
    }
    

}

int main(int argc, char* argv[])
{
    commandPrompt();

    return EXIT_SUCCESS;
};