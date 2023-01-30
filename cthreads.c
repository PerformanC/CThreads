#include "cthreads.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef __unix__
#include <pthread.h>
#elif _WIN32
#include <windows.h>
#endif

#if __unix__
void *__cthreads_pthread_function_wrapper(void *data) {
  struct thread_args *args = data;
  args->func(args->data);

  free(data);

  return NULL;
}
#elif _WIN32
DWORD WINAPI __cthreads_winthreads_function_wrapper(void *data) {
  struct thread_args *args = data;
  args.func(args.data);

  free(data);

  return TRUE;
}
#endif

void cthreads_create_thread(struct threading *threading, void *(*func)(void *data), void *data) {
  #ifdef __unix__
    pthread_t thread;

    struct thread_args *args = malloc(sizeof(struct thread_args));
    args->func = func;
    args->data = data;

    pthread_create(&thread, NULL, __cthreads_pthread_function_wrapper, (void *)args);

    threading->pThread = thread;
  #elif _WIN32
    struct thread_args *args = malloc(sizeof(struct thread_args));
    args->func = func;
    args->data = data;

    HANDLE thread = CreateThread(NULL, 0, __cthreads_winthreads_function_wrapper, args, 0, NULL);

    threading->wThread = thread;
  #endif

  return;
}

void cthreads_close_thread(void *code) {
  #ifdef __unix__
    pthread_exit(code);
  #elif _WIN32
    ExitThread((DWORD)code);
  #endif

  return;
}

void cthreads_wait_close_thread(struct threading *threading, void *code) {
  #ifdef __unix__
    pthread_join(threading->pThread, &code);
  #elif _WIN32
    WaitForSingleObject(threading->wThread, INFINITE);
    GetExitCodeThread(threading->wThread, (LPDWORD)&code);
  #endif

  return;
}

void cthreads_cancel_thread(struct threading *threading) {
  #ifdef __unix__
    pthread_cancel(threading->pThread);
  #elif _WIN32
    threading->cancelThread = 1;
  #endif

  return;
}

/* For Windows cancel support */
int cthreads_thread_cancelled(struct threading *threading) {
  (void)threading;
  #ifdef _WIN32
    if (threading->cancelThread) return 1;
  #endif

  return 0;
}

void cthreads_detach_thread(struct threading *threading) {
  #ifdef __unix__
    pthread_detach(threading->pThread);
  #elif _WIN32
    CloseHandle(threading->wThread);
  #endif

  return;
}

void cthreads_key_create(struct threading *threading, size_t size) {
  #ifdef __unix__
    pthread_key_t key; (void)size;
    pthread_key_create(&key, NULL);

    threading->pKey = key;
  #elif _WIN32
    DWORD key = TlsAlloc();
    if (key == TLS_OUT_OF_INDEXES) ErrorExit("TlsAlloc failed");

    threading->wKey = key;

    LPVOID lpvData;
    lpvData = (LPVOID)LocalAlloc(LPTR, size); 
    if (!TlsSetValue(dwTlsIndex, lpvData)) ErrorExit("TlsSetValue error");
  #endif

  return;
}

void cthreads_key_retrieve(struct threading *threading, void *data) {
  #ifdef __unix__
    pthread_setspecific(threading->pKey, data);
  #elif _WIN32
    LPVOID lpvData = TlsGetValue(threading->wKey);
    if (lpvData == NULL) ErrorExit("TlsGetValue failed");

    LPVOID lpvData = TlsGetValue(threading->tlsIndex);
    if ((lpvData == 0) && (GetLastError() != ERROR_SUCCESS)) ErrorExit("TlsGetValue error"); 

    memcpy(lpvData, data, sizeof(data));
  #endif

  return;
}

void cthreads_key_delete(struct threading *threading) {
  #ifdef __unix__
    pthread_key_delete(threading->pKey);
  #elif _WIN32
    TlsFree(threading->wKey);
  #endif

  return;
}

#ifdef _WIN32
BOOL CALLBACK __cthreads_winthreads_function_wrapper_once(PINIT_ONCE InitOnce, PVOID Parameter, PVOID *lpContext) {
  void (*func)(void) = Parameter;
  func();

  return TRUE;
}
#endif

void cthreads_once(struct threading *threading, void (*func)(void)) {
  #ifdef __unix__
    threading->pOnce = PTHREAD_ONCE_INIT;
    pthread_once(&threading->pOnce, func);
  #elif _WIN32
    INIT_ONCE initOnce = INIT_ONCE_STATIC_INIT;
    InitOnceExecuteOnce(&initOnce, __cthreads_winthreads_function_wrapper_once, (void *)&threading, NULL);
  #endif

  return;
}

void cthreads_create_mutex(struct threading *threading) {
  #ifdef __unix__
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    threading->pMutex = mutex;
  #elif _WIN32
    HANDLE mutex = CreateMutex(NULL, FALSE, NULL);

    threading->wMutex = mutex;
  #endif

  return;
}

void cthreads_mutex_lock(struct threading *threading) {
  #ifdef __unix__
    pthread_mutex_lock(&threading->pMutex);
  #elif _WIN32
    WaitForSingleObject(threading->wMutex, INFINITE);
  #endif

  return;
}

void cthreads_mutex_unlock(struct threading *threading) {
  #ifdef __unix__
    pthread_mutex_unlock(&threading->pMutex);
  #elif _WIN32
    ReleaseMutex(threading->wMutex);
  #endif

  return;
}

void cthreads_close_mutex(struct threading *threading) {
  #ifdef __unix__
    pthread_mutex_destroy(&threading->pMutex);
  #elif _WIN32
    CloseHandle(threading->wMutex);
  #endif

  return;
}
