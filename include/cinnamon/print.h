/// Copyright (C) strawberryhacker 

#ifndef PRINT_H
#define PRINT_H

#include <cinnamon/types.h>
#include <stdarg.h>

#define RED     "\033[31m"
#define NORMAL  "\033[0m"
#define GREEN   "\033[32m"
#define PINK    "\033[35m"
#define BLUE    "\033[34m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"

void print(const char* data, ...);

#endif