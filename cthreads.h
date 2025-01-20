#ifndef CTHREADS_H
#define CTHREADS_H

struct cthreads_args {
  void *(*func)(void *data);
  void *data;
};

#ifdef _WIN32
  #include <windows.h>
  #define CTHREADS_SEMAPHORE 1
#else
  #include <pthread.h>
#endif

#ifdef _WIN32
  #define CTHREADS_THREAD_DWCREATIONFLAGS 1

  #define CTHREADS_RWLOCK 1
#else
  #define CTHREADS_THREAD_STACKADDR 1
  #define CTHREADS_THREAD_DETACHSTATE 1
  #define CTHREADS_THREAD_GUARDSIZE 1
  #ifndef __ANDROID__
    #define CTHREADS_THREAD_INHERITSCHED 1
  #endif
  #define CTHREADS_THREAD_SCHEDPOLICY 1
  #define CTHREADS_THREAD_SCOPE 1
  #if _POSIX_C_SOURCE >= 200112L
    #include <semaphore.h>
    #define CTHREADS_SEMAPHORE 1
    #define CTHREADS_THREAD_STACK 1
  #endif

  #define CTHREADS_MUTEX_ATTR 1

  #define CTHREADS_MUTEX_PSHARED 1
  #if _POSIX_C_SOURCE >= 200809L
    #define CTHREADS_MUTEX_TYPE 1
  #endif
  #if _POSIX_C_SOURCE >= 200112L
    #if (defined __linux__ || defined __FreeBSD__) && !defined __ANDROID__
      #define CTHREADS_MUTEX_ROBUST 1
    #endif
  #endif
  #ifndef __ANDROID__
    #define CTHREADS_MUTEX_PROTOCOL 1
    #define CTHREADS_MUTEX_PRIOCEILING 1
  #endif

  #define CTHREADS_COND_ATTR 1

  #define CTHREADS_COND_PSHARED 1
  #if _POSIX_C_SOURCE >= 200112L
    #define CTHREADS_COND_CLOCK 1
  #endif

  #if _POSIX_C_SOURCE >= 200112L
    #define CTHREADS_RWLOCK 1
  #endif
#endif

struct cthreads_thread {
  #ifdef _WIN32
    HANDLE wThread;
  #else
    pthread_t pThread;
  #endif
};

struct cthreads_thread_attr {
  size_t stacksize;
  #ifdef _WIN32
    int dwCreationFlags;
  #else
    void *stackaddr;
    int detachstate;
    size_t guardsize;
    int inheritsched;
    int schedpolicy;
    int scope;
    #ifdef CTHREADS_THREAD_STACK
      size_t stack;
    #endif
  #endif
};

struct cthreads_mutex {
  #ifdef _WIN32
    CRITICAL_SECTION wMutex;
  #else
    pthread_mutex_t pMutex;
  #endif
};

#ifdef CTHREADS_MUTEX_ATTR
  struct cthreads_mutex_attr {
    #ifndef _WIN32
      int pshared;
      #ifdef CTHREADS_MUTEX_TYPE
        int type;
      #endif
      #ifdef CTHREADS_MUTEX_ROBUST
        int robust;
      #endif
      #ifdef CTHREADS_MUTEX_PROTOCOL
        int protocol;
      #endif
      #ifdef CTHREADS_MUTEX_PRIOCEILING
        int prioceiling;
      #endif
    #endif
  };
#endif

struct cthreads_cond {
  #ifdef _WIN32
    CONDITION_VARIABLE wCond;
  #else
    pthread_cond_t pCond;
  #endif
};

#ifdef CTHREADS_COND_ATTR
  struct cthreads_cond_attr {
    #ifndef _WIN32
      int pshared;
      #ifdef CTHREADS_COND_CLOCK
        int clock;
      #endif
    #endif
  };
#endif

#ifdef CTHREADS_RWLOCK
struct cthreads_rwlock {
  #ifdef _WIN32
    int type;
    PSRWLOCK wRWLock;
  #else
    pthread_rwlock_t pRWLock;
  #endif
};
#endif

#ifdef CTHREADS_SEMAPHORE
  struct cthreads_semaphore {
    #ifdef _WIN32
      HANDLE wSemaphore;
    #else
      sem_t pSemaphore;
    #endif
  };
