#include "disk_scheduling.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

static void append(DiskScheduleResult *r, long long position)
{
    long long previous = r->positions[r->count - 1];
    r->movement += llabs(position - previous);
    r->positions[r->count++] = position;
}

static void sort(long long values[], size_t count)
{
    size_t i, j;
    for (i = 1; i < count; i++) {
        long long value = values[i];
        j = i;
        while (j > 0 && values[j - 1] > value) { values[j] = values[j - 1]; j--; }
        values[j] = value;
    }
}

static void append_range(DiskScheduleResult *r, const long long a[], size_t n,
                         long long head, int direction, int side)
{
    size_t i;
    if (direction == 1 && side == 0) {
        for (i = 0; i < n; i++) {
            if (a[i] >= head) append(r, a[i]);
        }
    } else if (direction == 1 && side == 1) {
        for (i = n; i > 0; i--) {
            if (a[i - 1] < head) append(r, a[i - 1]);
        }
    } else if (side == 0) {
        for (i = n; i > 0; i--) {
            if (a[i - 1] <= head) append(r, a[i - 1]);
        }
    } else {
        for (i = 0; i < n; i++) {
            if (a[i] > head) append(r, a[i]);
        }
    }
}

int disk_schedule(DiskAlgorithm algorithm, const long long requests[], size_t count,
                  long long head, long long disk_size, int direction,
                  DiskScheduleResult *result)
{
    long long sorted[DISK_MAX_REQUESTS];
    size_t i, j;
    int used[DISK_MAX_REQUESTS] = {0};
    if (!requests || !result || count == 0 || count > DISK_MAX_REQUESTS || head < 0 || head > INT_MAX) return 0;
    if (algorithm < DISK_FCFS || algorithm > DISK_CLOOK) return 0;
    if (direction != 0 && direction != 1) return 0;
    if ((algorithm == DISK_SCAN || algorithm == DISK_CSCAN) &&
        (disk_size < 1 || head >= disk_size)) return 0;
    for (i = 0; i < count; i++) {
        if (requests[i] < 0 || requests[i] > INT_MAX ||
            ((algorithm == DISK_SCAN || algorithm == DISK_CSCAN) && requests[i] >= disk_size)) return 0;
        sorted[i] = requests[i];
    }
    sort(sorted, count);
    memset(result, 0, sizeof(*result));
    result->positions[result->count++] = head;

    if (algorithm == DISK_FCFS) {
        for (i = 0; i < count; i++) append(result, requests[i]);
    } else if (algorithm == DISK_SSTF) {
        long long current = head;
        for (i = 0; i < count; i++) {
            size_t best = count;
            long long best_distance = LLONG_MAX;
            for (j = 0; j < count; j++) if (!used[j]) {
                long long distance = llabs(requests[j] - current);
                if (distance < best_distance) { best_distance = distance; best = j; }
            }
            used[best] = 1;
            append(result, requests[best]);
            current = requests[best];
        }
    } else if (algorithm == DISK_SCAN) {
        if (direction == 1) {
            append_range(result, sorted, count, head, 1, 0);
            if (result->positions[result->count - 1] != disk_size - 1) append(result, disk_size - 1);
            append_range(result, sorted, count, head, 1, 1);
        } else {
            append_range(result, sorted, count, head, 0, 0);
            if (result->positions[result->count - 1] != 0) append(result, 0);
            append_range(result, sorted, count, head, 1, 1);
        }
    } else if (algorithm == DISK_CSCAN) {
        if (direction == 1) {
            append_range(result, sorted, count, head, 1, 0);
            if (result->positions[result->count - 1] != disk_size - 1) append(result, disk_size - 1);
            append(result, 0);
            for (i = 0; i < count; i++) if (sorted[i] < head) append(result, sorted[i]);
        } else {
            append_range(result, sorted, count, head, 0, 0);
            if (result->positions[result->count - 1] != 0) append(result, 0);
            append(result, disk_size - 1);
            for (i = count; i > 0; i--) if (sorted[i - 1] > head) append(result, sorted[i - 1]);
        }
    } else if (algorithm == DISK_LOOK) {
        append_range(result, sorted, count, head, direction, 0);
        append_range(result, sorted, count, head, direction, 1);
    } else { /* C-LOOK: circular jump between the extremes of pending requests. */
        append_range(result, sorted, count, head, direction, 0);
        if (direction == 1) {
            for (i = 0; i < count; i++) if (sorted[i] < head) append(result, sorted[i]);
        } else {
            for (i = count; i > 0; i--) if (sorted[i - 1] > head) append(result, sorted[i - 1]);
        }
    }
    return 1;
}

const char *disk_algorithm_name(DiskAlgorithm algorithm)
{
    static const char *names[] = {"", "FCFS", "SSTF", "SCAN", "C-SCAN", "LOOK", "C-LOOK"};
    return algorithm >= DISK_FCFS && algorithm <= DISK_CLOOK ? names[algorithm] : "Unknown";
}