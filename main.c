#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>


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
    char *home = getenv("HOME");
    chdir(home);
    printf("Changed directory to %s\n", home);
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

void status()
{

}

void tokenizer()
{

}

void commandPrompt()
{
    while(1)
    {
        char command[2048];
        printf(": ");
        fgets(command, 2048, stdin);
        command[strlen(command) - 1] = 0;

        // remove trailing and leading spaces 
        char *newCommand = removeSpaces(command);

        if (strcmp(newCommand, "exit") == 0)
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
        else
        {
        printf("You entered: %s\n", newCommand);
        }
        
    }

}

int main(int argc, char* argv[])
{
    commandPrompt();

    return EXIT_SUCCESS;
};