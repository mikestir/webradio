#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#define LOG_DEBUG(a,...)		fprintf(stderr, "%s(%d): " a, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_INFO(a,...)			fprintf(stderr, "%s(%d): " a, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_ERROR(a,...)		fprintf(stderr, "%s(%d): " a, __FILE__, __LINE__, ##__VA_ARGS__)

#endif
