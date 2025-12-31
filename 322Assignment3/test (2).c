// Exceptional Flow Control
#include "csapp.h"


/* 

	void systemCallErrorHandling(){
    
    on error, Linux system-level call functions typically return -1
    and set the global variable errno to a value that indicates the error

    Hard and fast rule: always check the return value of system calls, and if it indicates an error
    
    pid_t pid;
    if((pid = fork())<0){
        fprintf(stderr,"fork error");
        exit(0);
    }
}


// error reporting functions
void unix_error(char *msg){
    // can simply call this function with a string that describes the error
    fprintf(stderr,"%s: %d\n",msg, strerror(errno));
    exit(0);    
}

// now we can update the system call error handling function

void systemCallErrorHandling2(){
    pid_t pid;
    if((pid = fork())<0){
        unix_error("fork error");
    }
}

// Error handling on wrappers
// we can simply the code written by using Stevens style error handling for wrappers
pid_t Fork(void){
    pid_t pid;
    if((pid = fork())<0){
        unix_error("Fork error");
        return pid;
    }
}



*/
 

/*
To obtain process IDs we can use the following calls
pid_t getpid(void); -> returns the process ID of the calling process
pid_t  getppid(void); -> returns the parent process ID of the calling process
*/


/*
Creating and Terminating Processes

from a programmers prespective, we can consider a process to be in one of three states

1. Running: a process is either executing on the CPU or waiting its turn to chosen by the kernel to run
2. Stopped: process execution has been suspended, and will remain so until further notice
3. Terminated: process is finshed perminantly
*/

/*
Terminate Processes

We terminate processes for various reasons:
1: Reciving a signal to terminate
2: returning to main
3: caling exit

void exit(int status)
terminates with exit status of "status"
normal return is 0, non-zero indicates an error
another way to explicitly set theexit status is to return an integer from main
*/


/*
Create Processes

parent process creates a new running child process by calling fork

int fork(void)
llreturns: 0 to the child, child PID to the parent, -1 on error
Child process is almost identical to parent process
child receives a copy of the parents virtual address space
child gets its own copy of the parents file descriptors
child gets its own process ID

for is a very powerful system call, it is the basis for all process creation in Unix
it is called once, but returns twice
*/

void forkExample(){
    pid_t pid;
    int x = 1;

    pid = Fork();
    if(pid == 0){
        //Child
        printf("child: x=%d\n",++x);
        exit(0);
    }

    //Parent
    printf("parent: x=%d\n",--x);
    exit(0);
    

}

void fork2(){
	printf("L0\n");
	fork();
	printf("L1\n");
	fork();
	printf("bye\n");


}


/*
Reaping Child Processes

IDEA: after a process finshes it still consumes system recorces, think exit status
we call this a zombie, a half alive function

We must "reap" these functions
we do this using wait or waitpd
parent is given exit status information
kernel then deletes the zombie process child

In most cases any parent terminates without reaping a child then an orphaned child
will be reaped by the init process (pid == 1)
so only need explict reaping in long running processes


*/

// Zombie Child Example
void fork7(){
	printf("Zombie Child Example\n");
	if(fork() == 0){
		// Child
		printf("Term Child, PID = %d\n", getpid());
		exit(0);
	}
	else{
		printf("Runninf Parent, PID = %d\n", getpid());
		while(1);
	}


}


// Non terminating Child Example
void fork8(){
	if(fork()==0){
		// child
		printf("Running Child , PID = %d\n", getpid());
		while(1); // inf loop
	}
	else{
		printf("Terminating Parent, PID = %d \n", getpid());
		exit(0);
	}
}


/*
wait: synchronizing with Children
A parent can reap a chiold by calling the wait function

int wait(int *child_status)
	suspends a process until one of its childrenb terminates
	Return valuen is Pid of the child process that is terminated
	If child_status != NULL then int pointed to will be set ot a value that indicates reason child terminated ->
	-> and the exit status

		Check using marcos in wait.h

*/

void fork9(){
	int child_status;
	if(fork()==0){
		printf("HC: hello from child\n");
		exit(0);
	}
	else{
		printf("HP: Hello from parent \n");
		wait(&child_status);
		printf("CT: child has terminated\n");
	}
	printf("Goodbye\n");
}

/*
Second wait example
If multiple children completed, this will take an arbitrary order
Can use macros WIFEXITED and WEXITSTATUS to give us exit status information
*/

