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
#include <stdbool.h>

#include <sys/wait.h>
#include <sys/stat.h>


char *name[20], *value[20];
int nr_alias=0;

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
    fflush(stdout);
    pid_t pid;
    int state;
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
            if(nr_alias!=0){
                for(int i=0;i<idx;i++){
                    for(int j=0;j<nr_alias;j++){
                        if(!strcmp(command_1[i],name[j])){
                            free(command_1[i]);
                            command_1[i] = (char *)malloc(strlen(value[j]) + 1);
                            strcpy(command_1[i], value[j]);
                            break;
                        }
                    }
                }
            }

            execvp(command_1[0], command_1);
            return -1;
        }
        else {
            // 파이프 아닌 경우
            if (!strcmp(tokens[0], "cd")) {
                if(nr_tokens==1){
                    if (chdir(getenv("HOME")) == -1) {
                        return -1;
                    }
                    return 1;
                }
                else if (!strcmp(tokens[1], "~")) {
                    if (chdir(getenv("HOME")) == -1) {
                        return -1;
                    }
                    return 1;
                }
                else {
                    if (chdir(tokens[1]) == -1) {
                        return -1;
                    }
                    return 1;
                }
            }
            else if(!strcmp(tokens[0], "alias")){
                if(nr_tokens==1){
                    for(int i=0;i<nr_alias;i++){
                        fprintf(stderr,"%s: %s\n",name[i],value[i]);
                    }
                    return 1;
                }
                else {
                    char temp[128];
                    name[nr_alias] = (char *) malloc(sizeof(char) * (strlen(tokens[1])+1));
                    strcpy(name[nr_alias], tokens[1]);

                    strcpy(temp, tokens[2]);
                    for(int i=3;i<nr_tokens;i++){
                        strcat(temp, " ");
                        strcat(temp, tokens[i]);
                    }

                    value[nr_alias] =(char *) malloc(sizeof(char) * (strlen(temp) + 1));
                    strcpy(value[nr_alias], temp);

                    nr_alias++;
                    return 1;
                }
            }
            else if(!strcmp(tokens[0], "echo")){
                if(nr_alias!=0){
                    for(int i=0;i<nr_tokens;i++){
                        for(int j=0;j<nr_alias;j++){
                            if(!strcmp(tokens[i],name[j])){
                                free(tokens[i]);
                                tokens[i] = (char *)malloc(strlen(value[j]) + 1);
                                strcpy(tokens[i], value[j]);
                                break;
                            }
                        }
                    }
                }

            }

            execvp(tokens[0], tokens);

            return -1;
        }
    } else {
        if (idx) {
            pid_t pid2 = fork();
            if (pid2 == 0) {
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);


                char **command_2;
                command_2 = (char **) malloc(sizeof(char *) * (nr_tokens - idx));
                for (int i = idx + 1; i < nr_tokens; i++) {
                    command_2[i - idx - 1] = strdup(tokens[i]);
                }
                command_2[nr_tokens - idx - 1] = NULL;
                if(nr_alias!=0){
                    for(int i=0;i<nr_tokens-idx-1;i++){
                        for(int j=0;j<nr_alias;j++){
                            if(!strcmp(command_2[i],name[j])){
                                free(command_2[i]);

                                int k=i;
                                char *token;
                                token = strtok(value[j], " ");

                                while(token!=NULL){
                                    command_2[k] = (char *)malloc(strlen(token) + 1);
                                    strcpy(command_2[k++], token);
                                    token = strtok(NULL, " ");
                                }
                                break;
                            }
                        }
                    }

                }
                execvp(command_2[0], command_2);
                strcpy(tokens[0], command_2[0]);
                return -22;
            }
            else{
                close(pipefd[1]);
                close(pipefd[0]);
                waitpid(pid,&state,0);
                waitpid(pid2,&state,0);

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
