#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>

#define INSTRUCTIONS_THRESHOLD 1000000

static int perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                           int cpu, int group_fd, unsigned long flags)
{
    int ret = syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
    return ret;
}

static inline uint64_t rdtsc(void)
{
    uint32_t lo, hi;
    asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

static void event_overflow_callback(int signum)
{
    uint64_t rdtsc_value = rdtsc();
    printf("PMI Handler: RDTSC value is %llu\n", rdtsc_value);
}

int main(int argc, char **argv)
{
    struct perf_event_attr pe;
    int fd;

    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE;
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_INSTRUCTIONS;
    pe.sample_period = INSTRUCTIONS_THRESHOLD;
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;

    fd = perf_event_open(&pe, 0, -1, -1, 0);
    if (fd == -1)
    {
        perror("Error in perf_event_open");
        exit(EXIT_FAILURE);
    }

    signal(SIGIO, event_overflow_callback);
    fcntl(fd, F_SETFL, O_ASYNC);
    fcntl(fd, F_SETOWN, getpid());
    ioctl(fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(fd, PERF_EVENT_IOC_REFRESH, 1);

    while (1)
    {
        // 在这里执行您要监控的代码或任务
        asm volatile("nop");
    }

    close(fd);
    return 0;
}