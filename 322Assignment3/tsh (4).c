/*
 * tsh - A tiny shell program with job control
 *
 * <William Neel 33855470>
 * <Saad Malik 33783667>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include "csapp.c"
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Misc manifest constants */
//#define MAXLINE    1024   /* max line size */ Redef warning from csapp.c -> defined in csapp.c will change to 1024 there and comment out here
#define MAXARGS     128   /* max args on a command line */
#define MAXJOBS      16   /* max jobs at any point in time */
#define MAXJID    1<<16   /* max job ID */

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */

/*
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

/* Global variables */
extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
int nextjid = 1;            /* next job ID to allocate */
char sbuf[MAXLINE];         /* for composing sprintf messages */

struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */


/* Function prototypes */

/* redirection and pipeline functions */
int checkMultiRedir(char **argv);
void handleMultiRedir(char **argv, char *cmdline, int bg, int numRedir);


int checkPipelineChar(char **argv);
void handlePipe(char **argv, char *cmdline, int bg);

int checkRedirChar(char **argv);
void redir2Out(char **argv, char* cmdline, int bg);
void redir2In(char **argv, char *cmdline, int bg);
void redir2Append(char **argv, char *cmdline, int bg);
void redir2Err(char **argv, char *cmdline, int bg);

/* Signal handling functions */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);
void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv);
void sigquit_handler(int sig);
void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs);
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid);
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid);
int pid2jid(pid_t pid);
void listjobs(struct job_t *jobs);

void usage(void);
typedef void handler_t(int);

/* End function prototypes */

/*
 * main - The shell's main routine
 */
int main(int argc, char **argv)
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
            break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
            break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
            break;
        default:
            usage();
        }
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler);

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1) {

        /* Read command line */
        if (emit_prompt) {
            printf("%s", prompt);
            fflush(stdout);
        }
        if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
            app_error("fgets error");
        if (feof(stdin)) { /* End of file (ctrl-d) */
            fflush(stdout);
            exit(0);
        }

        /* Evaluate the command line */
        eval(cmdline);
        fflush(stdout);
        fflush(stdout);
    }

    exit(0); /* control never reaches here */
}


/*
 * eval - Evaluate the command line that the user has just typed in
 *
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.
*/
void eval(char *cmdline) {
    char *argv[MAXARGS]; // argument list
    char buf[MAXLINE]; // holds modified command line
    strcpy(buf, cmdline); // copy cmdline into buffer
    int bg = parseline(buf, argv); // parse command line and check for background job
    pid_t chld_pid; //child pid
    sigset_t block_mask, prev_mask; // singal vectors
        

    // check for empty command line
    if (argv[0] == NULL){
        return;
    }

    if(checkMultiRedir(argv)){
        return;
    }

    // Check for pipeline character
    if(checkPipelineChar(argv)){
        return;
    }

    // Check for redirection character
    if(checkRedirChar(argv)){
        return;
    }

    // Check if command is built-in
    if (!builtin_cmd(argv)) {
        // Block all signals while creating process and job
        sigemptyset(&block_mask);
        sigfillset(&block_mask);    
        sigprocmask(SIG_BLOCK, &block_mask, &prev_mask); // Block all signals

        if ((chld_pid = fork()) == 0) { // corrected Fork to fork
            setpgrp(); // set process group id
            sigprocmask(SIG_SETMASK, &prev_mask, NULL); // unblock for child

            if (execve(argv[0], argv, environ) < 0) { // execute & check command
                printf("%s: command not found.\n", argv[0]);
                exit(0); // exit on error
            }
        }

        // Handling background and foreground jobs
        if (bg){
            addjob(jobs, chld_pid, BG, cmdline); // add job to job list
            sigprocmask(SIG_SETMASK, &prev_mask, NULL); // unblock signals for parent process
            printf("[%d] (%d) %s", nextjid - 1, chld_pid, cmdline); // print job info
        } 
        else{
            addjob(jobs, chld_pid, FG, cmdline); // add job to job list
            sigprocmask(SIG_SETMASK, &prev_mask, NULL); // restore original mask
            waitfg(chld_pid); // wait for foreground job to finish
        }
    }
    return;
}


