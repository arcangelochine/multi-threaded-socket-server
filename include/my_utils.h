#ifndef MY_UTILS_H
#define MY_UTILS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

#include <math.h>

void Perror(const char *errname)
{
    fprintf(stderr, "Errore: %s [%s]\n", errname, strerror(errno));
    exit(EXIT_FAILURE);
}

void Fopen(const char *filename, const char *mode, FILE **file)
{
    if ((errno = 0, *file = fopen(filename, mode)) == NULL)
        Perror("fopen");
}

void Frewind(FILE *file)
{
    if ((errno = 0, fseek(file, 0, SEEK_SET)) == -1)
        Perror("fseek");
}

void Fclose(FILE *file)
{
    if ((errno = 0, fclose(file)) == -1)
        Perror("fclose");
}

void Close(int fd)
{
    errno = 0;
    if (close(fd) == -1)
        Perror("close");
}

pid_t Fork(void)
{   
    pid_t pid;

    if ((errno = 0, pid = fork()) == -1)
        Perror("fork");

    return pid;
}

#endif