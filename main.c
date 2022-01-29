#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void exitShell()
{

}

void changeDirectory()
{

}

void status()
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
            break;
        }
        printf("You entered: %s\n", command);
    }

}

int main(int argc, char* argv[])
{
    commandPrompt();

    return EXIT_SUCCESS;
};