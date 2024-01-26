# CThreads

Cross(-platform) threading library, using `pthread` and `Threads threads`.

## Purpose

The purpose of this project is to allow projects to have cross-compatibility, not being limited by either `pthread` or `Windows Threads`.
It has an easy syntax so that it can be used easily and with no problems.

## Usage

CThreads is simple and easy, with functions of the same name as `pthread`, but with different arguments.

> [!WARNING]
> To ensure you don't use a field or function that are not available on your platform, you can use the following:

```c
#ifdef CTHREADS_RWLOCK
 // Code using CThreads rwlock
#endif
```

Those macros are:
- `CTHREADS_THREAD_DWCREATIONFLAGS`
- `CTHREADS_MUTEX_BINITIALOWNER`
- `CTHREADS_MUTEX_LPNAME`
- `CTHREADS_COND_BMANUALRESET`
- `CTHREADS_COND_BINITIALSTATE`
- `CTHREADS_COND_LPNAME`
- `CTHREADS_RWLOCK`
- `CTHREADS_THREAD_STACKADDR`
- `CTHREADS_THREAD_DETACHSTATE`
- `CTHREADS_THREAD_GUARDSIZE`
- `CTHREADS_THREAD_INHERITSCHED`
- `CTHREADS_THREAD_SCHEDPOLICY`
- `CTHREADS_THREAD_SCOPE`
- `CTHREADS_THREAD_STACK`
- `CTHREADS_MUTEX_PSHARED`
- `CTHREADS_MUTEX_TYPE`
- `CTHREADS_MUTEX_ROBUST`
- `CTHREADS_MUTEX_PROTOCOL`
- `CTHREADS_MUTEX_PRIOCEILING`
- `CTHREADS_COND_PSHARED`
- `CTHREADS_COND_CLOCK`

> [!NOTE]
> Any function/field that is not listed there is available on all platforms.

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
