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


// FIX & SIGN AT END OF BUILT IN FUNCTIONS
// FIX CD WHEN ONE OF THE FILES IN PATH DOES NOT EXIST
// UPDATE white space stripper to remove extra spaces between command and arguments
// IMPLEMENT A TOKENIZER TO RETURN A LIST OF COMMAND AND ITS ARGUMENTS along with & if exists

struct backgroundPID
{
    pid_t pid;
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
    int count;
    char *inOut;
    char *inOutFile;
    char *out;
    char *outFile;
};

void exitShell(struct backgroundPID *bgList)
{
    // take an array of pids for background processes and kill them
    // printf("Ending processes and exiting shell\n");
    struct backgroundPID *tmp;
    while (bgList != NULL)
    {
        tmp = bgList;
        bgList = bgList->next;
        free(tmp);
    }
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
    // sleep(5);
    char *home = getenv("HOME");
    chdir(home);
    // printf("Changed directory to %s\n", home);
}

void printPath()
{
    char *path = getenv("PATH");
    printf("The path is: %s\n", path);
}



void storeBgProcess(struct backgroundPID *bgList, pid_t bgPid)
{
    // add new background PID to the tail of linked lists of background processes


    if (bgList->pid == -1)
    {
        bgList->pid = bgPid;
    }
    else
    {
        struct backgroundPID *pidNode = malloc(sizeof(struct backgroundPID));
        pidNode->pid = bgPid;
        pidNode->next = NULL;

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

struct parsedCommand tokenizer(char *command)
{
    struct parsedCommand commandLine;
    char *args[512];
    char *ptr;
    
    // get command
    char *token = strtok_r(command, " ", &ptr);
    commandLine.command = token;
    commandLine.count = 0;
    char *empty = "-";
    commandLine.inOut = empty;
    commandLine.out = empty;

    while (token != NULL)
    {
        if ((strcmp(token, ">") == 0) || (strcmp(token, "<") == 0))
        {
            if (strcmp(commandLine.inOut, "-") == 0)
            {
                commandLine.inOut = token;
                commandLine.inOutFile = strtok_r(NULL, " ", &ptr);
            }
            else
            {
                if (strcmp(commandLine.out, "-") == 0)
                {
                    commandLine.out = token;
                    commandLine.outFile = strtok_r(NULL, " ", &ptr);
                }
            }
        }
        else
        {
            if (strcmp(token, "$$") == 0)
            {
                int shellPID = getpid();
                char strPID[6];
                sprintf(strPID, "%d", shellPID);
                args[commandLine.count++] = strPID;

            }
            else if(strstr(token, "$$"))
            {
                char *newToken = token;
                char *nptr;
                token = strtok_r(NULL, " ", &ptr);
                char *newStr = strtok_r(newToken, "$$", &nptr);
                char finalStr[255];
                strcpy(finalStr, newStr);
                int shellPID = getpid();
                char strPID[6];
                sprintf(strPID, "%d", shellPID);
                strcat(finalStr, strPID);
                printf("newStr is: %s\n", finalStr);


                args[commandLine.count++] = finalStr;
            }
            else
            {
                args[commandLine.count++] = token;
            }
        }
        
        token = strtok_r(NULL, " ", &ptr);
    }

    args[commandLine.count++] = NULL;

    commandLine.args = malloc(commandLine.count * sizeof *commandLine.args);
    memcpy(commandLine.args, args, commandLine.count * sizeof *commandLine.args);

    // printf("command is: %s\n", commandLine.command);
    // printf("args are: \n");
    // for (int i = 0; i < commandLine.count; ++i)
    // {
    //     printf("%s ", commandLine.args[i]);
    // }


    return commandLine;
    
}

void checkBgProcesses(struct backgroundPID *bgList)
{
    if (bgList->pid != -1)
    {
        while (bgList != NULL)
        {
            int childExitStatus = -5;
            pid_t childPid = bgList->pid;
            // consider actualPid = waitpid(..) and check when actualPid returns childPid then check exit status
            waitpid(childPid, &childExitStatus, WNOHANG);
            if (WIFEXITED(childExitStatus))
            {
                printf("background pid %d is done: exit value %d\n", childPid, WEXITSTATUS(childExitStatus));
                fflush(stdout);
                bgList->pid = -1;
            }
            // else
            // {
            //     printf("background pid %d is done: terminated by signal %d\n", childPid, WTERMSIG(childExitStatus));
            //     fflush(stdout);
            //     bgList->pid = -1;
            // }
            bgList = bgList->next;

        }
    }
}

void redirectInput(char *fileName)
{
    int sourceFD = open(fileName, O_RDONLY);
    if (sourceFD == -1)
    {
        printf("cannot open %s for input\n", fileName);
        fflush(stdout);
        exit(1);
    }
    int result = dup2(sourceFD, 0);
    if (result == -1)
    {
        printf("stdin redirection failed\n");
        fflush(stdout);
        exit(1);
    }
}

void redirectOutput(char *fileName)
{
    int targetFD = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (targetFD == -1)
    {
        printf("cannot open %s for output\n", fileName);
        fflush(stdout);
        exit(1);
    }
    int result = dup2(targetFD, 1);
    if (result == -1)
    {
        printf("stdout redirection failed\n");
        fflush(stdout);
        exit(1);
    }
}


void executeCommand(char *command, int execMode, struct statusCode *exitStatus, struct backgroundPID *bgList)
{
    // printf("parent PID is: %d\n", getpid());
    // printf("backgorund process number: %d\n", bgList->pid);
    // printf("execMode is: %d\n", execMode);
    // printf("command is: %s\n", command);
    // printf("\n");
    struct parsedCommand commandLine = tokenizer(command);

    // printf("\n");
    // printf("arg count is: %d\n", commandLine.count);
    // printf("inOut is: %s\n", commandLine.inOut);
    // printf("inOutFile is: %s\n", commandLine.inOutFile);
    // printf("out is: %s\n", commandLine.out);
    // printf("outfile is: %s\n", commandLine.outFile);



    // printf("forking child..\n");

    pid_t childPid = -5;
    int childExitStatus = -5;

    childPid = fork();

    switch (childPid) {
        case -1: { perror("Fork unsuccessful!\n"); exit(1); break; }
        case 0: {
            // printf("New child forked with pid %d and parnet's pid is %d\n", getpid(), getppid());
            // sleep(1);
            // printf("Child with pid %d now executing command: %s\n", getpid(), commandLine.command);
            // sleep(3);
            // check input redirection here
            if (strcmp(commandLine.inOut, "-") != 0)
            {
                if (strcmp(commandLine.inOut, "<") == 0)
                {
                    // redirect stdin to commandLine.inOutFile
                    redirectInput(commandLine.inOutFile);
                }
                else
                {
                    // redirect stdout to commandLine->outFile
                    redirectOutput(commandLine.inOutFile);
                }
            }

            if (strcmp(commandLine.out, ">") == 0)
            {
                redirectOutput(commandLine.outFile);
            }

            execvp(commandLine.command, commandLine.args);
            printf("%s: command not found\n", commandLine.command);
            fflush(stdout);
            exit(2);
            break;
        }
        default: {
            // printf("parent with pid %d now sleeping for 2 secs\n", getpid());
            // execute foreground processes 
            if (execMode == 0)
            {
                // sleep(1);
                // printf("parent with pid %d now waiting for child with pid %d to terminate\n", getpid(), childPid);
                // check for signal here***
                waitpid(childPid, &childExitStatus, 0);
                // printf("parent with pid %d: child with pid %d terminated, now exiting!\n", getpid(), actualPid);
                // get foreground process exit status and save it in the struct
                if (WIFEXITED(childExitStatus))
                {
                    char *msg;
                    msg = "exit value";
                    exitStatus->message = msg;
                    exitStatus->code = WEXITSTATUS(childExitStatus);

                }
                else // add signal here
                {
                    char *msg;
                    msg = "terminated by signal";
                    exitStatus->message = msg;
                    exitStatus->code = WTERMSIG(childExitStatus);
                }
            
                // exit(0); break;
                break;
            }
            // execute background process
            else 
            {
                printf("background pid is %d\n", childPid);
                fflush(stdout);
                waitpid(childPid, &childExitStatus, WNOHANG);
                storeBgProcess(bgList, childPid);
                // exit(0); break;
                break;
            }


            
        }
    }

    free(commandLine.args);


    // get main command
    // 

    
}


void changeDirectory(char *path) // dont change directories if any of them is invalid
{

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
        checkBgProcesses(bgList);
        fflush(stdout);

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
            exitShell(bgList);
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