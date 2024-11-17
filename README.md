# CThreads

Cross(-platform) threading library, using `pthread` and `Windows Threads`.

## Purpose

CThreads is an extremely portable threading library, allowing you to use the same code on Windows and Unix-like systems. It is based on `pthread` and `Windows Threads`, and provides a simple interface to use them.

## API

- `cthreads_thread_create`: Creates a new thread.
- `cthreads_thread_detach`: Detaches a thread.
- `cthreads_thread_join`: Joins a thread.
- `cthreads_thread_equal`: Compares two thread structures for equality.
- `cthreads_thread_self`: Retrieves the thread identifier of the current thread.
- `cthreads_thread_id`: Retrieves the thread identifier of the specified thread.
- `cthreads_thread_exit`: Exits a thread.
- `cthreads_mutex_init`: Initializes a mutex.
- `cthreads_mutex_lock`: Locks a mutex.
- `cthreads_mutex_trylock`: Tries to lock a mutex without blocking.
- `cthreads_mutex_unlock`: Unlocks a mutex.
- `cthreads_mutex_destroy`: Destroys a mutex.
- `cthreads_cond_init`: Initializes a condition variable.
- `cthreads_cond_signal`: Signals a condition variable.
- `cthreads_cond_broadcast`: Broadcasts a condition variable.
- `cthreads_cond_destroy`: Destroys a condition variable.
- `cthreads_cond_wait`: Waits on a condition variable.
- `cthreads_cond_timedwait`: Waits on a condition variable till ms.
- `cthreads_rwlock_init`: Initializes a read-write lock. Locked by `CTHREADS_RWLOCK`.
- `cthreads_rwlock_rdlock`: Acquires a read lock on a read-write lock. Locked by `CTHREADS_RWLOCK`.
- `cthreads_rwlock_unlock`: Unlocks a read-write lock. Locked by `CTHREADS_RWLOCK`.
- `cthreads_rwlock_wrlock`: Acquires a write lock on a read-write lock. Locked by `CTHREADS_RWLOCK`.
- `cthreads_rwlock_destroy`: Destroys a read-write lock. Locked by `CTHREADS_RWLOCK`.
- `cthreads_error_code`: Gets the platform-specific error code after an operation.
- `cthreads_error_string`: Writes the platform-specific error message into a user-provided buffer.

> [!NOTE]
> For internal information of what functions are used on certain platform, see `cthreads.h` file.

## Usage

CThreads is simple and easy, with (most) functions of the same name as `pthread`, but with different arguments.

> [!WARNING]
> To ensure you don't use a field or function that are not available on your platform, you can use the following:

```c
#ifdef CTHREADS_RWLOCK
 // Code using CThreads rwlock
#endif
```

Those macros are:
- `CTHREADS_THREAD_DWCREATIONFLAGS`
- `CTHREADS_THREAD_STACKADDR`
- `CTHREADS_THREAD_DETACHSTATE`
- `CTHREADS_THREAD_GUARDSIZE`
- `CTHREADS_THREAD_INHERITSCHED`
- `CTHREADS_THREAD_SCHEDPOLICY`
- `CTHREADS_THREAD_SCOPE`
- `CTHREADS_THREAD_STACK`
- `CTHREADS_MUTEX_ATTR`
- `CTHREADS_MUTEX_PSHARED`
- `CTHREADS_MUTEX_TYPE`
- `CTHREADS_MUTEX_ROBUST`
- `CTHREADS_MUTEX_PROTOCOL`
- `CTHREADS_MUTEX_PRIOCEILING`
- `CTHREADS_COND_ATTR`
- `CTHREADS_COND_PSHARED`
- `CTHREADS_COND_CLOCK`
- `CTHREADS_RWLOCK`

> [!NOTE]
> Any function/field that is not listed there is available on all platforms.

For debugging, you can use the `CTHREADS_DEBUG` macro to enable debug messages, which will show which functions are being used.

## Tested compilers and platforms

CThreads has been tested on the following compilers and platforms:

- MSVC 2022, 2013 (Windows 10, 8.1)
- Cygwin GCC 11.3.0 (Windows 10)
- MinGW GCC 11.3.0 (Windows 10)
- OpenWatcom from June 2022 (Windows NT 4)
- GCC 11.3.0 (FreeBSD 586)
- GCC 8.3.0 (Linux 4.19.0 System/390 custom image)
- Clang 10.0 (FreeBSD 586)
- Clang & GCC 15.0.7 (Ubuntu 23.04, Linux 6.2.0-23-generic)
- Clang & GCC ?.?.? (Arch Linux)
- GCC 10.2.1-6 & Clang 11.0.1-2 (Raspbian GNU/Linux 11, Linux 6.1.21-v8+)
- GCC 16.0.4 & Clang 16.0.4 (Termux 0.118.0, Android 13 - Galaxy A53 5G)
- acomp SVR5 (UnixWare 7.1.1)
