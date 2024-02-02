#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

int untar(char* file, int remove, int suppress_output) {
    char* file_name = basename(file);
    char* path = dirname(file);

    // Construct the command
    size_t len = strlen("cd ") + strlen(path) + strlen(" && tar -xvhzvf ") + strlen(file_name) + strlen(" > /dev/null") + strlen(" && rm ") + strlen(file_name) + 1;
    char* command = malloc(len);
    strcpy(command, "cd ");
    strcat(command, path);
    strcat(command, " && tar -xvhzvf ");
    strcat(command, file_name);
    if (suppress_output) {
        strcat(command, " > /dev/null");
    }
    if (remove) {
        strcat(command, " && rm ");
        strcat(command, file_name);
    }

    return system(command);
}

int make_dir(char* dir) {
    size_t len = strlen("mkdir -p ") + strlen(dir) + 1;
    char* command = malloc(len);
    strcpy(command, "mkdir -p ");
    strcat(command, dir);

    return system(command);
}