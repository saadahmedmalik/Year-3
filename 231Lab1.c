/* Saad Malik, ECE 231, Lab Assignment 1: Basic Python Programming */

#include <stdio.h>
#include <unistd.h>
#include <time.h>

void meanANDtime(int array[], int *mean, struct timespec *timevalue);

int main() {
    struct timespec timevalue;
    int buffer[10];
    int i;
    int Number;
    int mean; 
    for (int r = 0; r < 2; r++){
        for (i=0; i<10; i++)
         {
            printf("\nEnter Integer Input (in range 0-9000): \n");
            scanf("%d", &Number);
            buffer[i] = Number;

        
        }
        
        for(int love =0;love<10;love++){
                printf(" %d ", buffer[love]);
        }
        meanANDtime(buffer, &mean, &timevalue);
        printf("\nCalculated Mean of Array: %d", mean);
        printf("\nTimestamp: %lu seconcds and %lu nanoseconds", timevalue.tv_sec, timevalue.tv_nsec);

        
    }

    return 0; 
}   

void meanANDtime(int array[], int *mean, struct timespec *timevalue) {
    float value;
    int total = 0;
    int k;

    clock_gettime(CLOCK_MONOTONIC, timevalue);

    for (k=0; k<10; k++) {
        total = total + array[k];
    } 
    *mean = (float) total/10;
    }