#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>

size_t strlen(const char*);;
int strcmp(char*, char*);
int strncmp(char*, char*, size_t);
int strcpy(char*, char*);
char* strchr(char*, int);
char* strdup(const char*);
int memcpy(void *, const void*, size_t);


#endif
