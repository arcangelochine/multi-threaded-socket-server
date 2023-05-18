#ifndef MY_DIR_H
#define MY_DIR_H

#include <my_utils.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

DIR *Dopen(const char *dirname)
{
    DIR *dirptr = NULL;

    if ((errno = 0, dirptr = opendir(dirname)) == NULL)
        Perror("opendir");

    return dirptr;
}

void Dclose(DIR *dirptr)
{
    if ((errno = 0, closedir(dirptr)) == -1)
        Perror("closedir");
}

struct dirent *Dread(DIR *dirptr)
{
    struct dirent *entry = NULL;

    if ((errno = 0, entry = readdir(dirptr)) == NULL && errno > 0)
        Perror("readdir");

    return entry;
}

void Drewind(DIR *dirptr)
{
    errno = 0;
    rewinddir(dirptr);
}

/**
 * @brief Tells if a directory does not contain any file
 * @warning Must rewind after calling this function on a DIR *
*/
unsigned char Dempty(DIR *dir)
{
    Drewind(dir);

    struct dirent *entry;

    while ((entry = Dread(dir)) != NULL)
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            return 0;

    return 1;
}

unsigned char is_dir(char *path)
{
    struct stat filestat;

    stat(path, &filestat);
    
    return S_ISDIR(filestat.st_mode);
}

unsigned char is_reg(char *path)
{
    struct stat filestat;

    stat(path, &filestat);
    
    return S_ISREG(filestat.st_mode);
}

#endif