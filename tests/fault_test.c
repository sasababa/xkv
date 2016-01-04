#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

unsigned test_duration = 5;
struct timespec time_start, time_end;
unsigned long faults;

void show_usage(const char *pgmname)
{
	fprintf(stderr, "Usage   : %s [ <duration (s)> ]\n", pgmname);
	fprintf(stderr, "Example : %s %d\n", pgmname, test_duration);
}

void segv_handler(int num, siginfo_t *info, void *unused)
{
	assert(num == SIGSEGV);
	assert(info->si_signo == SIGSEGV);
	assert(info->si_addr == NULL);

	faults++;
}

void alrm_handler(int num, siginfo_t *info, void *unused)
{
	int err;
	unsigned long elapsed_nanos;

	assert(num == SIGALRM);
	assert(info->si_signo == SIGALRM);

	err = clock_gettime(CLOCK_REALTIME, &time_end);
	assert(err != -1);

	elapsed_nanos = (time_end.tv_sec - time_start.tv_sec)* 1000000000UL +
			(time_end.tv_nsec - time_start.tv_nsec);

	printf("elapsed_nanos_per_op=%lu\n", elapsed_nanos / faults);

	exit(0);
}

void setup_handlers(void)
{
	int err;
	struct sigaction sa_segv;
	struct sigaction sa_alrm;

	sa_segv.sa_flags = SA_SIGINFO;
	sa_segv.sa_sigaction = segv_handler;
	sigemptyset(&sa_segv.sa_mask);

	err = sigaction(SIGSEGV, &sa_segv, NULL);
	assert(err != -1);

	sa_alrm.sa_flags = SA_SIGINFO;
	sa_alrm.sa_sigaction = alrm_handler;
	sigemptyset(&sa_alrm.sa_mask);

	err = sigaction(SIGALRM, &sa_alrm, NULL);
	assert(err != -1);

	return;
}

void __attribute__((noinline)) do_faults(void)
{
	volatile unsigned long *addr = NULL;

	*addr = 10;
}

int main(int argc, char *argv[])
{
	int err;

	if (argc > 2) {
		show_usage(argv[0]);
		exit(-1);
	}

	if (argc == 2)
		test_duration = atoi(argv[1]);

	setup_handlers();

	printf("faulting for %u seconds . . .\n", test_duration);

	err = alarm(test_duration);
	assert(err == 0);

	err = clock_gettime(CLOCK_REALTIME, &time_start);
	assert(err == 0);

	do_faults();

	/* should never reach here */

	return EXIT_FAILURE;
}