#endif

/**
 * Creates a new thread.
 *
 * - pthread: pthread_create
 * - windows threads: CreateThread
 *
 * @param thread Pointer to the thread structure to be filled with the new thread information.
 * @param attr Pointer to the thread attributes. Set it to NULL for default attributes.
 * @param func Pointer to the function that will be executed in the new thread.
 * @param data Pointer to the data that will be passed to the thread function.
 * @param args Pointer to the thread arguments.
 * @return 0 on success, non-zero error code on failure.
 */
int cthreads_thread_create(struct cthreads_thread *thread, struct cthreads_thread_attr *attr, void *(*func)(void *data), void *data, struct cthreads_args *args);

/**
 * Detaches a thread.
 *
 * - pthread: pthread_detach
 * - windows threads: CloseHandle
 *
 * @param thread Thread structure to be detached.
 * @return 0 on success, non-zero error code on failure.
 */
int cthreads_thread_detach(struct cthreads_thread thread);

/**
 * Joins a thread.
 *
 * - pthread: pthread_join
 * - windows threads: WaitForSingleObject & GetExitCodeThread
 *
 * @param thread Pointer to the thread structure to be joined.
 * @param code Pointer to store the exit code of the joined thread.
 * @return 0 on success, non-zero error code on failure.
 */
int cthreads_thread_join(struct cthreads_thread thread, void *code);

/**
 * Compares two thread structures for equality.
 *
 * - pthread: pthread_equal
 * - windows threads: GetCurrentThreadId
 *
 * @param thread1 First thread structure to compare.
 * @param thread2 Second thread structure to compare.
 * @return 1 if the threads are equal, zero otherwise.
 */
int cthreads_thread_equal(struct cthreads_thread thread1, struct cthreads_thread thread2);

/**
 * Retrieves the thread struct of the current thread.
 *
 * - pthread: pthread_self
 * - windows threads: GetCurrentThreadId
 *
 * @return Thread struct of the current thread.
 */
struct cthreads_thread cthreads_thread_self(void);

/**
 * Retrieves the thread identifier of the specified thread.
 * 
 * - pthread: N/A
 * - windows threads: GetThreadId
 * 
 * @param thread Thread structure to retrieve the identifier from.
 * @return Thread identifier of the specified thread.
*/
unsigned long cthreads_thread_id(struct cthreads_thread thread);

/**
 * Exits a thread.
 *
 * - pthread: pthread_exit
 * - windows threads: ExitThread
 *
 * @param code Pointer to the thread exit code.
 */
void cthreads_thread_exit(void *code);

/**
 * Cancels a thread.
 *
 * - pthread: pthread_cancel
 * - windows threads: TerminateThread
 *
 * @param thread Thread structure to be canceled.
 * @return 0 on success, non-zero error code on failure.
 */
int cthreads_thread_cancel(struct cthreads_thread thread);

/**
 * Initializes a mutex.
 *
 * - pthread: pthread_mutex_init
 * - windows threads: InitializeCriticalSection
 *
 * @param mutex Pointer to the mutex structure to be initialized.
 * @param attr Pointer to the mutex attributes. Set it to NULL for default attributes. Only available if CTHREADS_MUTEX_ATTR is defined.
 * @return 0 on success, non-zero error code on failure.
 */
#ifdef CTHREADS_MUTEX_ATTR
  int cthreads_mutex_init(struct cthreads_mutex *mutex, struct cthreads_mutex_attr *attr);
#else
  int cthreads_mutex_init(struct cthreads_mutex *mutex, void *attr);
#endif

/**
 * Locks a mutex.
 *
 * - pthread: pthread_mutex_lock
 * - windows threads: EnterCriticalSection
 *
 * @param mutex Pointer to the mutex structure to be locked.
 * @return 0 on success, non-zero error code on failure.
 */
int cthreads_mutex_lock(struct cthreads_mutex *mutex);

/**
 * Tries to lock a mutex without blocking.
 *
 * - pthread: pthread_mutex_trylock
 * - windows threads: TryEnterCriticalSection
 *
 * @param mutex Pointer to the mutex structure to be locked.
 * @return 0 on success, non-zero error code on failure.
 */
