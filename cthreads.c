#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef _WIN32
  #include <errno.h>  /* errno */
  #include <string.h> /* strerror(), strlen() */
#endif

#include "cthreads.h"

#ifdef _WIN32
#include <windows.h>
DWORD WINAPI __cthreads_winthreads_function_wrapper(void *data) {
  struct cthreads_args *args = data;
  return (DWORD)(uintptr_t)args->func(args->data);
}
#else
#include <pthread.h>
#endif

int cthreads_thread_create(struct cthreads_thread *thread, struct cthreads_thread_attr *attr, void *(*func)(void *data), void *data, struct cthreads_args *args) {
  #ifdef CTHREADS_DEBUG
    puts("cthreads_thread_create");
  #endif

  /* INFO: POSIX path may not use it, but we must guarantee symmetry */
  if (!args) return 1;

  #ifdef _WIN32
    args->func = func;
    args->data = data;

    DWORD tid;
    if (attr) {
      thread->wThread = CreateThread(NULL, attr->stacksize ? attr->stacksize : 0,
                                     __cthreads_winthreads_function_wrapper, args,
                                     attr->dwCreationFlags ? (DWORD)attr->dwCreationFlags : 0, &tid);
    } else {
      thread->wThread = CreateThread(NULL, 0, __cthreads_winthreads_function_wrapper, args, 0, &tid);
    }

    /* INFO: If successful, write tid for later access */
    if (thread->wThread) thread->wThreadId = tid;

    return thread->wThread == NULL;
  #else
    pthread_attr_t pAttr;
    if (attr) {
      if (pthread_attr_init(&pAttr)) return 1;

      int ret = 0;
      if (attr->detachstate) ret = pthread_attr_setdetachstate(&pAttr, attr->detachstate);
      if (ret == 0 && attr->guardsize) ret = pthread_attr_setguardsize(&pAttr, attr->guardsize);
      #ifdef CTHREADS_THREAD_INHERITSCHED
        if (ret == 0 && attr->inheritsched) ret = pthread_attr_setinheritsched(&pAttr, attr->inheritsched);
      #endif
      if (ret == 0 && attr->schedpolicy) ret = pthread_attr_setschedpolicy(&pAttr, attr->schedpolicy);
      if (ret == 0 && attr->scope) ret = pthread_attr_setscope(&pAttr, attr->scope);
      #ifdef CTHREADS_THREAD_STACK
        if (ret == 0 && attr->stack) ret = pthread_attr_setstack(&pAttr, attr->stackaddr, attr->stack);
        /* INFO: Using both _setstack and _setstacksize is disallowed by POSIX */
        #ifdef CTHREADS_THREAD_STACKADDR
        else
        #endif
      #endif
      #ifdef CTHREADS_THREAD_STACKADDR
        if (ret == 0 && attr->stacksize) ret = pthread_attr_setstacksize(&pAttr, attr->stacksize);
      #endif

      if (ret) {
        pthread_attr_destroy(&pAttr);

        return 1;
      }
    }

    int ret = pthread_create(&thread->pThread, attr ? &pAttr : NULL, func, data);
    if (attr) pthread_attr_destroy(&pAttr);

    return ret;
  #endif
}

int cthreads_thread_detach(struct cthreads_thread thread) {
  #ifdef CTHREADS_DEBUG
    puts("cthreads_thread_detach");
  #endif

  #ifdef _WIN32
    return CloseHandle(thread.wThread) == 0;
  #else
    return pthread_detach(thread.pThread);
  #endif
}

int cthreads_thread_join(struct cthreads_thread thread, void *code) {
  #ifdef CTHREADS_DEBUG
    puts("cthreads_thread_join");
  #endif

  #ifdef _WIN32
    HANDLE handle = thread.wThread;
    int opened = 0;

    if (!handle) {
      handle = OpenThread(THREAD_TERMINATE, FALSE, thread.wThreadId);
      if (!handle) return 1;

      opened = 1;
    }
  
    if (WaitForSingleObject(handle, INFINITE) == WAIT_FAILED) {
      if (opened) CloseHandle(handle);

      return 1;
    }

    DWORD exit_code;
    if (!GetExitCodeThread(handle, (LPDWORD)&exit_code)) {
      if (opened) CloseHandle(handle);

      return 1;
    }

    CloseHandle(handle);

    if (code) *(void **)code = (void *)(uintptr_t)exit_code;

    return cthreads_thread_detach(thread);
  #else
    return pthread_join(thread.pThread, code ? (void **)code : NULL);
  #endif
}