void fork10(){
	int N = 5;
	pid_t pid[N];
	int i, child_status;
	
	for(i=0;i<N;i++){
		if((pid[i]=fork())==0){
			exit(100+i); // child
		}
	}
	for(i=0;i<N;i++){
		//parent
		pid_t wpid = wait(&child_status);
		if(WIFEXITED(child_status)){
			printf("Child %d terminated with exit status %d\n",wpid,WEXITSTATUS(child_status));
		}
		else{
			printf("Child %d terminate abnormally \n",wpid);
		}
	}
}

/*
waitpid: waiting for a specific process
pid_t waitpid(pid_t pid, int &status, int options)
	suspens current process until input process terminates

*/

void fork11(){
	int N = 5;	
	pid_t pid[N];
	int i, child_status;

	for(i=0;i<N;i++){
		if((pid[i]=fork())==0){
			exit(100+i); // child
		}
	}
	for(i=N-1; i>0; i--){
		pid_t wpid = waitpid(pid[i], &child_status, 0);
		if(WIFEXITED(child_status)){
			printf("Child %d terminated with exit status %d \n", wpid, WEXITSTATUS(child_status));
		}
		else{
			printf("Child %d terminate abnormally \n");
		}
	}


}

/*
execve: Loading and Running Programsw
int execve(char *filename, char *argv[], char *envp[])
Loads and runs in the current process
	executable file name -> can be obj file or scipt with #!interpreter e.g. #1/bin/bash
	with argument list argv, argv[0] always is filename
	enviroment variable envp
Overwrites code, data, and stack while retaining PID, open files, and signals context
Called once and never returns unless error occurs


*/

// void eval(char *cmdline){
// 	int MAXARGS = 128;
// 	int MAXLINE = 8192;

// 	char *argv[MAXARGS]; // argument list
// 	char buf[MAXLINE]; // holds modified command line
// 	int bg; // background job
// 	pid_t pid; // process id

// 	strcpy(buf, cmdline);
// 	bg = parseline(buf, argv);
// 	if(argv[0] == NULL){
// 		return; // ignore empty lines
// 	}

// 	if(!builtin_command(argv)){
// 		if((pid = Fork())==0){
// 			if(execve(argv[0],argv, environ)<0){
// 				printf("%s: command not found \n", argv[0]);
// 				exit(0);
// 			}
// 		}
// 		// parent waits for foreground job to terminate
// 		if(!bg){
// 			int status;
// 			if(waitpid(pid, &status, 0)<0){
// 				unix_error("waitfg: waitpid error");
// 			}
// 		}
// 		else{
// 				printf("%s %s",pid,cmdline); 
// 		}
// 	return;
// 	}

// }

/*
Problem with our eval function (simple shell)

Our example shell correctly waits for and reaps foregound processes

However, it does not reap background processes
currently they will become zombies which could lead to a memory leak, causing the kernel to run out of process slots

ECF (Exceptional Control Flow) is a way to handle these cases

Our solution will use the kernel to interrupt regular processing to alert us when a background process has completed
In unix this is done using signals


*/

/*
Signals:

a singal is a small message that notifies a process that some event has occured,
these are akin to exceptions in other languages

Singals are generated by the kernel and sent to a process, and have small integer values as identifiers

Sending a Signal:
	Kernel sends a signal to a destination process by updating some state in the context of the destination process 

	Kernel sends a singal for a few reasons:

		Kenerl detects a system event like divide by zero (SIGFPE) 
		Termination of a child process (SIGCHLD)
		Another process has invoked the kill system call to explicty request the kernel to send a signal to destination process
Reciving a signal:

	A destination process recives a signal when it is forced by the kernel to react in someway to the delivery of the signal
		Process can ignore the signal
		Process can catch the signal
		Process can terminate
		Process can perform the default action

Pending and BLocking Signals:

	singal is pending if it is sent but not yet recived:
		there can be at most one pending signal of a particular type
		Important: Signals are to queued. If a signal is sent to a process that already has a pending signal of the same type, t0he new signal is discarded

	A process can block the receipt of certian signals		
		blocked singals can be delivered but cannot be recieved until unblocked
	
	A pending singal is recived at most once

	Pending/Blocked bits
		Kernel maintains pending and blocked bit vectors in the context of each process:
		    pending: repersents the set of pending singals
				kernel sets bit k in pending when a signal of type k is delivered
				kernel clears bit k in pending when a singal of type k is recieved
			blocked: reperesents the set of blocked signals
				Can be set and cleared using sigprocmask system call
				Also referred to as the signal mask


Singal Process Groups:
	every singal belongs to exactly one process group

	We can use the visual of a binary tree to repersent this relationship
		Root is the init process
		Each node is a process
		Each edge is a parent-child relationship
		Each leaf is a process group

Sending Singals with /bin/kill
	/bin/kill program senda arbitrary signal to a process or process group
	/bin/kill -9 pid	// sends SIGKILL to process with pid
	/bin/kill -9 -pgid	// sends SIGKILL to process group with pgid

Sending Signals from the keyboard:
	Typing (crtl-c)^(crtl-z) causes the kerner to send a SIGINT/SIGTSTP to every job in the foreground process group
		SIGINT: default action is to terminate the process
		SIGTSTP: default action is to stop(suspend) the process


*/

