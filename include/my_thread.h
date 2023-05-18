#ifndef MY_THREAD_H
#define MY_THREAD_H

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned char uint8;

typedef pthread_t Thread;
typedef pthread_mutex_t Mutex;
typedef pthread_cond_t Cond;

int terrno;

static void Tperror(const char *errname)
{
    fprintf(stderr, "Errore: %s [%s]\n", errname, strerror(terrno));
    exit(1);
}

void TCreate(Thread *tid, void *(*fn)(void *), void *arg)
{
    if ((terrno = pthread_create(tid, NULL, fn, arg)))
        Tperror("pthread_create");
}

void Join(Thread tid, void **buffer)
{
    if ((terrno = pthread_join(tid, buffer)))
        Tperror("pthread_join");
}

void MInit(Mutex *mutex)
{
    if ((terrno = pthread_mutex_init(mutex, NULL)))
        Tperror("pthread_mutex_init");
}

void Lock(Mutex *mutex)
{
    if ((terrno = pthread_mutex_lock(mutex)))
        Tperror("pthread_mutex_lock");
}

void Unlock(Mutex *mutex)
{
    if ((terrno = pthread_mutex_unlock(mutex)))
        Tperror("pthread_mutex_unlock");
}

void MDestroy(Mutex *mutex)
{
    if ((terrno = pthread_mutex_destroy(mutex)))
        Tperror("pthread_mutex_destroy");
}

void CInit(Cond *cond)
{
    if ((terrno = pthread_cond_init(cond, NULL)))
        Tperror("pthread_cond_init");
}

void Wait(Cond *cond, Mutex *mutex)
{
    if ((terrno = pthread_cond_wait(cond, mutex)))
        Tperror("pthread_cond_wait");
}

void Signal(Cond *cond)
{
    if ((terrno = pthread_cond_signal(cond)))
        Tperror("pthread_cond_signal");
}

void Broadcast(Cond *cond)
{
    if ((terrno = pthread_cond_broadcast(cond)))
        Tperror("pthread_cond_broadcast");
}

void CDestroy(Cond *cond)
{
    if ((terrno = pthread_cond_destroy(cond)))
        Tperror("pthread_cond_destroy");
}

#endif