int cthreads_thread_equal(struct cthreads_thread thread1, struct cthreads_thread thread2) {
  #ifdef CTHREADS_DEBUG
    puts("cthreads_thread_equal");
  #endif

  #ifdef _WIN32
    return thread1.wThreadId == thread2.wThreadId;
  #else
    return pthread_equal(thread1.pThread, thread2.pThread);
  #endif
}

struct cthreads_thread cthreads_thread_self(void) {
  struct cthreads_thread t;

  #ifdef CTHREADS_DEBUG
    puts("cthreads_thread_self");
  #endif

  #ifdef _WIN32
    /* INFO: No real handle, only the ID. */
    t.wThread = NULL;
    t.wThreadId = GetCurrentThreadId();
  #else
    t.pThread = pthread_self();
  #endif

  return t;
}

/*
   INFO: This is a best-effort implementation on POSIX systems. There is no
           reliable way to get the thread ID, as pthread_t is an opaque type.

         On Windows we return the stable thread ID stored at creation.
*/
unsigned long cthreads_thread_id(struct cthreads_thread thread) {
  #ifdef CTHREADS_DEBUG
    puts("cthreads_thread_id");
  #endif

  #ifdef _WIN32
    return (unsigned long)thread.wThreadId;
  #else
    return (unsigned long)(uintptr_t)thread.pThread;
  #endif
}

void cthreads_thread_exit(void *code) {
  #ifdef CTHREADS_DEBUG
    puts("cthreads_thread_exit");
  #endif

  #ifdef _WIN32
    /* INFO: On Windows 64-bit, we cannot losslessly convert a pointer to a DWORD */
    ExitThread((DWORD)(uintptr_t)code);
  #else
    pthread_exit(code);
  #endif
}

int cthreads_thread_cancel(struct cthreads_thread thread) {
  #ifdef CTHREADS_DEBUG
    puts("cthreads_thread_cancel");
  #endif

  #ifdef _WIN32
    HANDLE handle = thread.wThread;
    int opened = 0;

    if (!handle) {
      handle = OpenThread(THREAD_TERMINATE, FALSE, thread.wThreadId);
      if (!handle) return 1;

      opened = 1;
    }

    BOOL ret = TerminateThread(handle, 0);
    if (opened) CloseHandle(handle);

    return ret == 0;
  #else
    return pthread_cancel(thread.pThread);
  #endif
}