//sending signals with kill function
void fork12(){
	int N = 5;
	pid_t pid[N];
	int i, child_status;

	// create N children
	for(i=0;i<N;i++){
		if((pid[i]=fork())==0){
			//child inf loop
			while(1);
		}
	}

	// send SIGKILL to all children
	for(i=0;i<N;i++){
		printf("Killing process %d\n",pid[i]);
		kill(pid[i],SIGINT);

	}

	// wait for all children to terminate
	for(i=0;i<N;i++){
		pid_t wpid = wait(&child_status);
		if(WIFEXITED(child_status)){
			printf("Child %d termiated with exit status %d\n",wpid,WEXITSTATUS(child_status));
		}
		else{
			printf("Child %d terminated abnormally\n",wpid);
		}
	}
	
}

/* 
Receiving Signals
	Supposed kernel is returning from an exception handler and is ready to pass control to process p

	Kenerl computes pnb = pending & ~ blocked : the set of nonblocked signals for p

	if(pnb==0): pass control to the next function
	else:
		choose least nonzero bit k in pnb, and force process p to recieve signal k
		The recepit of the signal triggers some action by p
		reapeat for all nonzero k in pnb
		pass control to next logical flow

Default Actions:
	Each signal has a default action that is taken if the process does not catch the signal
		terminate: process is terminated
		ignore: signal is ignored until restarted by a SIGCONT
		stop: process is stopped

Installing Signal Handlers:
	the singal function modifies the defualt action associated with reciept of a signal from signum:
		
		handler_t *signal(int signum, handler_t *handler)

		Different values for handler
			SIG_IGN: ignores signals of a type signum
			SIG_DFL: revert to default action for signals of type signum
			Otherwise, handler is the address of a user-level signal handler function
				called when process recieves a signal of type signum
				this is referred to as "installing" the hanlder
				Executing the handler is referred to as "catching"/"handling" the signal
				When handler executes its return statement, control is returned to the instruction in the 
				control flow of the process that was interrupted by the signal
*/

// Signal Handler Example
void signal_handler(int signum){
	/*
	We can use the signal handler to catch the signal and perform some action
	this extends to interrupting other signal handlers

	Blocking and Unblocking:
		implicit blocking: Kernel blocks any pending siggnals of type being currently handled
		e.g. if a process is handling a SIGINT, then any pending SIGINT signals are blocked

		Explicit blocking: process can block and unblock signals using sigprocmask function

		sigemptyset(sigset_t *set): initializes set to empty
		sigfillset(sigset_t *set): initializes set to full
		sigaddset(sigset_t *set, int signum): adds signum to set
		sigdelset(sigset_t *set, int signum): removes signum from set

		consider the following example

		sigset_t mask, prev_mask;
		sigemptyset(&mask);
		sigaddset(&mask, SIGINT);

		//block SIGINT and save previous mask
		sigprocmask(SIG_BLOCK, &mask, &prev_mask);
		// do something
		// restore previous mask
		sigprocmask(SIG_SETMASK, &prev_mask, NULL);	
	
	*/



	printf("So, you think you can stop the bomb with control-c, do you?\n");
	sleep(2);
	printf("Well...");
	fflush(stdout);
	sleep(1);
	printf("OK, then...\n");
	exit(0);
}

