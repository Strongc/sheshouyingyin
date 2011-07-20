/* timing.c - functions to benchmark hash algorithms,
 * written by Alexei Kravchenko.
 *
 * Copyleft:
 * I, the author, hereby release this code into the public domain.
 * This applies worldwide. I grant any entity the right to use this work for
 * ANY PURPOSE, without any conditions, unless such conditions are required
 * by law.
 */

/* #include <unistd.h> */
#include "byte_order.h"
#include "rhash.h"
#include "timing.h"

/* DEFINE read_tsc() if possible */

#if (defined(CPU_IA32)) || defined(CPU_X64)

#if defined( _MSC_VER ) /* if MS VC */
# include <intrin.h>
# pragma intrinsic( __rdtsc )
# define read_tsc() __rdtsc()
# define HAVE_TSC

/* the MSVC assembler implementation
static inline volatile uint64_t read_tsc(void) {
  unsigned long lo, hi;
  __asm cpuid __asm rdtsc __asm mov [lo],eax __asm mov [hi],edx ; 
  return (((uint64_t)hi) << 32) + lo;
} */
#elif defined( __GNUC__ )
/*static inline volatile uint64_t read_tsc(void) {*/
static uint64_t read_tsc(void) {
    unsigned long lo, hi;
    __asm volatile("rdtsc" : "=a" (lo), "=d" (hi));
    /*__asm volatile("cpuid; rdtsc" : "=a" (lo), "=d" (hi) : "ebx", "ecx"); */
    return (((uint64_t)hi) << 32) + lo;
}
# define HAVE_TSC
#endif /* _MSC_VER, __GNUC__ */
#endif /* CPU_IA32, CPU_X64 */


/* TIMER FUNCTIONS */

#ifdef _WIN32
#include <windows.h>
#define get_timedelta(delta) QueryPerformanceCounter((LARGE_INTEGER*)delta)
#else
#define get_timedelta(delta) gettimeofday(delta, NULL)
#endif

/**
 * Return real-value representing number of seconds
 * stored in the given timeval structure.
 * The function is used with timers, when printing time statistics.
 *
 * @param delta time delta to be converted
 * @return number of seconds
 */
static double fsec(timedelta_t* timer)
{
#ifdef _WIN32
  LARGE_INTEGER freq;
  QueryPerformanceFrequency(&freq);
  return (double)*timer/freq.QuadPart;
#else
  return ((double)timer->tv_usec / 1000000.0) + timer->tv_sec;
#endif
}

/**
 * Start a timer.
 *
 * @param delta timer to start
 */
void timer_start(timedelta_t* timer)
{
  get_timedelta(timer);
}

/**
 * Stop given timer.
 *
 * @param timer the timer to stop
 * @return number of seconds timed
 */
double timer_stop(timedelta_t* timer)
{
  timedelta_t end;
  get_timedelta(&end);
#ifdef _WIN32
  *timer = end - *timer;
#else
  timer->tv_sec  = end.tv_sec  - timer->tv_sec - (end.tv_usec >= timer->tv_usec ? 0 : 1);
  timer->tv_usec = end.tv_usec + (end.tv_usec >= timer->tv_usec ? 0 : 1000000 ) - timer->tv_usec;
#endif
  return fsec(timer);
}

#ifdef _WIN32
/**
 * Set process priority and affinity to use all cpu's but the first one.
 * This improves benchmark results on a multi-cpu systems.
 */
static void benchmark_cpu_init(void)
{
  DWORD_PTR dwProcessMask, dwSysMask, dwDesired;

  SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

  if( GetProcessAffinityMask(GetCurrentProcess(), &dwProcessMask, &dwSysMask) ) {
    dwDesired = dwSysMask & (dwProcessMask & ~1); /* remove the first processor */
    dwDesired = (dwDesired ? dwDesired : dwSysMask & ~1);
    if(dwDesired != 0) {
      SetProcessAffinityMask(GetCurrentProcess(), dwDesired);
    }
  }
}
#endif

/* #define BENCHMARK_HASH_SIMULTANIOUSLY - should support in future */

/**
 * Hash a repeated message chunk by specified hash function.
 *
 * @param hash_id hash function identifier
 * @param message a message chunk to hash
 * @param msg_size message chunk size
 * @param count number of chunks
 * @param out computed hash
 * @return 1 on success, 0 on error
 */
static int hash_in_loop(unsigned hash_id, const unsigned char* message, size_t msg_size, int count, unsigned char* out)
{
  int i;
  struct rhash_context *context = rhash_init(hash_id);
  if(!context) return 0;

  /* process the repeated message buffer */
  for(i = 0; i < count; i++) rhash_update(context, message, msg_size);
  rhash_final(context, hash_id, out);
  rhash_free(context);
  return 1;
}

