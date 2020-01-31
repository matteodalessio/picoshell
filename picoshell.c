#define _POSIX_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define BUF_MAX 512
#define VERSION "0.1"

// Holds child PID, or 0 for the child itself, used in sighandler
pid_t child_pid;

// Handle env variables, more portable than envp
extern char **environ;

// Used to avoid quitting the shell if a child program is interrupted
void catch_sigint(int signo);
char cwd[1024];

int main(int argc, const char * * argv) {
    char *command, *saved[BUF_MAX], buffer[BUF_MAX];
    int status, i, path;
    
    // No need to use sigaction, it's called once
    signal(SIGINT, catch_sigint);
    
    printf("Operating systems project - 2016 \n", VERSION);
    printf("Type 'help' or 'h' if you get stuck\n");
    
    // Parser-loop
    while (1) {
        child_pid = 0;
        printf("~ : ");
        
        // Read line
        fgets(buffer, BUF_MAX, stdin);
        
        if (strcmp(buffer, "help\n") == 0 || strcmp(buffer, "h\n") == 0  ) {
            // Print help
            printf("  exit      - exit minishel \n");
            printf("  printenv  - print environment variables \n");
            printf("  help      - help message \n");
        } else
            if (strcmp(buffer, "exit\n") == 0)
                break;
            else if (strcmp(buffer, "printenv\n") == 0) {
                    // Print environment variables
                    int i;
                    for (i = 0; environ[i]; i++)
                        printf("  %s\n", environ[i]);
                } else {
                    // Reading stops after newline
                    command = strtok(buffer, " \n");
                    int i = 0;
                    
                    while (command != NULL) {
                        saved[i] = command;
                        i++;
                        command = strtok(NULL, " \n");
                    }
                    // Preventing bad address
                    saved[i] = NULL;
                    if (saved[0] == NULL)
                        continue;

                    if (strcmp(saved[0], "cd") == 0) {
                        int path = chdir(saved[1]);
                        if (path != 0)
                            perror("");
                    } else {
                        // child process
                        if ((child_pid = fork()) == 0) {
                            execvp(saved[0], saved);
                            perror("");
                            exit(-1);
                        } else {
                            wait(&status);
                            if (!WIFEXITED(status)) 
                                printf("Process terminated abnormally\n");
                        }
                    }
                }
             }
    return 0;
}

void catch_sigint(int signo) {
    printf("\n");
    
    // Interrupt child
    if (child_pid > 0)
        kill(child_pid, SIGINT);
    else
        exit(0);
}
