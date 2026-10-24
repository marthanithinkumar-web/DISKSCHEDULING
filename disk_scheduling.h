#ifndef DISK_SCHEDULING_H
#define DISK_SCHEDULING_H

#include <stddef.h>

#define DISK_MAX_REQUESTS 100

typedef enum {
    DISK_FCFS = 1,
    DISK_SSTF,
    DISK_SCAN,
    DISK_CSCAN,
    DISK_LOOK,
    DISK_CLOOK
} DiskAlgorithm;

typedef struct {
    /* Initial position, every request, and up to two physical endpoints. */
    long long positions[DISK_MAX_REQUESTS + 3];
    size_t count;
    long long movement;
} DiskScheduleResult;

/* direction: 0 = toward lower-numbered cylinders; 1 = toward higher. */
int disk_schedule(DiskAlgorithm algorithm, const long long requests[], size_t count,
                  long long head, long long disk_size, int direction,
                  DiskScheduleResult *result);
const char *disk_algorithm_name(DiskAlgorithm algorithm);

#endif