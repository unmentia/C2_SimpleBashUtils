#define _GNU_SOURCE

#include "s21_grep.h"

void add_pattern(Flags *flags, const char *pat) {
  if (flags->pattern_count == 0) {
    snprintf(flags->pattern, sizeof(flags->pattern), "%s", pat);
  } else {
    strncat(flags->pattern, "|", sizeof(flags->pattern) - strlen(flags->pattern) - 1);
    strncat(flags->pattern, pat, sizeof(flags->pattern) - strlen(flags->pattern) - 1);
  }
  flags->pattern_count++;
}

void parse_args(int argc, char **argv, Flags *flags) {
  int opt;
  memset(flags, 0, sizeof(Flags));

  while ((opt = getopt(argc, argv, "e:ivcln")) != -1) {
    switch (opt) {
      case 'e':
        flags->e = 1;
        add_pattern(flags, optarg);
        break;
      case 'i':
        flags->i = 1;
        break;
      case 'v':
        flags->v = 1;
        break;
      case 'c':
        flags->c = 1;
        break;
      case 'l':
        flags->l = 1;
        break;
      case 'n':
        flags->n = 1;
        break;
      default:
        fprintf(stderr, "usage: s21_grep [-eivcln] [pattern] [file ...]\n");
        exit(1)
    }
  }

  if (flags->pattern_count == 0 && optind < argc) {
    add_pattern(flags, argv[optind++]);
  }
}

void grep_file(const char *filename, Flags *flags, regex_t *re, int multiple_files) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, "s21_grep: %s: No such file or directory\n", filename);
    return;
  }

  char *line = NULL;
  size_t cap = 0;
  ssize_t read;
  int line_num = 0;
  int match_count = 0;
  int file_matched = 0;

  while ((read = getline(&line, &cap, file)) != -1) {
    line_num++;
    int matched = regexec(re, line, 0, NULL, 0) == 0;

    if (flags->v) {
      matched = !matched;
    }

    if (matched) {
      match_count++;
      file_matched = 1;

      if (flags->c || flags->l) {
        continue;
      }

      if (multiple_files) {
        printf("%s:", filename);
      }
      if (flags->n) {
        printf("%d:", line_num);
      }
      printf("%s", line);
      if (line[read - 1] != '\n') {
        printf("\n");
      }
    }
  }

  if (flags->c) {
    if (multiple_files) {
      printf("%s:%d\n", filename, match_count);
    } else {
      printf("%d\n", match_count);
    }
  }

  if (flags->l && file_matched) {
    printf("%s\n", filename);
  }

  free(line);
  fclose(file);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "usage: s21_grep [-eivcln] [pattern] [file ...]\n");
    return 1;
  }

  Flags flags;
  parse_args(argc, argv, &flags);

  if (flags.pattern_count == 0) {
    fprintf(stderr, "s21_grep: no pattern provided\n");
    return 1;
  }

  int cflags = REG_EXTENDED;
  if (flags.i) {
    cflags |= REG_ICASE;
  }

  regex_t re;
  if (regcomp(&re, flags.pattern, cflags) != 0) {
    fprintf(stderr, "s21_grep: could not comile regex\n");
    return 1;
  }

  int files_start = optind;
  int file_count = argc - files_start;
  int multiple_files = (file_count > 1);

  for (int i = files_start; i < argc; i++) {
    grep_file(argv[i], &flags, &re, multiple_files);
  }

  regfree(&re);
  return 0;
}