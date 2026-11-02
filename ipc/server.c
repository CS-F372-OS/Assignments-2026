#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>
#include <sys/time.h>

#define GRID_SIZE 10
#define MAX_STRING_LEN 64
#define MAX_CLIENTS 10

// Timestamped printf that prefixes each line with HH:MM:SS.mmm
static void ts_printf(const char *fmt, ...) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm tm_info;
    localtime_r(&tv.tv_sec, &tm_info);
    char tbuf[32];
    strftime(tbuf, sizeof(tbuf), "%H:%M:%S", &tm_info);

    fprintf(stdout, "%s.%03ld ", tbuf, tv.tv_usec / 1000);

    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);

    fflush(stdout);
}

#define printf(...) ts_printf(__VA_ARGS__)

int main() {
    printf("Server: Started\n");


    // Write your server logic here

    printf("Server: Shutdown complete\n");
    return 0;
}