#ifdef CTHREADS_MUTEX_ATTR
  int cthreads_mutex_init(struct cthreads_mutex *mutex, struct cthreads_mutex_attr *attr) {
#else
  int cthreads_mutex_init(struct cthreads_mutex *mutex, void *attr) {
#endif
  #ifdef _WIN32
    #ifdef CTHREADS_DEBUG
      puts("cthreads_mutex_init");
    #endif

    (void) attr;

    InitializeCriticalSection(&mutex->wMutex);

    return 0;
  #else
    pthread_mutexattr_t pAttr;

    #ifdef CTHREADS_DEBUG
      puts("cthreads_mutex_init");
    #endif
  
    /* CTHREADS_MUTEX_ATTR is always available on non-Windows platforms */
    if (attr) {
      if (pthread_mutexattr_init(&pAttr)) return 1;

      int ret = 0;
      if (attr->pshared) ret = pthread_mutexattr_setpshared(&pAttr, attr->pshared);
      #ifdef CTHREADS_MUTEX_TYPE
        if (ret == 0 && attr->type) ret = pthread_mutexattr_settype(&pAttr, attr->type);
      #endif
      #ifdef CTHREADS_MUTEX_ROBUST
        if (ret == 0 && attr->robust) ret = pthread_mutexattr_setrobust(&pAttr, attr->robust);
      #endif
      #ifdef CTHREADS_MUTEX_PROTOCOL
        if (ret == 0 && attr->protocol) ret = pthread_mutexattr_setprotocol(&pAttr, attr->protocol);
      #endif
      #ifdef CTHREADS_MUTEX_PRIOCEILING
        if (ret == 0 && attr->prioceiling) ret = pthread_mutexattr_setprioceiling(&pAttr, attr->prioceiling);
      #endif

      if (ret) {
        pthread_mutexattr_destroy(&pAttr);

        return 1;
      }
    }

    int ret = pthread_mutex_init(&mutex->pMutex, attr ? &pAttr : NULL);
    if (attr) pthread_mutexattr_destroy(&pAttr);

    return ret;
  #endif
}

int cthreads_mutex_lock(struct cthreads_mutex *mutex) {
  #ifdef CTHREADS_DEBUG
    puts("cthreads_mutex_lock");
  #endif

  #ifdef _WIN32
    EnterCriticalSection(&mutex->wMutex);

    return 0;
  #else
    return pthread_mutex_lock(&mutex->pMutex);
  #endif
}

int cthreads_mutex_trylock(struct cthreads_mutex *mutex) {
  #ifdef CTHREADS_DEBUG
    puts("cthreads_mutex_trylock");
  #endif

  #ifdef _WIN32
    return TryEnterCriticalSection(&mutex->wMutex) == 0;
  #else
    return pthread_mutex_trylock(&mutex->pMutex);
  #endif
}

int cthreads_mutex_unlock(struct cthreads_mutex *mutex) {
  #ifdef CTHREADS_DEBUG
    puts("cthreads_mutex_unlock");
  #endif

  #ifdef _WIN32
    LeaveCriticalSection(&mutex->wMutex);

    return 0;
  #else
    return pthread_mutex_unlock(&mutex->pMutex);
  #endif
}

int cthreads_mutex_destroy(struct cthreads_mutex *mutex) {
  #ifdef CTHREADS_DEBUG
    puts("cthreads_mutex_destroy");
  #endif

  #ifdef _WIN32
    DeleteCriticalSection(&mutex->wMutex);

    return 0;
  #else
    return pthread_mutex_destroy(&mutex->pMutex);
  #endif
}

#ifdef CTHREADS_COND_ATTR
  int cthreads_cond_init(struct cthreads_cond *cond, struct cthreads_cond_attr *attr) {
#else
  int cthreads_cond_init(struct cthreads_cond *cond, void *attr) {
#endif
  #ifdef CTHREADS_DEBUG
    puts("cthreads_cond_init");
  #endif

  #ifdef _WIN32
    (void) attr;

    InitializeConditionVariable(&cond->wCond);

    return 0;
  #else
    pthread_condattr_t pAttr;

    /* CTHREADS_COND_ATTR is always available on non-Windows platforms */
    if (attr) {
      if (pthread_condattr_init(&pAttr) != 0) return 1;

      int ret = 0;
      if (attr->pshared) ret = pthread_condattr_setpshared(&pAttr, attr->pshared);
      #ifdef CTHREADS_COND_CLOCK
        if (ret == 0 && attr->clock) ret = pthread_condattr_setclock(&pAttr, attr->clock);
      #endif

      if (ret) {
        pthread_condattr_destroy(&pAttr);

        return 1;
      }
    }

    #ifdef CTHREADS_COND_CLOCK
      cond->clock = (attr && attr->clock) ? attr->clock : CLOCK_REALTIME;
    #endif

    int ret = pthread_cond_init(&cond->pCond, attr ? &pAttr : NULL);
    if (attr) pthread_condattr_destroy(&pAttr);

    return ret;
  #endif
}

int cthreads_cond_signal(struct cthreads_cond *cond) {
  #ifdef CTHREADS_DEBUG
    puts("cthreads_cond_signal");
  #endif

  #ifdef _WIN32
    WakeConditionVariable(&cond->wCond);

    return 0;
  #else
    return pthread_cond_signal(&cond->pCond);
  #endif
}

int cthreads_cond_broadcast(struct cthreads_cond *cond) {
  #ifdef CTHREADS_DEBUG
    puts("cthreads_cond_broadcast");
  #endif

  #ifdef _WIN32
    WakeAllConditionVariable(&cond->wCond);

    return 0;
  #else
    return pthread_cond_broadcast(&cond->pCond);
  #endif
}

int cthreads_cond_destroy(struct cthreads_cond *cond) {
  #ifdef CTHREADS_DEBUG
    puts("cthreads_cond_destroy");
  #endif

  #ifdef _WIN32
    return 0;
  #else
    return pthread_cond_destroy(&cond->pCond);
  #endif
}

int cthreads_cond_wait(struct cthreads_cond *cond, struct cthreads_mutex *mutex) {
  #ifdef CTHREADS_DEBUG
    puts("cthreads_cond_wait");
  #endif

  #ifdef _WIN32
    return SleepConditionVariableCS(&cond->wCond, &mutex->wMutex, INFINITE) == 0;
  #else
    return pthread_cond_wait(&cond->pCond, &mutex->pMutex);
  #endif
}

int cthreads_cond_timedwait(struct cthreads_cond *cond, struct cthreads_mutex *mutex, unsigned int ms) {
  #ifdef CTHREADS_DEBUG
    puts("cthreads_cond_wait");
  #endif

  #ifdef _WIN32
    return SleepConditionVariableCS(&cond->wCond, &mutex->wMutex, (DWORD)ms) == 0;
  #else
    struct timespec ts;
    #ifdef CTHREADS_COND_CLOCK
      if (clock_gettime(cond->clock, &ts)) return 1;
    #else
      if (clock_gettime(CLOCK_REALTIME, &ts)) return 1;
    #endif

    ts.tv_sec += ms / 1000;
    ts.tv_nsec += (ms % 1000) * 1000000;
    ts.tv_sec += ts.tv_nsec / 1000000000;
    ts.tv_nsec %= 1000000000;

    return pthread_cond_timedwait(&cond->pCond, &mutex->pMutex, &ts);
  #endif
}

#ifdef CTHREADS_RWLOCK
  int cthreads_rwlock_init(struct cthreads_rwlock *rwlock) {
    #ifdef CTHREADS_DEBUG
      puts("cthreads_rwlock_init");
    #endif

    #ifdef _WIN32
      rwlock->type = 0;
      rwlock->wRWLock = malloc(sizeof(SRWLOCK));
      if (!rwlock->wRWLock) return 1;

      InitializeSRWLock(rwlock->wRWLock);

      return 0;
    #else
      return pthread_rwlock_init(&rwlock->pRWLock, NULL);
    #endif
  }

  int cthreads_rwlock_rdlock(struct cthreads_rwlock *rwlock) {
    #ifdef CTHREADS_DEBUG
      puts("cthreads_rwlock_rdlock");
    #endif

    #ifdef _WIN32
      AcquireSRWLockShared(rwlock->wRWLock);
      rwlock->type = 1;

      return 0;
    #else
      return pthread_rwlock_rdlock(&rwlock->pRWLock);
    #endif
  }

  int cthreads_rwlock_unlock(struct cthreads_rwlock *rwlock) {
    #ifdef CTHREADS_DEBUG
      puts("cthreads_rwlock_unlock");
    #endif

    #ifdef _WIN32
      switch (rwlock->type) {
        case 1: {
          ReleaseSRWLockShared(rwlock->wRWLock);

          break;
        }
        case 2: {
          ReleaseSRWLockExclusive(rwlock->wRWLock);

          break;
        }
      }

      return 0;
    #else
      return pthread_rwlock_unlock(&rwlock->pRWLock);
    #endif
  }

  int cthreads_rwlock_wrlock(struct cthreads_rwlock *rwlock) {
    #ifdef CTHREADS_DEBUG
      puts("cthreads_rwlock_wrlock");
    #endif

    #ifdef _WIN32
      AcquireSRWLockExclusive(rwlock->wRWLock);
      rwlock->type = 2;

      return 0;
    #else
      return pthread_rwlock_wrlock(&rwlock->pRWLock);
    #endif
  }

  int cthreads_rwlock_destroy(struct cthreads_rwlock *rwlock) {
    #ifdef CTHREADS_DEBUG
      puts("cthreads_rwlock_destroy");
    #endif

    #ifdef _WIN32
      free(rwlock->wRWLock);
      rwlock->wRWLock = NULL;
      rwlock->type = 0;

      return 0;
    #else
      return pthread_rwlock_destroy(&rwlock->pRWLock);
    #endif
  }
#endif

int cthreads_error_code(void) {
  #ifdef CTHREADS_DEBUG
    puts("cthreads_error_code");
  #endif

  #ifdef _WIN32
    return GetLastError();
  #else
    return errno;
  #endif
}

size_t cthreads_error_string(int error_code, char *buf, size_t length) {
  #ifdef CTHREADS_DEBUG
    puts("cthreads_error_string");
  #endif

  if (length == 0) return 0;

  #ifdef _WIN32
    LPSTR error_str = NULL;

    /* 
      INFO: The string that is written also contains a \n, which we must ignore, besides the
              NULL terminator.
    */
    DWORD format_ret = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                      NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&error_str, 0, NULL);
    if (format_ret == 0) {
      strncpy(buf, "Unknown error", length - 1);
      buf[length - 1] = '\0';

      return length - 1;
    }

    size_t error_str_len = (size_t)format_ret;

    /* INFO: Remove trailing newline and carriage return */
    while (error_str_len > 0 && (error_str[error_str_len - 1] == '\n' || error_str[error_str_len - 1] == '\r'))
      error_str_len--;
  #else
    char error_buf[256];
    const char *error_str = strerror_r(error_code, error_buf, sizeof(error_buf));
    const size_t error_str_len = strlen(error_str);
  #endif

  size_t final_len = length > error_str_len ? error_str_len : length - 1;
  if (final_len == 0) return 0;

  strncpy(buf, (char *)error_str, final_len);
  buf[final_len] = '\0';

  #ifdef _WIN32
    LocalFree(error_str);
  #endif

  return final_len;
}

#ifdef CTHREADS_SEMAPHORE
  int cthreads_sem_init(struct cthreads_semaphore *sem, int initial_count) {
    #ifdef CTHREADS_DEBUG
      puts("cthreads_sem_init");
    #endif
  
    #ifdef _WIN32
      sem->wSemaphore = CreateSemaphore(NULL, initial_count, LONG_MAX, NULL);

      return sem->wSemaphore == NULL;
    #else
      return sem_init(&sem->pSemaphore,0, initial_count);
    #endif
  }
  
  int cthreads_sem_wait(struct cthreads_semaphore *sem) {
    #ifdef CTHREADS_DEBUG
      puts("cthreads_sem_wait");
    #endif

    #ifdef _WIN32
      return (WaitForSingleObject(sem->wSemaphore, INFINITE) != WAIT_OBJECT_0);
    #else
      return sem_wait(&sem->pSemaphore);
    #endif
  }
  
  int cthreads_sem_trywait(struct cthreads_semaphore *sem) {
    #ifdef CTHREADS_DEBUG
      puts("cthreads_sem_trywait");
    #endif

    #ifdef _WIN32
      DWORD ret = WaitForSingleObject(sem->wSemaphore, INFINITE);
      if (ret == WAIT_OBJECT_0) return 0;
      if (ret == WAIT_TIMEOUT) SetLastError(ERROR_TIMEOUT);

      return -1;
    #else
      return sem_trywait(&sem->pSemaphore);
    #endif
  }
  
  int cthreads_sem_timedwait(struct cthreads_semaphore *sem, unsigned int ms) {
    #ifdef CTHREADS_DEBUG
      puts("cthreads_sem_timedwait");
    #endif

    #ifdef _WIN32
      DWORD ret = WaitForSingleObject(sem->wSemaphore, (DWORD)ms);
      if (ret == WAIT_OBJECT_0) return 0;
      if (ret == WAIT_TIMEOUT) SetLastError(ERROR_TIMEOUT);

      return -1;
    #else
      struct timespec ts;
      if (clock_gettime(CLOCK_REALTIME, &ts)) return 1;
 
      ts.tv_sec += ms / 1000;
      ts.tv_nsec += (ms % 1000) * 1000000;
      ts.tv_sec += ts.tv_nsec / 1000000000;
      ts.tv_nsec %= 1000000000;
 
      return sem_timedwait(&sem->pSemaphore, &ts);
    #endif
  }
  int cthreads_sem_post(struct cthreads_semaphore *sem) {
    #ifdef CTHREADS_DEBUG
      puts("cthreads_sem_post");
    #endif

    #ifdef _WIN32
      return ReleaseSemaphore(sem->wSemaphore, 1, NULL) == 0;
    #else
      return sem_post(&sem->pSemaphore);
    #endif
  }
  
  int cthreads_sem_destroy(struct cthreads_semaphore *sem) {
    #ifdef CTHREADS_DEBUG
      puts("cthreads_sem_destroy");
    #endif

    #ifdef _WIN32
      return CloseHandle(sem->wSemaphore) == 0;
    #else
      return sem_destroy(&sem->pSemaphore);
    #endif
  }
#endif