/*
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.
 */
int builtin_cmd(char **argv) {
    if (!strcmp(argv[0], "quit")) {
        // quit command
        exit(0);
    }
    else if (!strcmp(argv[0], "&")) {
        // Ignore singleton &
        return 1;
    }
    else if (!strcmp(argv[0], "jobs")){
        // list jobs command
        listjobs(jobs);
        return 1;
    }
    else if (!strcmp(argv[0], "fg") || !strcmp(argv[0], "bg")){ 
        // fg or bg command
        do_bgfg(argv);
        return 1;
    }
    else {
        return 0; // not a built-in command
    }
}


/*
 * parseline - Parse the command line and build the argv array.
 *
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.
 */
int parseline(const char *cmdline, char **argv)
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
        buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
        buf++;
        delim = strchr(buf, '\'');
    }
    else {
        delim = strchr(buf, ' ');
    }

    while (delim) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* ignore spaces */
               buf++;

        if (*buf == '\'') {
            buf++;
            delim = strchr(buf, '\'');
        }
        else {
            delim = strchr(buf, ' ');
        }
    }
    argv[argc] = NULL;

    if (argc == 0)  /* ignore blank line */
        return 1;

    /* should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0) {
        argv[--argc] = NULL;
    }
    return bg;
}


/*
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(char **argv){
    struct job_t* job;
    // Check given argument is valid
    if(argv[1] == NULL){
        // no argument given 
        printf("%s command requires PID or %%jobid\n",argv[0]);
        return;
    }
    if(!(isdigit(argv[1][0]) || argv[1][0] == '%')){
        // invalid argument
        printf("%s: argument must be PID or %%jobid\n",argv[0]);
        return;
    }

    // search for job based off of PID or JID
    
    if(isdigit(argv[1][0])){
        // get job based off of pid
        job = getjobpid(jobs, atoi(argv[1]));
        if(job == NULL){
            printf("(%d): Process does not exist\n", atoi(argv[1]));
            return;
        }
    }
    else{
        // get job based off of JID
        job = getjobjid(jobs, atoi(&argv[1][1]));
        if(job==NULL){
            printf("%s: Job does not exist\n",argv[1]);
            return;
        }
    }

    // execute operation as BG || FG
    if(strcmp(argv[0], "bg") == 0){
         kill(-(job->pid), SIGCONT); // send sigcont to job pid
         job->state = BG; // put job in BG
         printf("[%d] (%d) %s", job->jid, job->pid, job->cmdline);
    }
    else{
        kill(-(job->pid), SIGCONT); // send sigcont to job pid
        job->state = FG; // put job in fg
        waitfg(job->pid); // wait for fg process to execute
    }
}


/*
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid){
    struct job_t* job;
    // Loop until the job is either completed or removed
    while (1) {
        job = getjobpid(jobs, pid); // Retrieve job by pid
        // If the job doesn't exist or is no longer in the foreground exit loop
        if ((job == NULL) || (job->state != FG)) {
            return;
        }
        sleep(1); // Wait one second and checking again
    }
}


// End of Helper Functions


// Start of Redirection/Pipelining Functions   


/*
Checks commandline for multiple redirection arguments
Params: char **argv 
Return: int, 1 if success, 0 if failure

-> improve robustness of the program if I integreate with the other redirection functions

*/
int checkMultiRedir(char **argv){
    int i = 0; // init argument counter
    int numRedir = 0; // init number of redirection characters
    while(argv[i] != NULL){
        if(strcmp(argv[i], ">") == 0 || strcmp(argv[i], "<") == 0 || strcmp(argv[i], ">>") == 0 || strcmp(argv[i], "2>") == 0){
            numRedir++; // increment number of redirection characters
        }
        i++; // increment counter
    }
    if(numRedir > 1){
        handleMultiRedir(argv, argv[i], 0, numRedir); // handle multiple redirection characters
        return 1; // return true if more than one redirection character is found
    }
    else{
        return 0; // return false if only one redirection character is found
    }
}


