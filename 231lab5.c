/*
Saad Malik, ECE 231: Lab 5
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h> 
#include <sys/epoll.h> 
#include "tempFunc.c"

struct sensor_type{
    struct timespec timestamp;
    double celcius_temp;
};

struct sensor_type buffer[10];

pthread_mutex_t lock;

bool createfile(FILE **fp);

void *inputThread(void* Array){
    
    struct sensor_type* buffer = (struct sensor_type*)Array;
    
    int gpio_number = 67;
    printf("Begin Input Thread \n");
    char InterruptPath[40];
    sprintf(InterruptPath, "/sys/class/gpio/gpio%d/value", gpio_number);
    int epfd;
    struct epoll_event ev;

    FILE *fp = fopen(InterruptPath, "r");
    int fd = fileno(fp);
    
    epfd = epoll_create(1);

    ev.events = EPOLLIN | EPOLLET;
     
    ev.data.fd = fd;

    epoll_ctl(epfd,EPOLL_CTL_ADD, fd, &ev);

    int capture_interrupt;
    struct epoll_event ev_wait;

    pthread_mutex_lock(&lock);
    
    for(int i=0; i < 10; i++){

        capture_interrupt = epoll_wait(epfd, &ev_wait, 1, -1);
        clock_gettime(CLOCK_MONOTONIC, &buffer[i].timestamp);
        buffer[i].celcius_temp = rtTemp(); 

    }

    pthread_mutex_unlock(&lock);
    
    printf("End Input Thread Process \n");
    
    fclose(fp);
    
    close(epfd);
    
}            

void *outputThread(void* Array){ 
        
    FILE *sensor_data = fopen("Saad_Malik_sensordata.txt","a"); 
    struct sensor_type* buffer = (struct sensor_type*)Array;
    
    pthread_mutex_lock(&lock);

    for(int i = 0; i < 10; i++){

        uint64_t timestamp_ns = ((uint64_t)buffer[i].timestamp.tv_sec * 1000000000) + ((uint64_t)buffer[i].timestamp.tv_nsec);

        printf("Timestamp: %llu | Temp in C: %f\n", timestamp_ns ,buffer[i].celcius_temp);
        fprintf(sensor_data, "%llu|%f\n", timestamp_ns, buffer[i].celcius_temp);
  
    }
    pthread_mutex_unlock(&lock);

}

bool createfile(FILE **fp){
    *fp = fopen("Saad_Malik_sensordata.txt", "a");   

    if (*fp == NULL) {
        printf("Error opening file: \n");
        return false;
    }
    return true;
}

int main(){

    int getData = 50;
    
    FILE *sensor_data;
    if (!createfile(&sensor_data)) {
        printf("File not unable to be created: \n");
        exit(-1);
    }

    pthread_t input_thread, output_thread;

    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init has failed\n");
        return 1;
    }

    while (getData > 0){
        
        if (pthread_create(&output_thread, NULL, outputThread, (void*)(&buffer)) !=0 ){
            printf("Error Creating output thread: end process \n");
            return -1;
        }    
        
        if (pthread_create(&input_thread, NULL, inputThread, (void*)(&buffer)) !=0 ){
            printf("Error Creating input thread: end process \n");
            return -1;
        }

        if (pthread_join(output_thread, NULL) !=0 ){
            printf("Error joining output thread: end process \n");
            return -1;
        }
        
        if (pthread_join(input_thread, NULL) !=0 ){
            printf("Error joining input thread: end process \n");
            return -1;
        }

        getData -= 10;
    }
    
    pthread_mutex_destroy(&lock);

    fclose(sensor_data);

    pthread_exit(0);

}
