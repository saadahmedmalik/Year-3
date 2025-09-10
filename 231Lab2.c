/* Saad Malik, ECE 231, Lab Assignment 2: BeagleBoneBlack*/

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>



int main() {
    FILE *timefile;
    FILE *LEDPoint = NULL;
    const char *LEDPathway = "/sys/class/leds/beaglebone:green:usr3/brightness";
    int i = 0;
    long a,b;
    struct timespec timeNow;
    char currentState;

    timefile = fopen("time_diff_file.txt", "w");

    printf("Initiate Program \n");
    while (i<1000) {
        printf("Iteration number: %d\n", (i+1));

        clock_gettime(CLOCK_MONOTONIC, &timeNow);
        a = timeNow.tv_nsec;
        
        LEDPoint = fopen(LEDPathway, "w");

        fscanf(LEDPoint, "%d", &currentState);
        
        printf("LED State is as follows: %c\n", currentState);
        if (currentState == '1') {
            fwrite("0", sizeof(char), 1, LEDPoint);
        }
        else { 
            fwrite("1",sizeof(char), 1, LEDPoint);
        }
        fclose(LEDPoint);

        clock_gettime(CLOCK_MONOTONIC, &timeNow);
        b = timeNow.tv_nsec;
        usleep(500000);
        fprintf(timefile, "%ld\n", abs(b-a));

        i++;
    }
    fclose(timefile);
    printf("Terminate Process: \n");
    return 0;
}

