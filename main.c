#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

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

void changeDirectory()
{
    char *home = getenv("HOME");
    chdir(home);
    printf("Changed directory to %s\n", home);
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
            changeDirectory();
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