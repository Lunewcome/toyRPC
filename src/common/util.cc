/* Util functions.
 *
 * Copyright (c) 2013, Codefor <hk dot yuhe at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "util.h"

#include <assert.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/stat.h>

int silence()
{
	int fd = open("/dev/null", O_RDWR, 0);
	if(fd == -1) return -1;

	if(dup2(fd, STDIN_FILENO) < 0) {
		perror("dup2 stdin");
		return -1;
	}

	if(dup2(fd, STDOUT_FILENO) < 0) {
		perror("dup2 stdout");
		return -1;
	}

	if(dup2(fd, STDERR_FILENO) < 0) {
		perror("dup2 stderr");
		return -1;
	}

	if (fd > STDERR_FILENO) {
		if(close(fd) < 0) {
			perror("close");
			return -1;
		}
	}
	return 0;
}

int daemonize(int chdir2root)
{
	switch (fork()) {
		case -1: /* error */
			return -1;
		case 0:  /* child */
			break;
		default: /* parent */
			_exit(EXIT_SUCCESS);
	}

	if (setsid() == -1)
		return -1;

	if (chdir2root) {
		if(chdir("/") != 0) {
			perror("chdir");
			return -1;
		}
	}

	return 0;
}

void setupSignalHandlers(void sigtermHandler(int)) {
    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

	struct sigaction act;

    /* When the SA_SIGINFO flag is set in sa_flags then sa_sigaction is used.
     * Otherwise, sa_handler is used. */
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = sigtermHandler;
    sigaction(SIGTERM, &act, NULL);

#ifdef HAVE_BACKTRACE
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
    act.sa_sigaction = sigsegvHandler;
    sigaction(SIGSEGV, &act, NULL);
    sigaction(SIGBUS, &act, NULL);
    sigaction(SIGFPE, &act, NULL);
    sigaction(SIGILL, &act, NULL);
#endif
}

void createPidFile(const char *pidfile) {
    /* Try to write the pid file in a best-effort way. */
	if(pidfile == NULL) return;
    struct stat buf;
    if (!stat(pidfile, &buf)) {
        printf("%s exits, maybe another process is running.\n", pidfile);
        return;
    }

    FILE *fp = fopen(pidfile,"w");
    if (fp) {
        fprintf(fp,"%d\n",(int)getpid());
        fclose(fp);
    }
}

long long ustime(void) {
    struct timeval tv; 
    long long ust;

    gettimeofday(&tv, NULL);
    ust = ((long)tv.tv_sec)*1000000;
    ust += tv.tv_usec;
    return ust;
}

long long mstime(void) {
    struct timeval tv; 
    long long mst;

    gettimeofday(&tv, NULL);
    mst = ((long long)tv.tv_sec)*1000;
    mst += tv.tv_usec/1000;
    return mst;
}

/* Convert an amount of bytes into a human readable string in the form
 * of 100B, 2G, 100M, 4K, and so forth. */
void bytesToHuman(char *s, unsigned long long n) {
    double d;

    if (n < 1024) {
        /* Bytes */
        sprintf(s,"%lluB",n);
        return;
    } else if (n < (1024*1024)) {
        d = (double)n/(1024);
        sprintf(s,"%.2fK",d);
    } else if (n < (1024LL*1024*1024)) {
        d = (double)n/(1024*1024);
        sprintf(s,"%.2fM",d);
    } else if (n < (1024LL*1024*1024*1024)) {
        d = (double)n/(1024LL*1024*1024);
        sprintf(s,"%.2fG",d);
    }
}

