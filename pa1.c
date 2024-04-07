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

    pid_t pid;
    pid = fork();
    int state;

    if (pid == 0) {
        if (!strcmp(tokens[0], "cd")) {
            if (!strcmp(tokens[1], "~") || nr_tokens == 1) {
                if (chdir(getenv("HOME")) == -1) {
//                    fprintf(stderr, "-bash: cd: %s: No such file or directory\n", tokens[1]);
                    return -1;
                }
            } else {
                if (chdir(tokens[1]) == -1) {
//                    fprintf(stderr, "-bash: cd: %s: No such file or directory\n", tokens[1]);
                    return -1;
                }
            }

        } else if (!strcmp(tokens[0], "exit")) {
            return 0;
        } else {
            return execv(tokens[0], tokens);
        }
    } else {
        wait(&state);
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