/**
 * Benchmark a hash algorithm.
 *
 * @param hash_id hash algorithm identifier
 * @param flags benchmark flags, can be BENCHMARK_QUIET and BENCHMARK_CPB
 * @param output the stream to print results
 */
void run_benchmark(unsigned hash_id, unsigned flags, FILE* output)
{
  unsigned char ALIGN_ATTR(16) message[8192]; /* 8Kb */
  timedelta_t timer;
  int i, j;
  /*int msg_size = 1073741824; unsigned sz_mb;*/
  size_t sz_mb, msg_size;
  double time, total_time = 0, cpb = 0;
  const int rounds = 4;
  const char* hash_name;
  unsigned char out[130];

#ifdef _WIN32
  benchmark_cpu_init(); /* set cpu affinity to improve test results */
#endif

  /* set message size for fast and slow hash functions */
  msg_size = 1073741824 / 2;
  if(hash_id & (RHASH_WHIRLPOOL | RHASH_SNEFRU128 | RHASH_SNEFRU256)) {
    msg_size /= 8;
  } else if(hash_id & (RHASH_GOST | RHASH_GOST_CRYPTOPRO | RHASH_SHA384 | RHASH_SHA512)) {
    msg_size /= 2;
  }
  sz_mb = msg_size / (1 << 20); /* size in MiB */
  hash_name = rhash_get_name(hash_id);
  if(!hash_name) hash_name = ""; /* benchmarking several hashes*/

  /*fprintf(output, " benchmark:");
  for(i = 1; i & RHASH_ALL_HASHES; i <<= 1) {
    if(i & opt.sum_flags) printf(" %s", rhash_get_name(i));
  }
  fprintf(output, "\n");
  fflush(output);*/

  for(i = 0; i < (int)sizeof(message); i++) message[i] = i & 0xff;

  for(j = 0; j < rounds; j++) {
    timer_start(&timer);
    hash_in_loop(hash_id, message, sizeof(message), (int)(msg_size / sizeof(message)), out);

    time = timer_stop(&timer);
    total_time += time;

    if((flags & (BENCHMARK_QUIET|BENCHMARK_RAW)) == 0) {
      fprintf(output, "%s %u MiB calculated in %.3f sec, %.3f MBps\n", hash_name, (unsigned)sz_mb, time, (double)sz_mb / time);
      fflush(output);
    }
  }

#if defined(HAVE_TSC)
  /* measure the CPU "clocks per byte" speed */
  if(flags & BENCHMARK_CPB) {
    unsigned int c1 = -1, c2 = -1;
    unsigned volatile long long cy0, cy1, cy2;
    int msg_size = 128*1024;

    /* make 200 tries */
    for(i = 0; i < 200; i++) {
        cy0 = read_tsc();
        hash_in_loop(hash_id, message, sizeof(message), msg_size/sizeof(message), out);
        cy1 = read_tsc();
        hash_in_loop(hash_id, message, sizeof(message), msg_size/sizeof(message), out);
        hash_in_loop(hash_id, message, sizeof(message), msg_size/sizeof(message), out);
        cy2 = read_tsc();

        cy2 -= cy1;
        cy1 -= cy0;
        c1 = (unsigned int)(c1 > cy1 ? cy1 : c1);
        c2 = (unsigned int)(c2 > cy2 ? cy2 : c2);
    }
    cpb = ((c2 - c1) + 1) / (double)msg_size;
    /*printf("%8.2f", ((c2 - c1) + 1) / (double)msg_size);*/
  }
#endif /* HAVE_TSC */

  if(flags & BENCHMARK_RAW) {
    /* output result in a "raw" machine-readable format */
    fprintf(output, "%s\t%u\t%.3f\t%.3f", hash_name, ((unsigned)sz_mb * rounds), total_time, (double)(sz_mb * rounds) / total_time);
#if defined(HAVE_TSC)
    if(flags & BENCHMARK_CPB) fprintf(output, "\t%.2f", cpb);
#endif /* HAVE_TSC */
    fprintf(output, "\n");
  } else {
    fprintf(output, "%s %u MiB total in %.3f sec, %.3f MBps", hash_name, ((unsigned)sz_mb * rounds), total_time, (double)(sz_mb * rounds) / total_time);
#if defined(HAVE_TSC)
    if(flags & BENCHMARK_CPB) fprintf(output, ", CPB=%.2f", cpb);
#endif /* HAVE_TSC */
    fprintf(output, "\n");
  }
}
