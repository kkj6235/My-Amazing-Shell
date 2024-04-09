/**********************************************************************
 * Copyright (c) 2020-2024
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/stat.h>


/***********************************************************************
 * run_command()
 *
 * DESCRIPTION
 *   Implement the specified shell features here using the parsed
 *   command tokens.
 *
 * RETURN VALUE
 *   Return 1 on successful command execution
 *   Return 0 when user inputs "exit"
 *   Return <0 on error
 */
int run_command(int nr_tokens, char *tokens[]) {
    if (strcmp(tokens[0], "exit") == 0) return 0;
    int pipefd[2];
    int idx = 0;
    for (int i = 0; i < nr_tokens; i++) {
        if (!strcmp(tokens[i], "|")) {
            idx = i;
            if (pipe(pipefd) == -1) {
                perror("pipe 생성 중 에러");
                exit(EXIT_FAILURE);
            }
            break;
        }
    }
    pid_t pid;
    pid = fork();


    if (pid == 0) {
        if (idx) {
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);

            char **command_1;
            command_1 = (char **) malloc(sizeof(char *) * (idx + 1));
            for (int i = 0; i < idx; i++) {
                command_1[i] = strdup(tokens[i]);
            }
            command_1[idx] = NULL;

            execvp(command_1[0], command_1);
            return -1;

        } else {
            if (!strcmp(tokens[0], "cd")) {
                if (!strcmp(tokens[1], "~") || nr_tokens == 1) {
                    if (chdir(getenv("HOME")) == -1) {
                        return -1;
                    }
                } else {
                    if (chdir(tokens[1]) == -1) {
                        return -1;
                    }
                }
            }
            else{
                execvp(tokens[0], tokens);
            }
        }
    } else {
        if (idx) {
            close(pipefd[1]);
            pid_t pid2 = fork();
            if (pid2 == 0) {
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[0]);

                char **command_2;
                command_2 = (char **) malloc(sizeof(char *) * (nr_tokens - idx));
                for (int i = idx + 1; i < nr_tokens; i++) {
                    command_2[i - idx - 1] = strdup(tokens[i]);
                }
                command_2[nr_tokens - idx - 1] = NULL;
                execvp(command_2[0], command_2);
                strcpy(tokens[0], command_2[0]);
                return -1;
            }
            else{
                close(pipefd[0]);
                wait(NULL);
                wait(NULL);
            }
        }
        else{
            wait(NULL);
        }
    }
    return 1;
}


/***********************************************************************
 * initialize()
 *
 * DESCRIPTION
 *   Call-back function for your own initialization code. It is OK to
 *   leave blank if you don't need any initialization.
 *
 * RETURN VALUE
 *   Return 0 on successful initialization.
 *   Return other value on error, which leads the program to exit.
 */
int initialize(int argc, char *const argv[]) {
    return 0;
}


/***********************************************************************
 * finalize()
 *
 * DESCRIPTION
 *   Callback function for finalizing your code. Like @initialize(),
 *   you may leave this function blank.
 */
void finalize(int argc, char *const argv[]) {
}
