/*
 * procfork.c
 *
 *  Created on: 2015?1?18?
 *      Author: rongxinhua
 */

#include "proc_fork_smp.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

static int data = 111;

//fork不共享数据（子进程复制父进程的内存页）
void test_fork()
{
	static int stackdata = 222;
	printf("start ...pid = %d\n", getpid());
	pid_t pid = fork();
	switch(pid)
	{
	case -1:
		perror("fork fail");
		break;
	case 0:
		data *= 3;
		stackdata *= 3;
		printf("This is child process, pid = %d, data = %d, stackdata = %d\n", getpid(), data, stackdata);
		break;
	default:
		printf("This is parent process, pid = %d, childpid = %d\n", getpid(), pid);
		sleep(2);
		break;
	}
	printf("end ...pid = %d, data = %d, stackdata = %d\n", getpid(), data, stackdata);
}

//vfork子进程共享父进程的内存，直至成功执行了exec()或_exit(); 
//子进程调用exec()或_exit()之前，将暂停父进程的执行
void test_vfork()
{
	static int stackdata = 222;
	printf("start ...pid = %d\n", getpid());
	pid_t pid = vfork();
	switch(pid)
	{
	case -1:
		perror("fork fail");
		break;
	case 0:
		data *= 3;
		stackdata *= 3;
		printf("This is child process, pid = %d, data = %d, stackdata = %d\n", getpid(), data, stackdata);
		_exit(0);
		break;
	default:
		printf("This is parent process, pid = %d, childpid = %d\n", getpid(), pid);
		sleep(2);
		break;
	}
	printf("end ...pid = %d, data = %d, stackdata = %d\n", getpid(), data, stackdata);
}

void test_wait()
{
	printf("[%d] start...\n", getpid());
	pid_t pid = fork();
	switch(pid)
	{
	case -1:
		perror("fork fail");
		break;
	case 0:
		printf("[%d] This is child process\n", getpid());
		sleep(2);
		_exit(1);
		//break;
	default:
		printf("[%d] This is parent process, childpid = %d\n", getpid(), pid);
		break;
	}
	printf("[%d] switch end ...\n", getpid());
	for(;;)
	{
		//pid_t childPid = wait(NULL);
		int exitStatus;
		pid_t childPid = wait(&exitStatus);
		if(childPid == -1)
		{
			if(errno == ECHILD)
			{
				printf("[%d] No more children - bye!\n", getpid());
				exit(EXIT_SUCCESS);
				//exit(EXIT_FAILURE);
			} else {
				printf("[%d] wait..", getpid());
			}
		} else {
			printf("[%d] wait child %d end. exitStatus = %d\n", getpid(), childPid, exitStatus);
			if (WIFEXITED(exitStatus))	printf("WIFEXITED\n");	//子进程正常结束
			if (WIFSIGNALED(exitStatus))	printf("WIFSIGNALED\n");	//通过信号杀掉子进程
			if (WIFSTOPPED(exitStatus))		printf("WIFSTOPPED\n");	//子进程因信号而停止
			if (WIFCONTINUED(exitStatus))	printf("WIFCONTINUE\n");	//子进程收到SIGCONT而恢复执行
		}
	}
}


void test_execl(char **argv)
{
	const char *exepath = *argv;
	printf("[%d] execl start...\n", getpid());
	if(execl(exepath, *argv, "fork", (char *)NULL) == -1) {
		printf("execl fail, errno = %d, %s\n", errno, strerror(errno));
	}
	printf("[%d] execl end...\n", getpid());
}

static void onSigHandler(int sig)
{
	printf("onSigHandler, sig = %d\n", sig);
	exit(EXIT_SUCCESS);
}

void test_signal()
{
	if(signal(SIGINT, onSigHandler) == SIG_ERR)
	{
		perror("signal fail.\n");
		return;
	}
	if(signal(SIGQUIT, onSigHandler) == SIG_ERR)
	{
		perror("signal fail.\n");
		return;
	}
	int j;
	for(j = 0; ; j++) {
		printf("j = %d\n", j);
		if(j == 5)
		{
			//raise(SIGQUIT);
			kill(getpid(), SIGQUIT);
		}
		sleep(3);
	}
}

void test_sigset()
{

}


int main(int argc, char **argv)
{
	char *func;
	if(argc >= 2)
	{
		func = *(argv+1);
		printf("%s %s", *argv, func);
		if(argc >= 3) {
			printf("%s", *(argv+2));
		}
		printf("\n");
	}
	if(!strcmp(func, "fork")) {
		test_fork();
	} else if(!strcmp(func, "vfork")) {
		test_vfork();
	} else if(!strcmp(func, "wait")) {
		test_wait();
	} else if(!strcmp(func, "execl")) {
		test_execl(argv);
	} else if(!strcmp(func, "signal")) {
		test_signal();
	}
	return 0;
}


