#ifndef CTHREADS_H
#define CTHREADS_H

#ifdef __unix__
#include <pthread.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

struct threading {
  #ifdef __unix__
  pthread_t pThread;
  pthread_mutex_t pMutex;
  pthread_key_t pKey;
  pthread_once_t pOnce;
  #endif
  #ifdef _WIN32
  HANDLE wThread;
  HANDLE wMutex;
  DWORD tlsIndex;
  int stopThread;
  #endif
};

struct thread_args {
  void *(*func)(void *data);
  void *data;
};

void cthreads_create_thread(struct threading *threading, void *(*func)(void *data), void *data);

void cthreads_close_thread(void *code);

void cthreads_wait_close_thread(struct threading *threading, void *code);

void cthreads_cancel_thread(struct threading *threading);

/* For Windows cancel support */
int cthreads_thread_cancelled(struct threading *threading);

void cthreads_key_create(struct threading *threading, size_t size);

void cthreads_key_retrieve(struct threading *threading, void *data);

void cthreads_key_delete(struct threading *threading);

void cthreads_once(struct threading *threading, void (*func)(void));

void cthreads_create_mutex(struct threading *threading);

void cthreads_mutex_lock(struct threading *threading);

void cthreads_mutex_unlock(struct threading *threading);

void cthreads_close_mutex(struct threading *threading);

#endif
