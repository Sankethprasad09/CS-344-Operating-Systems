#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_COMMAND_LENGTH 2048
#define MAX_ARGS 512

// Function Prototypes
// These define all the functions used in the code for execution, signal handling, etc.
void execute_command(char* command);
void handle_SIGTSTP(int signo);
void handle_SIGINT(int signo);
int built_in_commands(char* command, char* args[], int numArgs);
void exec_other_commands(char* command, char* args[], int numArgs, int background);
void input_output_redirection(char** args, int numArgs, int* inputFile, int* outputFile);

// Shell state variables
// These variables store the current state of the shell and its running processes.
int status = 0;
int foreground_only_mode = 0;
int last_exit_status = 0;
int last_signal_received = 0;

// Function to replace a substring with another substring in a given string.
char* replace_string(char* str, char* oldSubstr, char* newSubstr) {
    char* result;
    int i, count = 0;
    int newSubstr_len = strlen(newSubstr);
    int oldSubstr_len = strlen(oldSubstr);

    for(i = 0; str[i] != '\0'; i++) {
        if(strstr(&str[i], oldSubstr) == &str[i]) {
            count++;
            i += oldSubstr_len - 1;
        }
    }

    result = (char*) malloc(i + count * (newSubstr_len - oldSubstr_len) + 1);
    i = 0;
    while(*str) {
        if(strstr(str, oldSubstr) == str) {
            strcpy(&result[i], newSubstr);
            i += newSubstr_len;
            str += oldSubstr_len;
        } else
            result[i++] = *str++;
    }
    result[i] = '\0';
    return result;
}

// Function to execute a command
void execute_command(char* command) {

    if (command[0] == '\n' || command[0] == '#') {
        return;
    }

    char pid[10];
    sprintf(pid, "%d", getpid());
    command = replace_string(command, "$$", pid);

    
    char* args[MAX_ARGS];
    int numArgs = 0;
    char* token = strtok(command, " \n");
    while(token != NULL) {
        args[numArgs] = strdup(token);
        numArgs++;
        token = strtok(NULL, " \n");
    }
    args[numArgs] = NULL;
    int background = 0;
    if (strcmp(args[numArgs-1], "&") == 0) {
        if (!foreground_only_mode) {
            background = 1;
        }
        args[numArgs-1] = NULL;
        numArgs--;
    }
    status = built_in_commands(args[0], args, numArgs);
    if (status == -1) {
        exec_other_commands(args[0], args, numArgs, background);
    }
    while(numArgs >= 0) {
        free(args[numArgs]);
        numArgs--;
    }
}

// Function to handle SIGTSTP signal (Ctrl-Z). Switches the shell between foreground-only and normal mode.
void handle_SIGTSTP(int signo) {
    if (!foreground_only_mode) {
        char* message = "\nEntering foreground-only mode (& is now ignored)\n";
        write(STDOUT_FILENO, message, 50);
        fflush(stdout);
        foreground_only_mode = 1;
    } else {
        char* message = "\nExiting foreground-only mode\n";
        write(STDOUT_FILENO, message, 30);
        fflush(stdout);
        foreground_only_mode = 0;
    }
}

void handle_SIGINT(int signo) {
    // Do nothing as the shell must ignore SIGINT
}
// Function to execute built-in commands: exit, cd, status. Returns -1 if command is not built-in.
 int built_in_commands(char* command, char* args[], int numArgs) {
    if (strcmp(command, "exit") == 0) {
        exit(0);
    } else if (strcmp(command, "cd") == 0) {
        if (numArgs == 1) {
            chdir(getenv("HOME"));
        } else {
            if (chdir(args[1]) == -1) {
                printf("%s: no such file or directory\n", args[1]);
            }
        }
        return 0;
    } else if (strcmp(command, "status") == 0) {
        if (last_signal_received != 0) {
            printf("terminated by signal %d\n", last_signal_received);
        } else {
            printf("exit value %d\n", last_exit_status);
        }
        return 0;
    } else {
        return -1;
    }
}

// Function to redirect input and output
void input_output_redirection(char** args, int numArgs, int* inputFile, int* outputFile) {
    int i=0;
    for ( i; i < numArgs; i++) {
        if (strcmp(args[i], "<") == 0) {
            *inputFile = open(args[i+1], O_RDONLY);
            if (*inputFile == -1) {
                perror("input file open");
                exit(1);
            }
            args[i] = NULL;
        } else if (strcmp(args[i], ">") == 0) {
            *outputFile = open(args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (*outputFile == -1) {
                perror("output file open");
                exit(1);
            }
            args[i] = NULL;
        }
    }
}
// Function to execute other (non-built-in) commands
void exec_other_commands(char* command, char* args[], int numArgs, int background) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) { // Child process
        int inputFile = -1;
        int outputFile = -1;
        input_output_redirection(args, numArgs, &inputFile, &outputFile);
        if (inputFile != -1) {
            if (dup2(inputFile, STDIN_FILENO) == -1) {
                fprintf(stderr, "Error cannot open file: %s for input\n", args[numArgs - 1]);
                exit(1);
            }
            close(inputFile);
        }
        if (outputFile != -1) {
            if (dup2(outputFile, STDOUT_FILENO) == -1) {
                perror("dup2");
                exit(1);
            }
            close(outputFile);
        }
        if (execvp(command, args) < 0) {
            printf("%s: command not found\n", command);
            exit(1);
        }
    } else { // Parent process
        if (!background) {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status)) {
                last_exit_status = WEXITSTATUS(status);
                last_signal_received = 0;
            } else if (WIFSIGNALED(status)) {
                last_signal_received = WTERMSIG(status);
                last_exit_status = 0;
            }
        } else {
            printf("background pid is %d\n", pid);
        }
    }
}


int main() {

    struct sigaction SIGINT_action = {{0}}, SIGTSTP_action = {{0}};
    SIGINT_action.sa_handler = handle_SIGINT;
    SIGTSTP_action.sa_handler = handle_SIGTSTP;
    sigfillset(&SIGINT_action.sa_mask);
    sigfillset(&SIGTSTP_action.sa_mask);
    SIGINT_action.sa_flags = SA_RESTART;
    SIGTSTP_action.sa_flags = SA_RESTART;

    sigaction(SIGINT, &SIGINT_action, NULL);
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    char command[MAX_COMMAND_LENGTH];
    pid_t bg_pid;
    int bg_status;

    while(1) {
        // Check for terminated background processes
        do {
            bg_pid = waitpid(-1, &bg_status, WNOHANG);
            if (bg_pid > 0) {
                printf("background pid %d is done: ", bg_pid);
                if (WIFEXITED(bg_status)) {
                    printf("exit value %d\n", WEXITSTATUS(bg_status));
                } else if (WIFSIGNALED(bg_status)) {
                    printf("terminated by signal %d\n", WTERMSIG(bg_status));
                }
            }
        } while (bg_pid > 0);
        
        printf(": ");
        fflush(stdout);
        fgets(command, MAX_COMMAND_LENGTH, stdin);
        execute_command(command);
    }

    return 0;
}


