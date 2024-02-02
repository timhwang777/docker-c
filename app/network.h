#ifndef _NETWORK_H
#define _NETWORK_H

#include <stdio.h>
#include <stdlib.h>

char* get_response(char* uri, char* bearer_token);
int download_file(char* uri, char* file, char* bearer_token);
size_t write_handler_mem(void* data, size_t size, size_t num_mem, void* arg);
size_t write_handler_file(void* contents, size_t size, size_t num_items, FILE* file);

#endif