/*
Safe Signal Handling:

Handlers are tricky because they run concurrently with our main program and share the same global data
	We must be careful

Guidelines for writing safe signal handlers:
	G0: Keep your handlers as simple as possible; e.g. set a global flag and return
	G1: Call only asynchronous-singal-safe functions in your handlers; e.g. printf, sprintf, malloc, free, exit
	G2: Save and restore errno on entry and exit; so that other handlers do not overwrite it
	G3: Protect access to shared data structures by temporarily blocking all signals: prevents possible corruption of shared data
	G4: Declare global variables as volatile: To prevent compliler from storing them in registers
	G5: Declare global flags as volatile and sig_atmoic_t
		flag: variable that is only read or written
		Flag declared this way dose not need to be protected like other global vars

Async-Singal-Saftey:
	Function is async-signal-safe if either reentrant(e.g not all vars stored on stack frame) or it is non interruptable by signals
	Posix guarentees 117 functions are async-signal-safe:
		Source "man 7 signal"
		popular funcs on this list: _exit, write, wait, waitpid, sleep, kill
		populat funcs not on this list: printf, malloc, free, exit
		Unfortunatly write is the only async-signal-safe way to print to stdout




*/

/*
Safely generating formatted output
use of the reentrant SIO from csapp.c in the signal handler

ssize_t sio_puts(char s[]); // write string to stdout
ssize_t sio_putl(long v); // write long to stdout
ssize_t sio_put(int v); // write int to stdout

*/
void sigint_handler(int sig){
	// safe SIGINT handler
	
	Sio_puts("So, you think you can stop the bomb with control-c, do you?\n");
	sleep(2);
	Sio_puts("Well...");
	fflush(stdout);
	sleep(1);
	Sio_puts("OK, then...\n");
	_exit(0);
}

/*
Incorrect Signal Handler Example

***** Pending signals are not queued *****
	For each signal type one bit indicates wether or not a signal is pending
	thus at most one signal of a particular type can be pending at a time
You cannot use signals to count events, such as children terminating

e.g N = 5
*/

void incorrectChildHandler(int signum){
	int olderrno = errno;
	pid_t pid;
	if((pid = wait(NULL)) <0 ){
		Sio_error("wait error");
	}
	ccount--;
	Sio_puts("Handler reaped child\n");
	Sio_putl((long)pid);
	Sio_puts("\n");
	sleep(1);
	errno = olderrno;

}

void correctChildHandler(int signum){
	int olderrno = errno;
	pid_t pid;
	while((pid = wait(NULL))>0){
		ccount--;\
		Sio_puts("Handler reaped child\n");
		Sio_putl((long)pid);
		Sio_puts("\n");
	}
	if(errno!=ECHILD){
		Sio_error("wait error");
	}
	errno = olderrno;
}

// 
void fork14(){
	int choice;
	printf("Enter 1 for correct handler, 2 for incorrect handler\n");
	scanf("%d",&choice);


	int i, N=5;
	pid_t pid[N];
	ccount = N;
	if(choice == 1){
		Signal(SIGCHLD, sigchld_handler);
	}
	else{
		Signal(SIGCHLD, incorrectChildHandler);
	}
	for(i=0;i<N;i++){
		if((pid[i]=fork())==0){
			sleep(1);
			exit(0);
		}
	}
	while(ccount>0){
		// parent waits for all children to terminate
	}

}



int main(){
	int ccount = 0;	
	
	
	printf("Selct a fork example to run\n");
	printf("1: fork\n");
	printf("2: fork2\n");
	printf("3: fork7\n");
	printf("4: fork8\n");
	printf("5: fork9\n");
	printf("6: fork10\n");
	printf("7: fork11\n");
	printf("8: fork12\n");
	printf("9: signal_handler\n");
	printf("10: sigint_handler\n");
	printf("11: fork14\n");
	printf("Enter choice: ");
	int choice;
	scanf("%d",&choice);
	switch(choice){
		case 1:
			forkExample();
			break;
		case 2:
			fork2();
			break;
		case 3:
			fork7();
			break;
		case 4:
			fork8();
			break;
		case 5:
			fork9();
			break;
		case 6:
			fork10();
			break;
		case 7:
			fork11();
			break;
		case 8:
			fork12();
			break;
		case 9:
			// instsll the SIGINT handler
			if(signal(SIGINT, signal_handler)==SIG_ERR){
				unix_error("signal error");
			}
			// wait
			pause();
			break;
		case 10:
			// install the SIGINT handler
			if(signal(SIGINT, sigint_handler)==SIG_ERR){
				unix_error("signal error");
			}
			// wait
			pause();
			break;		
		default:
			printf("Invalid choice\n");
			break;
	}
	return 0;

}


// EOF
