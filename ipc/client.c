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
#define MSG_SIZE 256

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

void msleep(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}


int client_id_global;
// Pass the document string to be printed
// Sample: "Hello World\nThis is a test\n"
void print_doc(const char* doc) {
    char filename[64];
    snprintf(filename, sizeof(filename), "output_client%d.txt", client_id_global);

    FILE* fp = fopen(filename, "w");
    if (fp) {
        fprintf(fp, "%s", doc);
        fclose(fp);
    }
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <client_id>\n", argv[0]);
        return 1;
    }

    int client_id = atoi(argv[1]);
    client_id_global = client_id;
    printf("Client %d: Starting\n", client_id);

    // Write your client logic here

    return 0;
}
