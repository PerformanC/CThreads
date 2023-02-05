#ifndef CTHREADS_H
#define CTHREADS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

struct cthreads_args {
  void *(*func)(void *data);
  void *data;
};

#if __unix__
#include <pthread.h>
void *__cthreads_pthread_function_wrapper(void *data) {
  struct cthreads_args *args = data;
  args->func(args->data);

  free(data);

  return NULL;
}
#elif _WIN32
#include <windows.h>
DWORD WINAPI __cthreads_winthreads_function_wrapper(void *data) {
  struct thread_args *args = data;
  args.func(args.data);

  free(data);

  return TRUE;
}
#endif

struct cthreads_thread {
    #ifdef __unix__
    pthread_t pThread;
    #elif _WIN32
    HANDLE wThread;
    #endif
};

struct cthreads_mutex {
    #ifdef __unix__
    pthread_mutex_t pMutex;
    #elif _WIN32
    HANDLE wMutex;
    #endif
};

struct cthreads_cond {
    #ifdef __unix__
    pthread_cond_t pCond;
    #elif _WIN32
    HANDLE wCond;
    #endif
};

int cthreads_thread_create(struct cthreads_thread *thread, void *(*func)(void *data), void *data) {
  #ifdef __unix__
    pthread_t pthread;
    int res;

    struct cthreads_args *args = malloc(sizeof(struct cthreads_args));
    args->func = func;
    args->data = data;

    res = pthread_create(&pthread, NULL, __cthreads_pthread_function_wrapper, (void *)args);

    thread->pThread = pthread;

    return res;
  #elif _WIN32
    struct cthreads_args *args = malloc(sizeof(struct cthreads_args));
    args->func = func;
    args->data = data;

    HANDLE thread = CreateThread(NULL, 0, __cthreads_winthreads_function_wrapper, args, 0, NULL);

    threading->wThread = thread;

    return 0;
  #endif
}

void cthreads_thread_close(void *code) {
  #ifdef __unix__
    pthread_exit(code);
  #elif _WIN32
    ExitThread((DWORD)code);
  #endif

  return;
}

int cthreads_mutex_init(struct cthreads_mutex *mutex) {
  #ifdef __unix__
    pthread_mutex_t pMutex;
    int res = pthread_mutex_init(&pMutex, NULL);

    mutex->pMutex = pMutex;

    return res;
  #elif _WIN32
    HANDLE wMutex = CreateMutex(NULL, FALSE, NULL);

    mutex->wMutex = wMutex;

    return 0;
  #endif
}

int cthreads_mutex_lock(struct cthreads_mutex *mutex) {
  #ifdef __unix__
    return pthread_mutex_lock(&mutex->pMutex);
  #elif _WIN32
    WaitForSingleObject(mutex->wMutex, INFINITE);

    return 0;
  #endif
}

int cthreads_mutex_trylock(struct cthreads_mutex *mutex) {
  #ifdef __unix__
    return pthread_mutex_trylock(&mutex->pMutex);
  #elif _WIN32
    return WaitForSingleObject(mutex->wMutex, 0);
  #endif
}

int cthreads_mutex_unlock(struct cthreads_mutex *mutex) {
  #ifdef __unix__
    return pthread_mutex_unlock(&mutex->pMutex);
  #elif _WIN32
    ReleaseMutex(mutex->wMutex);

    return 0;
  #endif
}

void cthreads_mutex_destroy(struct cthreads_mutex *mutex) {
  #ifdef __unix__
    pthread_mutex_destroy(&mutex->pMutex);
  #elif _WIN32
    CloseHandle(mutex->wMutex);
  #endif
}

int cthreads_cond_init(struct cthreads_cond *cond) {
  #ifdef __unix__
    pthread_cond_t pCond;
    int res = pthread_cond_init(&pCond, NULL);

    cond->pCond = pCond;

    return res;
  #elif _WIN32
    HANDLE wCond = CreateEvent(NULL, FALSE, FALSE, NULL);

    cond->wCond = wCond;

    return 0;
  #endif
}

int cthreads_cond_signal(struct cthreads_cond *cond) {
  #ifdef __unix__
    return pthread_cond_signal(&cond->pCond);
  #elif _WIN32
    SetEvent(cond->wCond);

    return 0;
  #endif
}

int cthreads_cond_broadcast(struct cthreads_cond *cond) {
  #ifdef __unix__
    return pthread_cond_broadcast(&cond->pCond);
  #elif _WIN32
    SetEvent(cond->wCond);

    return 0;
  #endif
}

int cthreads_cond_destroy(struct cthreads_cond *cond) {
  #ifdef __unix__
    return pthread_cond_destroy(&cond->pCond);
  #elif _WIN32
    CloseHandle(cond->wCond);

    return 0;
  #endif
}

int cthreads_cond_wait(struct cthreads_cond *cond, struct cthreads_mutex *mutex) {
  #ifdef __unix__
    return pthread_cond_wait(&cond->pCond, &mutex->pMutex);
  #elif _WIN32
    WaitForSingleObject(cond->wCond, INFINITE);

    return 0;
  #endif
}

int cthreads_wait_close_thread(struct cthreads_thread *thread, void *code) {
  #ifdef __unix__
    return pthread_join(thread->pThread, &code);
  #elif _WIN32
    WaitForSingleObject(thread->wThread, INFINITE);
    GetExitCodeThread(thread->wThread, (LPDWORD)&code);

    return 0;
  #endif
}

#ifdef __cplusplus
}
#endif

#endif
