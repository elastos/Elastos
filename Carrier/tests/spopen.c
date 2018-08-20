/*
 * Copyright (c) 2017-2018 iwhisper.io
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#include <process.h>
#include <fcntl.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

#include "spopen.h"

struct _subprocess {
    int sig;
    FILE *in;
    FILE *out;
    FILE *err;
#if defined(_WIN32) || defined(_WIN64)
    PROCESS_INFORMATION pi;
#else
    pid_t pid;
#endif
};

#define _SUBPROCESS_SIG_INIT     0x73706D6E

subprocess_t spopen(const char *command, const char *mode)
{
    subprocess_t sp = NULL;
    int in = 0;
    int out = 0;

    if (!command || !*command || !mode || !*mode) {
        errno = EINVAL;
        return NULL;
    }

    if (*mode == 'r') {
        if (mode[1] == '+' && mode[2] == '\x0') {
            in = 1;
            out = 1;
        } else if (mode[1] == '\x0') {
            out = 1;
        } else {
            errno = EINVAL;
            return NULL;
        }
    } else if (*mode == 'w' && mode[1] == '\x0') {
        in = 1;
    } else {
        errno = EINVAL;
        return NULL;
    }

    sp = (subprocess_t)calloc(1, sizeof(struct _subprocess));
    if (!sp) {
        errno = ENOMEM;
        return NULL;
    }

#if defined(_WIN32) || defined(_WIN64)
    SECURITY_ATTRIBUTES sa;
    STARTUPINFO si;
    HANDLE h_stdin[2] = { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };
    HANDLE h_stdout[2] = { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };
    HANDLE h_stderr[2] = { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (in) {
        if (!CreatePipe(&h_stdin[0], &h_stdin[1], &sa, 0))
            goto error_exit;

        // Ensure the write handle to the pipe for STDIN is not inherited.
        if (!SetHandleInformation(h_stdin[1], HANDLE_FLAG_INHERIT, 0))
            goto error_exit;
    }

    if (out) {
        if (!CreatePipe(&h_stdout[0], &h_stdout[1], &sa, 0))
            goto error_exit;

        // Ensure the read handle to the pipe for STDOUT is not inherited.
        if (!SetHandleInformation(h_stdout[0], HANDLE_FLAG_INHERIT, 0))
            goto error_exit;

        if (!CreatePipe(&h_stderr[0], &h_stderr[1], &sa, 0))
            goto error_exit;

        // Ensure the read handle to the pipe for STDERR is not inherited.
        if (!SetHandleInformation(h_stderr[0], HANDLE_FLAG_INHERIT, 0))
            goto error_exit;
    }

    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = h_stdin[0];
    si.hStdOutput = h_stdout[1];
    si.hStdError = h_stderr[1];

    if (!CreateProcess(NULL, (LPTSTR)command, NULL, NULL, TRUE, 0, NULL,
                       NULL, &si, &sp->pi))
        goto error_exit;

    sp->sig = _SUBPROCESS_SIG_INIT;

    if (in) {
        sp->in = fdopen(_open_osfhandle((intptr_t)h_stdin[1], 0), "w");
        setvbuf(sp->in, NULL, _IONBF, 0);
    }

    if (out) {
        sp->out = fdopen(_open_osfhandle((intptr_t)h_stdout[0], _O_RDONLY), "r");
        sp->err = fdopen(_open_osfhandle((intptr_t)h_stderr[0], _O_RDONLY), "r");

        setvbuf(sp->out, NULL, _IONBF, 0);
        setvbuf(sp->err, NULL, _IONBF, 0);
    }

    return sp;

error_exit:
    if (h_stdin[0] != INVALID_HANDLE_VALUE)
        CloseHandle(h_stdin[0]);
    if (h_stdin[1] != INVALID_HANDLE_VALUE)
        CloseHandle(h_stdin[1]);

    if (h_stdout[0] != INVALID_HANDLE_VALUE)
        CloseHandle(h_stdout[0]);
    if (h_stdout[1] != INVALID_HANDLE_VALUE)
        CloseHandle(h_stdout[1]);

    if (h_stderr[0] != INVALID_HANDLE_VALUE)
        CloseHandle(h_stderr[0]);
    if (h_stderr[1] != INVALID_HANDLE_VALUE)
        CloseHandle(h_stderr[1]);

    if (sp)
        free(sp);

    return NULL;
#else
    int p_stdin[2] = {-1, -1};
    int p_stdout[2] = {-1, -1};
    int p_stderr[2] = {-1, -1};
    pid_t pid;

    if (in) {
        if (pipe(p_stdin) < 0)
            goto error_exit;
    }

    if (out) {
        if (pipe(p_stdout) < 0)
            goto error_exit;

        if (pipe(p_stderr) < 0)
            goto error_exit;
    }

    pid = fork();
    if (pid < 0) {
        // Error
        goto error_exit;
    } else if (pid == 0) {
        // Child
        if (in) {
            if (dup2(p_stdin[0], STDIN_FILENO) < 0)
                goto error_exit;

            close(p_stdin[0]);
            close(p_stdin[1]);
            p_stdin[0] = -1;
            p_stdin[1] = -1;
        }

        if (out) {
            if (dup2(p_stdout[1], STDOUT_FILENO) < 0)
                goto error_exit;

            if (dup2(p_stderr[1], STDERR_FILENO) < 0)
                goto error_exit;

            close(p_stdout[0]);
            close(p_stdout[1]);
            p_stdout[0] = -1;
            p_stdout[1] = -1;

            close(p_stderr[0]);
            close(p_stderr[1]);
            p_stderr[0] = -1;
            p_stderr[1] = -1;
        }

        execl("/bin/sh", "sh", "-c", command, NULL);
        //execl("/bin/sh", "sh", "-c", "ifconfig -v", NULL);
        exit(127);
    }

    // Parent
    sp->sig = _SUBPROCESS_SIG_INIT;
    sp->pid = pid;

    if (in) {
        close(p_stdin[0]);
        p_stdin[0] = -1;

        sp->in = fdopen(p_stdin[1], "w");
    }

    if (out) {
        close(p_stdout[1]);
        close(p_stderr[1]);

        p_stdout[1] = -1;
        p_stderr[1] = -1;

        sp->out = fdopen(p_stdout[0], "r");
        sp->err = fdopen(p_stderr[0], "r");
    }

    return sp;

error_exit:
    if (p_stdin[0] > 0)
        close(p_stdin[0]);
    if (p_stdin[1] > 0)
        close(p_stdin[1]);

    if (p_stdout[0] > 0)
        close(p_stdout[0]);
    if (p_stdout[1] > 0)
        close(p_stdout[1]);

    if (p_stderr[0] > 0)
        close(p_stderr[0]);
    if (p_stderr[1] > 0)
        close(p_stderr[1]);

    if (sp)
        free(sp);

    return NULL;
#endif
}

int spclose(subprocess_t subprocess)
{
    if (subprocess == NULL || subprocess->sig != _SUBPROCESS_SIG_INIT) {
        errno = EINVAL;
        return -1;
    }

    if (subprocess->in)
        fclose(subprocess->in);

    if (subprocess->out)
        fclose(subprocess->out);

    if (subprocess->err)
        fclose(subprocess->err);

#if defined(_WIN32) || defined(_WIN64)
    DWORD rc = (DWORD)-1;

    WaitForSingleObject(subprocess->pi.hProcess, INFINITE);
    GetExitCodeProcess(subprocess->pi.hProcess, &rc);

    CloseHandle(subprocess->pi.hThread);
    CloseHandle(subprocess->pi.hProcess);

    free(subprocess);

    return rc;
#else
    int pstat;
    pid_t pid;

    do {
        pid = waitpid(subprocess->pid, &pstat, 0);
    } while (pid == -1 && errno == EINTR);

    subprocess->sig = -1;
    free(subprocess);

    return pid == -1 ? -1 : pstat;
#endif
}

FILE *spstdin(subprocess_t subprocess)
{
    if (subprocess == NULL || subprocess->sig != _SUBPROCESS_SIG_INIT) {
        errno = EINVAL;
        return NULL;
    }

    return subprocess->in;
}

FILE *spstdout(subprocess_t subprocess)
{
    if (subprocess == NULL || subprocess->sig != _SUBPROCESS_SIG_INIT) {
        errno = EINVAL;
        return NULL;
    }

    return subprocess->out;
}

FILE *spstderr(subprocess_t subprocess)
{
    if (subprocess == NULL || subprocess->sig != _SUBPROCESS_SIG_INIT) {
        errno = EINVAL;
        return NULL;
    }

    return subprocess->err;
}

int spid(subprocess_t subprocess)
{
    if (subprocess == NULL || subprocess->sig != _SUBPROCESS_SIG_INIT) {
        errno = EINVAL;
        return -1;
    }

#if defined(_WIN32) || defined(_WIN64)
    return (int)subprocess->pi.dwProcessId;
#else
    return subprocess->pid;
#endif
}

int spkill(subprocess_t subprocess)
{
    if (subprocess == NULL || subprocess->sig != _SUBPROCESS_SIG_INIT)
        return EINVAL;

#if defined(_WIN32) || defined(_WIN64)
    return TerminateProcess(subprocess->pi.hProcess, (UINT)-1) ? 0 : ESRCH;
#else
    return kill(subprocess->pid, SIGKILL);
#endif
}
