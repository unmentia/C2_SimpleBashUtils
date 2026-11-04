#ifndef S21_GREP_H
#define S21_GREP_H

#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int e;
  int i;
  int v;
  int c;
  int l;
  int n;
  char pattern[4096];
  int pattern_count;
} Flags;

void parse_args(int argc, char **argv, Flags *flags);
void grep_file(const char *filename, Flags *flags, regex_t *re, int multiple_files);

#endif