int cthreads_mutex_trylock(struct cthreads_mutex *mutex);

/**
 * Unlocks a mutex.
 *
 * - pthread: pthread_mutex_unlock
 * - windows threads: LeaveCriticalSection
 *
 * @param mutex Pointer to the mutex structure to be unlocked.
 * @return 0 on success, non-zero error code on failure.
 */
int cthreads_mutex_unlock(struct cthreads_mutex *mutex);

/**
 * Destroys a mutex.
 *
 * - pthread: pthread_mutex_destroy
 * - windows threads: DeleteCriticalSection
 *
 * @param mutex Pointer to the mutex structure to be destroyed.
 * @return 0 on success, non-zero error code on failure.
 */
int cthreads_mutex_destroy(struct cthreads_mutex *mutex);

/**
 * Initializes a condition variable.
 *
 * - pthread: pthread_cond_init
 * - windows threads: InitializeConditionVariable
 *
 * @param cond Pointer to the condition variable structure to be initialized.
 * @param attr Pointer to the condition variable attributes. Set it to NULL for default attributes. Only available if CTHREADS_COND_ATTR is defined.
 * @return 0 on success, non-zero error code on failure.
 */
#ifdef CTHREADS_COND_ATTR
  int cthreads_cond_init(struct cthreads_cond *cond, struct cthreads_cond_attr *attr);
#else
  int cthreads_cond_init(struct cthreads_cond *cond, void *attr);
#endif

/**
 * Signals a condition variable.
 *
 * - pthread: pthread_cond_signal
 * - windows threads: WakeConditionVariable
 *
 * @param cond Pointer to the condition variable structure.
 * @return 0 on success, non-zero error code on failure.
 */
int cthreads_cond_signal(struct cthreads_cond *cond);

/**
 * Broadcasts a condition variable.
 *
 * - pthread: pthread_cond_broadcast
 * - windows threads: WakeAllConditionVariable
 *
 * @param cond Pointer to the condition variable structure.
 * @return 0 on success, non-zero error code on failure.
 */
int cthreads_cond_broadcast(struct cthreads_cond *cond);

/**
 * Destroys a condition variable.
 *
 * - pthread: pthread_cond_destroy
 * - windows threads: DeleteConditionVariable
 *
 * @param cond Pointer to the condition variable structure to be destroyed.
 * @return 0 on success, non-zero error code on failure.
 */
int cthreads_cond_destroy(struct cthreads_cond *cond);

/**
 * Waits on a condition variable.
 *
 * - pthread: pthread_cond_wait
 * - windows threads: SleepConditionVariableCS
 *
 * @param cond Pointer to the condition variable structure.
 * @param mutex Pointer to the associated mutex structure.
 * @return 0 on success, non-zero error code on failure.
 */
int cthreads_cond_wait(struct cthreads_cond *cond, struct cthreads_mutex *mutex);

/**
 * Waits on a condition variable till set ms.
 *
 * - pthread: pthread_cond_timedwait
 * - windows threads: SleepConditionVariableCS
 *
 * @param cond Pointer to the condition variable structure.
 * @param mutex Pointer to the associated mutex structure.
 * @param ms Time in milliseconds to unlock if not unlocked in time.
 * @return 0 on success, non-zero error code on failure.
 */
int cthreads_cond_timedwait(struct cthreads_cond *cond, struct cthreads_mutex *mutex, unsigned int ms);