/*
handleMultiRedir
Params: char **argv, char *cmdline, int bg, int numRedir
return: void


NEED TO IMPLEMENT SIGNAL HANDLING WHEN CREATING CHILD PROCESS
*/
void handleMultiRedir(char **argv, char *cmdline, int bg, int numRedir) {
    int i = 0; // init counter i
    int fileDes[numRedir]; // file descriptors
    char *redirType[numRedir]; // redirection types
    char *redirFile[numRedir]; // redirection file names
    pid_t redir_pid; // process id

    // Find and strip the redirection characters
    int redirCount = 0;
    while (argv[i] != NULL) {
        if (strcmp(argv[i], ">") == 0 || strcmp(argv[i], "<") == 0 || strcmp(argv[i], ">>") == 0 || strcmp(argv[i], "2>") == 0) {
            redirType[redirCount] = argv[i]; // store redirection type
            redirFile[redirCount] = argv[i + 1]; // store redirection file name
            argv[i] = NULL; // set redirection character to NULL
            redirCount++;
        }
        i++;
    }

    // Open the files and handle the redirection
    for (int j = 0; j < redirCount; j++){
        if (strcmp(redirType[j], ">") == 0){
            fileDes[j] = Open(redirFile[j], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); // open file for writing
            if (fileDes[j] < 0){
                unix_error("Redirection Failure: Error opening file");
                return; // return if file open fails
            }
        } 
        else if (strcmp(redirType[j], "<") == 0){
            fileDes[j] = Open(redirFile[j], O_RDONLY, 0); // open file for reading
            if (fileDes[j] < 0){
                unix_error("Redirection Failure: Error opening file");
                return; // return if file open fails
            }
        } 
        else if (strcmp(redirType[j], ">>") == 0){
            fileDes[j] = Open(redirFile[j], O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); // open file for appending
            if (fileDes[j] < 0){
                unix_error("Redirection Failure: Error opening file");
                return; // return if file open fails
            }
        } 
        else if (strcmp(redirType[j], "2>") == 0){
            fileDes[j] = Open(redirFile[j], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); // open file for writing stderr
            if (fileDes[j] < 0){
                unix_error("Redirection Failure: Error opening file");
                return; // return if file open fails
            }
        }
    }

    
    // Refactor below ASAP need to implement signal handling & fix syntax mismatch

    // create child process

    if ((redir_pid = Fork()) == 0) { // if Child process
        // For each redirection detected, handle that redirection
        for (int j = 0; j < redirCount; j++) {
            // Write or append
            if (strcmp(redirType[j], ">") == 0 || strcmp(redirType[j], ">>") == 0) {
                dup2(fileDes[j], STDOUT_FILENO);
            } 
            // Read
            else if (strcmp(redirType[j], "<") == 0) {
                dup2(fileDes[j], STDIN_FILENO);
            } 
            // Write to stderr
            else if (strcmp(redirType[j], "2>") == 0) {
                dup2(fileDes[j], STDERR_FILENO);
            }
            // close file descriptor after redirection
            close(fileDes[j]);
        }

        // Execute command
        execvp(argv[0], argv);
        // Error handling -> should not reach this point
        unix_error("Exec failed");
        exit(EXIT_FAILURE);
    } else if (redir_pid > 0) { // Parent process
        if (!bg) waitpid(redir_pid, NULL, 0);
    } else {
        unix_error("Fork failed");
    }
}


/* 
CheckRedirChar
Parameters: char **argv
Description: This function checks for redirection characters in the command line
*/
int checkRedirChar(char **argv){
    int i = 0; // init argv counter
    while(argv[i] != NULL){
        //printf("%d\n", i); // debugging 
        if(strcmp(argv[i], ">") == 0){
            redir2Out(argv, argv[i], 0); // handle redirection
            return 1; // return true if found
        }
        else if(strcmp(argv[i],"<") == 0){
            redir2In(argv, argv[i], 0); // handle redirection
            return 1; 
        }
        else if(strcmp(argv[i], ">>") == 0){
            redir2Append(argv, argv[i], 0);
            return 1;
        }
        else if(strcmp(argv[i], "2>") == 0){
            redir2Err(argv, argv[i], 0);
            return 1;
        }
        i++; // increment counter   
    }
    return 0; // return false if not found

}

