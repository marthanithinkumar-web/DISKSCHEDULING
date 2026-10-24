#include "disk_scheduling.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static int read_number(const char *prompt, long long *value)
{
    char line[128], *end;

    printf("%s", prompt);

    if (fgets(line, sizeof(line), stdin) == NULL)
        return 0;

    errno = 0;
    *value = strtoll(line, &end, 10);

    if (end == line || errno == ERANGE)
        return -1;

    while (*end == ' ' || *end == '\t' ||
           *end == '\r' || *end == '\n')
        end++;

    return (*end == '\0') ? 1 : -1;
}

static void run_algorithm(DiskAlgorithm algorithm)
{
    long long n_value, head, disk_size = 0, direction = 1;
    long long requests[DISK_MAX_REQUESTS];
    DiskScheduleResult result;
    size_t n, i;
    int status;
    char prompt[100];

    status = read_number("Number of requests (1-100): ", &n_value);

    if (status <= 0 || n_value < 1 ||
        n_value > DISK_MAX_REQUESTS) {
        puts("Invalid number of requests.");
        return;
    }

    n = (size_t)n_value;

    if (algorithm == DISK_SCAN || algorithm == DISK_CSCAN) {
        status = read_number("Disk size (number of cylinders): ",
                             &disk_size);

        if (status <= 0 || disk_size < 1 ||
            disk_size > INT_MAX) {
            puts("Invalid disk size.");
            return;
        }
    }

    if (algorithm == DISK_SCAN || algorithm == DISK_CSCAN)
        snprintf(prompt, sizeof(prompt),
                 "Initial head position (0 to %lld): ",
                 disk_size - 1);
    else
        snprintf(prompt, sizeof(prompt),
                 "Initial head position: ");

    status = read_number(prompt, &head);

    if (status <= 0 || head < 0 || head > INT_MAX ||
        ((algorithm == DISK_SCAN || algorithm == DISK_CSCAN) &&
         head >= disk_size)) {
        puts("Invalid head position.");
        return;
    }

    for (i = 0; i < n; i++) {
        snprintf(prompt, sizeof(prompt),
                 "Request %zu position: ", i + 1);

        status = read_number(prompt, &requests[i]);

        if (status <= 0 || requests[i] < 0 ||
            requests[i] > INT_MAX ||
            ((algorithm == DISK_SCAN || algorithm == DISK_CSCAN) &&
             requests[i] >= disk_size)) {
            puts("Invalid request position.");
            return;
        }
    }

    if (algorithm == DISK_SCAN || algorithm == DISK_CSCAN ||
        algorithm == DISK_LOOK || algorithm == DISK_CLOOK) {
        status = read_number(
            "Direction (0 = left, 1 = right): ", &direction);

        if (status <= 0 ||
            (direction != 0 && direction != 1)) {
            puts("Invalid direction.");
            return;
        }
    }

    if (!disk_schedule(algorithm, requests, n, head,
                       disk_size, (int)direction, &result)) {
        puts("Could not calculate schedule.");
        return;
    }

    printf("\nService order: %lld", result.positions[0]);

    for (i = 1; i < result.count; i++)
        printf(" -> %lld", result.positions[i]);

    printf("\nTotal head movement: %lld cylinders\n",
           result.movement);
}

int main(void)
{
    long long choice;

    for (;;) {
        puts("\n====================================");
        puts("      DISK SCHEDULING SIMULATOR");
        puts("====================================");
        puts("1. FCFS");
        puts("2. SSTF");
        puts("3. SCAN");
        puts("4. C-SCAN");
        puts("5. LOOK");
        puts("6. C-LOOK");
        puts("0. Exit");

        int status = read_number("Enter your choice: ", &choice);

        if (status == 0)
            break;

        if (status < 0) {
            puts("Invalid input.");
            continue;
        }

        if (choice == 0) {
            puts("Exiting simulator. Goodbye!");
            break;
        }

        if (choice < 1 || choice > 6) {
            puts("Invalid choice. Select 0-6.");
            continue;
        }

        run_algorithm((DiskAlgorithm)choice);
    }

    return 0;
}