#ifdef CTHREADS_RWLOCK
  /**
   * Initializes a read-write lock.
   *
   * - pthread: pthread_rwlock_init
   * - windows threads: InitializeSRWLock
   *
   * @param rwlock Pointer to the read-write lock structure to be initialized.
   * @return 0 on success, non-zero error code on failure.
   */
  int cthreads_rwlock_init(struct cthreads_rwlock *rwlock);

  /**
   * Acquires a read lock on a read-write lock.
   *
   * - pthread: pthread_rwlock_rdlock
   * - windows threads: AcquireSRWLockShared
   *
   * @param rwlock Pointer to the read-write lock structure to be locked.
   * @return 0 on success, non-zero error code on failure.
   */
  int cthreads_rwlock_rdlock(struct cthreads_rwlock *rwlock);

  /**
   * Unlocks a read-write lock.
   *
   * - pthread: pthread_rwlock_unlock
   * - windows threads: ReleaseSRWLockShared or ReleaseSRWLockExclusive
   *
   * @param rwlock Pointer to the read-write lock structure to be unlocked.
   * @return 0 on success, non-zero error code on failure.
   */
  int cthreads_rwlock_unlock(struct cthreads_rwlock *rwlock);

  /**
   * Acquires a write lock on a read-write lock.
   *
   * - pthread: pthread_rwlock_wrlock
   * - windows threads: AcquireSRWLockExclusive
   *
   * @param rwlock Pointer to the read-write lock structure to be locked.
   * @return 0 on success, non-zero error code on failure.
   */
  int cthreads_rwlock_wrlock(struct cthreads_rwlock *rwlock);

  /**
   * Destroys a read-write lock.
   *
   * - pthread: pthread_rwlock_destroy
   * - windows threads: N/A
   *
   * @param rwlock Pointer to the read-write lock structure to be destroyed.
   * @return 0 on success, non-zero error code on failure.
   */
  int cthreads_rwlock_destroy(struct cthreads_rwlock *rwlock);

  /**
   * Returns the platform-specific error code.
   *
   * - pthread: errno
   * - windows: GetLastError()
   *
   * @return Platform-specific error code
   */
  int cthreads_error_code(void);
#endif

/**
 * Obtains the error code and writes at most `length` 
 * bytes of the associated message to `buf`.
 *
 * @param error_code Platform-specific error code. (See: `cthreads_error_code()`)
 * @param buf Buffer of `length` bytes and target of the error message
 * @param length Length of the provided buffer
 *
 * @return Number of bytes required to print the message + NULL-terminator
 */
size_t cthreads_error_string(int error_code, char *buf, size_t length);

#ifdef CTHREADS_SEMAPHORE
  /**
  * Initializes a semaphore.
  *
  * - pthread: sem_init()
  * - windows threads: CreateSemaphore()
  *
  * @param semaphore Pointer to the semaphore structure to be initialized.
  * @param initial count of the semaphore
  * @return 0 on success, non-zero error code on failure.
  */
  int cthreads_sem_init(struct cthreads_semaphore *sem, int initial_count);

  /**
  * Decrease a semaphore.
  *
  * - pthread: sem_wait()
  * - windows threads: WaitForSingleObject()
  *
  * @param semaphore Pointer to the semaphore structure to be decreased.
  * @return 0 on success, non-zero error code on failure.
  */
  int cthreads_sem_wait(struct cthreads_semaphore *sem);

  /**
  * Tries to decrease a semaphore without blocking.
  *
  * - pthread: sem_trywait()
  * - windows threads: WaitForSingleObject()
  *
  * @param semaphore Pointer to the semaphore structure to be decreased.
  * @return 0 on success, non-zero error code on failure.
  */
  int cthreads_sem_trywait(struct cthreads_semaphore *sem);


  /**
  * Waits on a condition variable till set ms.
  *
  * - pthread: sem_timedwait
  * - windows threads: WaitForSingleObject()
  *
  * @param semaphore Pointer to the semaphore structure to be decreased.
  * @param ms Time in milliseconds to unlock if not unlocked in time.
  * @return 0 on success, non-zero error code on failure.
  */
  int cthreads_sem_timedwait(struct cthreads_semaphore *sem, unsigned int ms);

  /**
  * Increase a semaphore.
  *
  * - pthread: sem_post()
  * - windows threads: ReleaseSemaphore()
  *
  * @param semaphore Pointer to the semaphore structure to be increased.
  * @return 0 on success, non-zero error code on failure.
  */
  int cthreads_sem_post(struct cthreads_semaphore *sem);

  /**
  * Destroy a semaphore.
  *
  * - pthread: sem_destroy()
  * - windows threads: CloseHandle()
  *
  * @param semaphore Pointer to the semaphore structure to be destroyed.
  * @return 0 on success, non-zero error code on failure.
  */
  int cthreads_sem_destroy(struct cthreads_semaphore *sem);
#endif

#endif /* CTHREADS_H */
