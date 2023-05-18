#ifndef MY_MEMORY_H
#define MY_MEMORY_H

#include <my_utils.h>
#include <stdlib.h>
#include <errno.h>

void *Malloc(size_t size)
{
    void *ptr = NULL;

    if ((errno = 0, ptr = malloc(size)) == NULL)
        Perror("malloc");

    return ptr;
}

void *Realloc(void * ptr, size_t size)
{
    if ((errno = 0, ptr = realloc(ptr, size)) == NULL)
        Perror("realloc");

    return ptr;
}

void Free(void *ptr)
{
    if (ptr != NULL)
        free(ptr);
}

#endif
