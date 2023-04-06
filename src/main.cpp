//
// Created by victoryang00 on 4/4/23.
//
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <sys/time.h>
#include <asm/unistd.h>

#define PMI_VECTOR 0xF0 // 假设你使用了中断向量0xF0

unsigned long long last_tsc = 0;
unsigned long long instruction_count = 0;
unsigned long long pid_count = 0;

static void pmi_handler(int sig, siginfo_t *info, void *context) {
  unsigned long long current_tsc;
  __asm__ volatile("rdtsc" : "=A" (current_tsc));
  instruction_count += current_tsc - last_tsc;
  last_tsc = current_tsc;
  
  if (instruction_count >= 1000000) {
    instruction_count -= 1000000;
    pid_count++;
    printf("PID %d instruction count: %llu\n", getpid(), pid_count);
  }
}

int main(int argc, char *argv[]) {
  struct sigaction sa;

  // 设置PMI handler
  sa.sa_sigaction = pmi_handler;
  sa.sa_flags = SA_SIGINFO;
  sigemptyset(&sa.sa_mask);
  sigaction(PMI_VECTOR, &sa, NULL);

  // 设置性能计数器
  unsigned long long event = (PERF_COUNT_HW_INSTRUCTIONS << 0) |
                             (PERF_COUNT_HW_CPU_CYCLES << 8);
  unsigned long long umask = 0x00;
  unsigned long long config = event | (umask << 8);
  unsigned long long period = 1000000; // 1 million instructions
  int fd = syscall(__NR_perf_event_open, &config, 0, -1, -1, 0);
  ioctl(fd, PERF_EVENT_IOC_PERIOD, &period);

  // 让程序执行一些指令
  for (int i = 0; i < 10000000000; i++) {
    // 执行一些指令
    __asm__ volatile("nop");
  }

  return 0;
}