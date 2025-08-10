#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define MAX_SLAB_ENTRIES 512
#define MAX_LINE_LENGTH 256

// Structure to hold information about a single slab cache
typedef struct {
    char name[64];
    long active_objs;
    long num_objs;
    long obj_size;
    long obj_per_slab;
    long pages_per_slab;
} SlabInfo;

// Structure to store two snapshots of slab data for growth calculation
typedef struct {
    SlabInfo current[MAX_SLAB_ENTRIES];
    SlabInfo previous[MAX_SLAB_ENTRIES];
    int count_current;
    int count_previous;
    double growth[MAX_SLAB_ENTRIES];
} SlabData;

// Function to read and parse /proc/slabinfo
int parse_slabinfo(SlabInfo *slab_entries) {
    FILE *fp = fopen("/proc/slabinfo", "r");
    if (fp == NULL) {
        perror("Error opening /proc/slabinfo");
        return 0;
    }

    char line[MAX_LINE_LENGTH];
    int count = 0;

    // Skip the header lines
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp) != NULL && count < MAX_SLAB_ENTRIES) {
        sscanf(line, "%63s %ld %ld %ld %ld %ld",
               slab_entries[count].name,
               &slab_entries[count].active_objs,
               &slab_entries[count].num_objs,
               &slab_entries[count].obj_size,
               &slab_entries[count].obj_per_slab,
               &slab_entries[count].pages_per_slab);
        count++;
    }

    fclose(fp);
    return count;
}

// Function to calculate the growth rate of slab caches
void calculate_growth(SlabData *data, int interval) {
    for (int i = 0; i < data->count_current; i++) {
        data->growth[i] = 0; // Default to 0 growth
        for (int j = 0; j < data->count_previous; j++) {
            if (strcmp(data->current[i].name, data->previous[j].name) == 0) {
                long active_growth = data->current[i].active_objs - data->previous[j].active_objs;
                if (interval > 0) {
                    data->growth[i] = (double)active_growth / interval;
                }
                break;
            }
        }
    }
}

// Function to print the top N growing slab caches
void print_top_n(const SlabData *data, int n, long threshold) {
    int sorted_indices[MAX_SLAB_ENTRIES];
    for (int i = 0; i < data->count_current; i++) {
        sorted_indices[i] = i;
    }

    // A simple bubble sort for demonstration.
    for (int i = 0; i < data->count_current - 1; i++) {
        for (int j = 0; j < data->count_current - i - 1; j++) {
            if (data->growth[sorted_indices[j]] < data->growth[sorted_indices[j + 1]]) {
                int temp = sorted_indices[j];
                sorted_indices[j] = sorted_indices[j + 1];
                sorted_indices[j + 1] = temp;
            }
        }
    }

    // Clear the screen
    system("clear");

    printf("--- Top %d Growing Slab Caches (updated every second) ---\n", n);
    printf("%-24s %12s %12s %12s %12s\n", "Cache Name", "Active Objs", "Total Objs", "Obj Size", "Growth/s");
    printf("--------------------------------------------------------------------------------\n");

    for (int i = 0; i < n && i < data->count_current; i++) {
        int index = sorted_indices[i];
        printf("%-24s %12ld %12ld %12ld %12.2f",
               data->current[index].name,
               data->current[index].active_objs,
               data->current[index].num_objs,
               data->current[index].obj_size,
               data->growth[index]);

        if (data->current[index].active_objs > threshold) {
            printf("  <-- ALERT: THRESHOLD EXCEEDED!");
        }
        printf("\n");
    }
    fflush(stdout);
}

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s [-i interval] [-n top_n] [-t threshold]\n", prog_name);
    fprintf(stderr, "  -i: monitoring interval in seconds (default: 1)\n");
    fprintf(stderr, "  -n: number of top slab caches to display (default: 10)\n");
    fprintf(stderr, "  -t: alert threshold for active objects (default: 100000)\n");
}

int main(int argc, char *argv[]) {
    int interval = 1;
    int top_n = 10;
    long threshold = 100000;
    int opt;

    while ((opt = getopt(argc, argv, "i:n:t:h")) != -1) {
        switch (opt) {
            case 'i':
                interval = atoi(optarg);
                break;
            case 'n':
                top_n = atoi(optarg);
                break;
            case 't':
                threshold = atol(optarg);
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    SlabData slab_data = {0};

    // Initial data read
    slab_data.count_previous = parse_slabinfo(slab_data.previous);

    while (1) {
        sleep(interval);

        slab_data.count_current = parse_slabinfo(slab_data.current);
        if (slab_data.count_current == 0) {
            continue;
        }

        calculate_growth(&slab_data, interval);
        print_top_n(&slab_data, top_n, threshold);

        // Current data becomes previous for the next iteration
        memcpy(slab_data.previous, slab_data.current, sizeof(SlabInfo) * slab_data.count_current);
        slab_data.count_previous = slab_data.count_current;
    }

    return 0;
}
