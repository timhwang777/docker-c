#ifndef _DOCKER_REGISTRY_H
#define _DOCKER_REGISTRY_H

char* docker_registry_auth(char* scope);
char** docker_enumerate_layers(char* token, char* repo, char* image, char* tag);
int docker_get_layer(char* token, char* dir, char* repo, char* image, char* id);
char *make_file_from_id(char *id);
char **parse_layers(char *response_content);
char *parse_token(char *response_content);

#endif