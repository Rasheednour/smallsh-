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
void exitShell()
{
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

int executeCommand(char *command)
{
    // printf("Command is: %s\n", command);
    printf("parent PID is: %d\n", getpid());
    printf("forking child..\n");

    pid_t spawnPid = -5;
    int childExitStatus = -5;

    spawnPid = fork();

    switch (spawnPid) {
        case -1: { perror("Fork unsuccessful!\n"); exit(1); break; }
        case 0: {
            printf("New child forked with pid %d and parnet's pid is %d\n", getpid(), getppid());
            sleep(1);
            printf("Child with pid %d now executing command: %s\n", getpid(), command);
            sleep(2);
            execlp(command, command, NULL);
            perror("child exec failure!\n");
            exit(2); break;
        }
        default: {
            printf("parent with pid %d now sleeping for 2 secs\n", getpid());
            sleep(2);
            printf("parent with pid %d now waiting for child with pid %d to terminate\n", getpid(), spawnPid);
            pid_t actualPid = waitpid(spawnPid, &childExitStatus, 0);
            printf("parent with pid %d: child with pid %d terminated, now exiting!\n", getpid(), actualPid);
            exit(0); break;
        }
    }

    fork();

    
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

void updateStatus(int statusCode)
{
    printf("exit value %d\n", statusCode);
}

void commandTokenizer()
{

}

void commandPrompt()
{
    int statusCode = 0;
    updateStatus(statusCode);

    while(1)
    {
        char command[2048];
        printf(": ");
        fgets(command, 2048, stdin);
        command[strlen(command) - 1] = 0;

        // remove trailing and leading spaces 
        char *newCommand = removeSpaces(command);

        if (strncmp(newCommand, "exit", 4) == 0)
        {
            exitShell();
            break;
        }
        else if ((strcmp(newCommand, "") == 0) || (newCommand[0] == '#')) // fix multiple spaces
        {
            continue;
        }
        else if (strcmp(newCommand, "pwd") == 0)
        {
            char path[255];
            getcwd(path, sizeof(path));
            printf("Current working directory is %s\n", path);
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
            updateStatus(0);
        }
        else if (strcmp(newCommand, "pid") == 0)
        {
            printf("My pid is %d\n", getpid());
        }
        else if (strcmp(newCommand, "ppid") == 0)
        {
            printf("My parent's pid is %d\n", getppid());
        }
        else if (strcmp(newCommand, "path") == 0)
        {
            printPath();
        }
        else
        {
            executeCommand(newCommand);
        }
        
    }

}

int main(int argc, char* argv[])
{
    commandPrompt();

    return EXIT_SUCCESS;
};