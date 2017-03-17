#ifndef _LOG_H_
#define _LOG_H_
#include <stdio.h>


char* timeToString(struct tm *t);
FILE *log_open(void);
void log_msg(const char *format, ...);

/***********************************************************************************
readCount and writeCount do not mean just the count of read ans write function call
becasue system request one big file operation, fuse do it by deviding several chunk
the deviding byte size is following

read = 131072byte (32* 4096byte)

wrtie = 4096byte

2016, Rocky, Ajou University, lrocky1229@gmail.com
************************************************************************************/
static unsigned long readCount=0;
static unsigned long writeCount=0;


static unsigned long long readByte=0;
static unsigned long long writeByte=0;



#endif