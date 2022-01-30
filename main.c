#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void exitShell()
{
    printf("Ending processes and exiting shell\n");
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
        if (strcmp(command, "exit") == 0)
        {
            exitShell();
            break;
        }
        else if ((strcmp(command, "") == 0) || (command[0] == '#')) // fix multiple spaces
        {
            continue;
        }
        else if (strcmp(command, "pwd") == 0)
        {
            char path[255];
            getcwd(path, sizeof(path));
            printf("Current working directory is %s\n", path);
        }
        else if (strcmp(command, "cd") == 0)
        {
            changeDirectory();
        }
        else
        {
        printf("You entered: %s\n", command);
        }
        
    }

}

int main(int argc, char* argv[])
{
    commandPrompt();

    return EXIT_SUCCESS;
};