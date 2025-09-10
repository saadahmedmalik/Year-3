// Saad Ahmed Malik, ECE 231, Lab Assignment 4 //

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <pthread.h>

#define BUFFER_SIZE 10

void* threadFunction(void* input){

    long* var = (long*) input;

    int gpio_number = 67;

    char InterruptPath[40];
    sprintf(InterruptPath, "/sys/class/gpio/gpio%d/value", gpio_number);
    int epfd;
    struct epoll_event ev;

    FILE* fp = fopen(InterruptPath, "r");

    int fd = fileno(fp);

    epfd = epoll_create(1);

    ev.events = EPOLLIN | EPOLLET;

    ev.data.fd = fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    int capture_interrupt;
    struct epoll_event ev_wait;
    struct timespec tm;

    int buffer_index = 0;

    for (int i = 0; i < 10; i++){
        capture_interrupt = epoll_wait(epfd, &ev_wait, 1, -1);
        if (capture_interrupt == 1){
            clock_gettime(CLOCK_MONOTONIC,&tm);
            var[i] = tm.tv_sec;

        }
    }
    close(epfd);
    return 0;

}

int main() {
    
    long data[10];
    pthread_t thread_id;
    printf("Before Thread\n");

    pthread_create(&thread_id, NULL, threadFunction, (void*)(data));

    pthread_join(thread_id, NULL);
    printf("After Thread\n");

    for (int i = 0; i<BUFFER_SIZE; i++){
        printf("%li\n",data[i]);
    }

    pthread_exit(0);
    return 0;
}