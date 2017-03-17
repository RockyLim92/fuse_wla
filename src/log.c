#include "params.h"

#include <errno.h>
#include <fuse.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "log.h"

#include <time.h>


void log_print_time(){

    struct tm *t;
    time_t timer;
    timer = time(NULL);
    t = localtime(&timer);
    log_msg("%s : ",timeToString(t));
}

char* timeToString(struct tm *t) {
  static char s[20];

  sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d",
              t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
              t->tm_hour, t->tm_min, t->tm_sec
          );

  return s;
}

FILE *log_open()
{
    FILE *logfile;
    
    // very first thing, open up the logfile and mark that we got in
    // here.  If we can't open the logfile, we're dead.
    logfile = fopen("analysys.log", "w");
    if (logfile == NULL) {
	   perror("analysys file");
	   exit(EXIT_FAILURE);
    }
    
    // set logfile to line buffering
    setvbuf(logfile, NULL, _IOLBF, 0);

    return logfile;
}

void log_msg(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    vfprintf(WLA_DATA->logfile, format, ap);
}