/*
Redir2Err
Parameters: char **argv, char *cmdline, int bg
Description: This function handles the redirection of standard error to a file

NEEDS SIGNAL HANDLING IMPLEMENTED
*/
void redir2Err(char **argv,char *cmdline, int bg){
    int i = 0; // init counter
    int pass = 0; // init pass variable
    int fileDes; // file descriptor
    pid_t redir_pid; // process id
    sigset_t mask, mask_prev;

    while(argv[i] != NULL){
        if(strcmp(argv[i], "2>") == 0){
            pass++; // pass redirection
            break;
        }
        i++; // increment counter 
    }

    if(pass == 0){
        return; // return if no redirection character is found
    }

    fileDes = Open(argv[i+1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); // open file for writing
    if(fileDes < 0){
        unix_error("Redirection Failure: Error opening file\n"); // print error message
        return; // return if file open fails
    }

    sigemptyset(&mask);
    sigfillset(&mask);    
    sigprocmask(SIG_BLOCK, &mask, &mask_prev); // Block all signals

    argv[i] = NULL; // set last element in string to NULL
    if((redir_pid = Fork()) == 0){
        dup2(fileDes, 2); // copy file discriptor to stderr
        close(fileDes); // close file descriptor
        sigprocmask(SIG_SETMASK, &mask_prev, NULL); // unblock for child        

        if(execve(argv[0], argv, environ) < 0){
            unix_error("Redirection Failure: Error executing command\n"); // print error to user
            exit(1); // exit on error
        }
    }

    // add job to job list
    if(bg){
        // add job background tasks
        addjob(jobs, redir_pid, bg, cmdline); // append job to job list
        sigprocmask(SIG_SETMASK, &mask_prev, NULL); // unblock signals for parent process
        printf("[%d] (%d) %s", nextjid - 1, redir_pid, cmdline); // print job info

    }
    else{
        // add job in foreground tasks
        addjob(jobs, redir_pid, FG, cmdline); //append job to foregound
        sigprocmask(SIG_SETMASK, &mask_prev, NULL); // unblock signals for parent process
        waitfg(redir_pid); // wait for foreground job to finish
    }

}


/*
Redir2Append
Parameters: char **argv, char *cmdline, int bg
Description: This function handles the redirection of standard output to a file in append mode

NEEDS SIGNAL HANDLING IMPLEMENTED
*/
void redir2Append(char **argv, char *cmdline, int bg){
    int i = 0; // init counter i
    int pass = 0; // init pass variable
    int fileDes; // file descriptor
    pid_t redir_pid; // process id
    sigset_t mask, mask_prev; // initalize bit vectors

    printf("append redir \n"); // debugging
    while(argv[i]!=NULL){
        if(strcmp(argv[i], ">>") == 0 ){
            pass++; // allow redirection as we found the character
            break;
        }
        i++; // inc counter
    }

    if(pass == 0){
        return; // return if no redirection character is found
    }

    fileDes = Open(argv[i+1], O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); // open file for writing
    if(fileDes < 0){
        unix_error("Redirection Failure: Error opening file\n"); // print error message
        return; // return if file open fails
    }

    argv[i] = NULL; // set last element in string to NULL

    sigemptyset(&mask);
    sigfillset(&mask);    
    sigprocmask(SIG_BLOCK, &mask, &mask_prev); // Block all signals


    if((redir_pid = Fork()) == 0){
        dup2(fileDes, 1); //copy file des to stdout
        close(fileDes); // close file descriptor
        sigprocmask(SIG_SETMASK, &mask_prev, NULL); // unblock for child
        if(execve(argv[0], argv, environ) < 0){
            unix_error("Redirection Failure: Error executing command\n"); // print error to user
            exit(1); // exit on error
        }
    }

    if(bg){
        addjob(jobs, redir_pid, BG, cmdline); // append job to job list
        sigprocmask(SIG_SETMASK, &mask_prev, NULL); // unblock signals for parent process
        printf("[%d] (%d) %s", nextjid - 1, redir_pid, cmdline); // print job info
    }
    else{
        addjob(jobs, redir_pid, FG, cmdline); // append job to job list
        sigprocmask(SIG_SETMASK, &mask_prev, NULL); // unblock signals for parent process
        waitfg(redir_pid); // wait for foreground job to finish
    } 

}


/*
redir2In
Parameters: char **argv, char *cmdline, int bg
Description: This function handles the redirection of standard input to a file

NEEDS SIGNAL HANDLING IMPLEMENTED
*/
void redir2In(char **argv, char *cmdline, int bg){
    int i = 0; // init counter i
    int pass = 0; // init pass variable
    int fileDes; // file descriptor
    pid_t redir_pid; // process id
    sigset_t mask, mask_prev;

    while(argv[i]!=NULL){
        if(strcmp(argv[i], "<") == 0 ){
            pass++; // allow redirection as we found the character
            break;
        }
        i++; // inc counter
    }

    if(pass == 0){
        return; // return if no redirection character is found
    }

    fileDes = Open(argv[i+1], O_RDONLY, 0); // open file for reading
    if(fileDes < 0){
        unix_error("Redirection Failure: Error opening file\n"); // print error message
        return; // return if file open fails
    }

    // Block all signals while creating process and job
    sigemptyset(&mask);
    sigfillset(&mask);    
    sigprocmask(SIG_BLOCK, &mask, &mask_prev); 
    
    argv[i] = NULL; // replace redirection character with NULL
    if((redir_pid=Fork()) == 0){
        dup2(fileDes, 0); // copy file descriptor to standard in
        close(fileDes); // close descriptor
        setpgrp(); // set process group id
        sigprocmask(SIG_SETMASK, &mask_prev, NULL); // unblock for child
        if(execve(argv[0], argv, environ) < 0){
            unix_error("Redirection Failure: Error executing command\n"); // print error to user
            exit(1); // exit on error
        }
    }

    addjob(jobs, redir_pid, FG, cmdline); // append job to job list
    sigprocmask(SIG_SETMASK, &mask_prev, NULL); // unblock signals for parent process
    waitfg(redir_pid); // wait for foreground job to finish
    
}

/*
redir2Out:
Parameters: char **argv, char *cmdline, int bg
Description: This function handles the redirection of standard output to a file

NEEDS SIGNAL HANDLING IMPLEMENTED
*/
void redir2Out(char **argv, char* cmdline, int bg){
    int i = 0; // init counter i
    int pass = 0; // init pass variable
    int fileDes; // file descriptor
    pid_t redir_pid; // process id
    sigset_t mask, mask_prev;

    // 
    while(argv[i] != NULL){
        if(strcmp(argv[i], ">") == 0){
            pass++; // increment pass counter
            break; 
        }
        i++; // increment counter
    }

    if(pass == 0){
        return; // return if no redirection character is found
    }

    
    fileDes = Open(argv[i+1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); // open file for writing
    if(fileDes < 0){
        unix_error("Redirection Failure: Error opening file\n"); // print error message
        return; // return if file open fails
    }

    argv[i] = NULL; // set last element in string to NULL
    sigemptyset(&mask);
    sigfillset(&mask);    
    sigprocmask(SIG_BLOCK, &mask, &mask_prev); // Block all signals



    if((redir_pid = Fork()) == 0){
        dup2(fileDes, 1); // copy file descriptor to stdout
        close(fileDes); // close file descriptor
        setpgrp(); // set process group id
        sigprocmask(SIG_SETMASK, &mask_prev, NULL); // unblock for child


        if(execve(argv[0], argv, environ) < 0){
            unix_error("Redirection Failure: Error executing command\n"); // print error to user
            exit(1); // exit on error
        }
    }

    if(bg){
        addjob(jobs, redir_pid, BG, cmdline); // append job to job list
        sigprocmask(SIG_SETMASK, &mask_prev, NULL); // unblock signals for parent process
        printf("[%d] (%d) %s", nextjid - 1, redir_pid, cmdline); // print job info
    }
    else{
        addjob(jobs, redir_pid, FG, cmdline); // append job to job list
        sigprocmask(SIG_SETMASK, &mask_prev, NULL); // unblock signals for parent process
        waitfg(redir_pid); // wait for foreground job to finish
    }
    
}


/*
checkPipelineChar:
Parameters: char **argv
Description: This function checks for pipeline characters in the command line
*/
int checkPipelineChar(char **argv){
    int i = 0; // init argument counter
    while(argv[i]!=NULL){ // loop until end of arguments
        if(strcmp(argv[i], "|") == 0){ // check for pipeline character
            handlePipe(argv, argv[i], 0); 
            return 1; // return true if found
        }
        i++; // increment counter
    }
    return 0; // return false if not found
}


/*
handlePipe:
Parameters: char **argv, char *cmdline, int bg
Description: This function handles the pipeline functionality

NEEDS SIGNAL HANDLING IMPLEMENTED
*/
void handlePipe(char **argv, char *cmdline, int bg){
    // we need to seperate the command line into sections to handle the pipeline
    // we create a "Pipe" to connect the two processes
    int i = 0; // init counter i
    int j = 0; // init counter j
    int fileDes[2];// File descriptors for pipe, tells us where to read and write
    pid_t pid_in, pid_out; // process ids for I/O
    char *argv_in[MAXARGS]; // input argument array
    char *argv_out[MAXARGS]; // output argument array
    sigset_t mask, mask_prev; // initalize singnal vectors
    
    // iterate through argv until we find the pipline character
    while(argv[i] != NULL && strcmp(argv[i], "|") != 0){
        argv_in[i] = argv[i]; // will add the first command to argv_in: expected behavior of pipeline is to take the output of the first command and use it as the input of the second command 
        i++; // increment counter
    }
    // we dont iterate i because we want to skip the pipeline character
    argv_in[i] = NULL; // Set last element in string to NULL (note we dont use '\0' because we are working with an array of strings)
    // now moce past the pipeline character
    
    i++; // increment counter
    // iterate through the rest of the arguments
    while(argv[i] != NULL){
        argv_out[j] = argv[i]; // add the rest of the arguments to argv_out
        i++; // increment counter
        j++; // increment counter
    }
    argv_out[j] = NULL; // set last element in string to NULL

    /* 
    now we need to create a pipe
    pipe() creates a pipe, a unidirectional data channel that can be used for interprocess communication
    this opens both ends of the pipe and returns two file descriptors
    fileDes[0] is the read end of the pipe -> first command will write to this end
    fileDes[1] is the write end of the pipe -> second command will read from this end
    
    */
    if(pipe(fileDes) < 0){
        unix_error("Error creating pipeline");
        return; // return if pipe fails
    }

    // Block all signals while creating process and job
    sigemptyset(&mask);
    sigfillset(&mask);    
    sigprocmask(SIG_BLOCK, &mask, &mask_prev); // Block all signals


    // fork first process
    // we need to fork the first process to create a child process that will execute the first command
    if((pid_in = Fork()) == 0){
        close(fileDes[0]); // close read end of pipe
        dup2(fileDes[1], STDOUT_FILENO); // copy write end of pipe to stdout
        close(fileDes[1]); // close write end of pipe

        setpgrp(); // set process group id
        sigprocmask(SIG_SETMASK, &mask_prev, NULL); // unblock for child

        // execute first command in pipeline
        if(execve(argv_in[0], argv_in, environ) < 0){
            unix_error("Pipeline Failure: Error executing command 1\n"); // print error to user on pipeline failure
            return; // return on error
        }

    }

    if((pid_out = Fork() == 0)){
        close(fileDes[1]); // close write end of pipe
        dup2(fileDes[0], STDIN_FILENO); // copy read end of pipe to stdin
        close(fileDes[0]); // close read end of pipe

        setpgrp(); // set process group id
        sigprocmask(SIG_SETMASK, &mask_prev, NULL); // unblock for child
        
        // execute second command in pipeline
        if(execve(argv_out[0], argv_out, environ) < 0){
            unix_error("Pipeline Failure: Error executing command 2\n"); // print error to user on pipeline failure
            return;           
        }
    }

    for(int q=0; q<2; q++){
        close(fileDes[q]); // close both ends of the pipe
    }

    // add jobs to job list
    if(bg){
        addjob(jobs, pid_in, BG, cmdline); // append job to job list
        sigprocmask(SIG_SETMASK, &mask_prev, NULL); // unblock signals for parent process
        printf("[%d] (%d) %s", nextjid - 1, pid_in, cmdline); // print job info
    }
    else{
        addjob(jobs, pid_in, FG, cmdline); // append job to job list
        sigprocmask(SIG_SETMASK, &mask_prev, NULL); // unblock signals for parent process
        waitfg(pid_in); // wait for foreground job to finish
    }
    return; // add return statement
}

/*****************
 * Signal handlers
 *****************/

/*
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.
 */
void sigchld_handler(int sig){
	int chld_status=-1; // status of child

	while(1) {
		pid_t pid = waitpid((pid_t) -1, &chld_status, (WUNTRACED | WNOHANG)); // waits for any child to change state
		if(pid < 1){break;} // no more children to reap
        // Check if child was stopped -> update job list and set state of job to stopped
		if(WIFSTOPPED(chld_status)){getjobpid(jobs, pid)->state = ST;} 
        // Check if child received sigint -> delete from job list
        if(WIFSIGNALED(chld_status)){deletejob(jobs, pid);} 
        // Check if child exited normally -> delete from job list
	    if(WIFEXITED(chld_status)){deletejob(jobs, pid);} 
    }
    return; 
}

/*
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.
 */
void sigint_handler(int sig){   
    int olderrno = errno; // store errno
    sigset_t mask, prev; // initalize signal vectors
    pid_t pid = fgpid(jobs); // get process group id of foreground job
    sigemptyset(&mask); // empty mask
    sigaddset(&mask, SIGINT); // add SIGINT to mask
    sigprocmask(SIG_BLOCK, &mask, &prev); // set blocked vector
    kill(-pid, SIGINT); // send SIGINT to process group
    sigprocmask(SIG_SETMASK, &prev, NULL); // reset blocked vector
    errno = olderrno; // restore errno
    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.
 */
void sigtstp_handler(int sig){
    int olderrno = errno; // store errno
    pid_t pid = fgpid(jobs); // get pgid
    kill(-pid, SIGTSTP); // send suspend to foreground group
    errno = olderrno; // restore errno
    return;
}

/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
        clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs)
{
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].jid > max)
            max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline)
{
    int i;

    if (pid < 1)
        return 0;

    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid == 0) {
            jobs[i].pid = pid;
            jobs[i].state = state;
            jobs[i].jid = nextjid++;
            if (nextjid > MAXJOBS)
                nextjid = 1;
            strcpy(jobs[i].cmdline, cmdline);
            if(verbose){
                printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            }
            return 1;
        }
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid){
    int i;

    if (pid < 1)
        return 0;

    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid == pid) {
            clearjob(&jobs[i]);
            nextjid = maxjid(jobs)+1;
            return 1;
        }
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].state == FG)
            return jobs[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
        return NULL;
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].pid == pid)
            return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid)
{
    int i;

    if (jid < 1)
        return NULL;
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].jid == jid)
            return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid)
{
    int i;

    if (pid < 1)
        return 0;
    for (i = 0; i < MAXJOBS; i++)
        if (jobs[i].pid == pid) {
            return jobs[i].jid;
        }
    return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs)
{
    int i;

    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid != 0) {
            printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
            switch (jobs[i].state) {
                case BG:
                    printf("Running ");
                    break;
                case FG:
                    printf("Foreground ");
                    break;
                case ST:
                    printf("Stopped ");
                    break;
            default:
                    printf("listjobs: Internal error: job[%d].state=%d ",
                           i, jobs[i].state);
            }
            printf("%s", jobs[i].cmdline);
        }
    }
}
/******************************
 * end job list helper routines
 ******************************/


/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void)
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig)
{
    sio_puts("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}
