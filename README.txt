# Smallsh

This repository hosts a mini shell program written in C. The program provides a command-line interface for executing shell commands. The features of the shell include:

- A prompt for running shell commands.
- The ability to handle blank lines and comment lines (beginning with #).
- Supports the expansion for the variable $$.
- Three built-in commands: cd, exit, and status.
- Executing other shell commands entered by the user using the exec family of functions.
- Input and output redirection using the '<' and '>' operators.
- Running commands in both the foreground and background processes.
- Custom signal handlers for SIGINT and SIGTSTP signals.

## Files

- `main.c` - The source code file for the mini shell program.

## How to Run

1. Clone this repository to your local machine.
2. Navigate to the directory containing `main.c`.
3. Compile the program using the GCC compiler.

    ```bash
    gcc --std=gnu99 -o smallsh main.c
    ```

4. Run the compiled program.

    ```bash
    ./smallsh
    ```

## Built-In Commands

### `cd`

Change the current working directory. If a directory is provided (e.g., `cd /path/to/directory`), the shell will navigate to the specified directory. If no directory is provided (e.g., `cd`), the shell will navigate to the home directory.

### `exit`

Terminate the shell, including all background processes.

### `status`

Print the exit status or the terminating signal of the last foreground process.

## Redirection

The shell supports input and output redirection using the '<' and '>' symbols respectively. For example:

- Input redirection:

    ```bash
    command < filename
    ```

  This command will run `command` using `filename` as the input.

- Output redirection:

    ```bash
    command > filename
    ```

  This command will run `command`, directing the output to `filename`.

## Background Processes

The shell can run processes in the background by appending '&' at the end of the command.
