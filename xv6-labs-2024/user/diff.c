#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

struct line {
  char *data;
  int len;
  int capacity;
  int has_newline;
};

// Initialize *line to {0} before the first call; free data when finished.
// Returns 1 for a line, 0 for EOF without data, -1 for read/allocation errors.
int
readline(int fd, struct line *line)
{
  line->len = 0;
  line->has_newline = 0;
  if(line->capacity == 0){
    line->data = malloc(128);
    if(line->data == 0)
      return -1;
    line->capacity = 128;
  }
  line->data[0] = '\0';

  for(;;){
    char c;
    int n = read(fd, &c, 1);
    if(n < 0)
      return -1;
    if(n == 0)
      return line->len > 0 ? 1 : 0;
    if(c == '\n'){
      line->has_newline = 1;
      return 1;
    }

    // Reserve one extra byte for the terminating '\0'.
    if(line->len + 1 >= line->capacity){
      if(line->capacity >= (1 << 30))
        return -1;
      int capacity = line->capacity * 2;
      char *data = malloc(capacity);
      if(data == 0)
        return -1;
      memmove(data, line->data, line->len);
      free(line->data);
      line->data = data;
      line->capacity = capacity;
    }
    line->data[line->len++] = c;
    line->data[line->len] = '\0';
  }
}

static void
usage(void)
{
  fprintf(2, "Usage: diff file1 file2 [-q] or diff -q file1 file2\n");
  exit(2);
}

static int
printline(char *path, int number, char *marker, int present, struct line *line)
{
  printf("%s:%d: %s ", path, number, marker);
  if(!present){
    printf("EOF\n");
    return 0;
  }
  // Use len rather than %s so embedded zero bytes are not discarded.
  int written = 0;
  while(written < line->len){
    int n = write(1, line->data + written, line->len - written);
    if(n <= 0)
      return -1;
    written += n;
  }
  printf("\n");
  if(!line->has_newline)
    printf("\\ No newline at end of file\n");
  return 0;
}

int
main(int argc, char *argv[])
{
  int quiet = 0;
  char *path1 = 0, *path2 = 0;

  if(argc == 3){
    path1 = argv[1];
    path2 = argv[2];
  } else if(argc == 4 && strcmp(argv[1], "-q") == 0){
    quiet = 1;
    path1 = argv[2];
    path2 = argv[3];
  } else if(argc == 4 && strcmp(argv[3], "-q") == 0){
    quiet = 1;
    path1 = argv[1];
    path2 = argv[2];
  } else {
    usage();
  }

  // Use ./name for a file whose name starts with '-'.
  if(path1[0] == '-' || path2[0] == '-')
    usage();

  int fd1 = open(path1, O_RDONLY);
  if(fd1 < 0){
    fprintf(2, "diff: cannot open %s\n", path1);
    exit(2);
  }
  int fd2 = open(path2, O_RDONLY);
  if(fd2 < 0){
    fprintf(2, "diff: cannot open %s\n", path2);
    close(fd1);
    exit(2);
  }

  struct line line1 = {0}, line2 = {0};
  struct stat st1, st2;
  // Exit status: 0 identical, 1 different, 2 error (our lab convention).
  int status = 0;
  if(fstat(fd1, &st1) < 0 || fstat(fd2, &st2) < 0 ||
     st1.type != T_FILE || st2.type != T_FILE){
    fprintf(2, "diff: inputs must be regular files\n");
    status = 2;
    goto done;
  }

  for(int number = 1;; number++){
    int has1 = readline(fd1, &line1);
    int has2 = readline(fd2, &line2);
    if(has1 < 0 || has2 < 0){
      fprintf(2, "diff: read error or out of memory\n");
      status = 2;
      break;
    }
    if(has1 == 0 && has2 == 0)
      break;

    int different = has1 != has2 || line1.len != line2.len ||
                    line1.has_newline != line2.has_newline ||
                    memcmp(line1.data, line2.data, line1.len) != 0;
    if(!different)
      continue;

    status = 1;
    if(quiet){
      printf("diff: files differ\n");
      break;
    }
    if(printline(path1, number, "<", has1, &line1) < 0 ||
       printline(path2, number, ">", has2, &line2) < 0){
      fprintf(2, "diff: write error\n");
      status = 2;
      break;
    }
  }

done:
  if(line1.data)
    free(line1.data);
  if(line2.data)
    free(line2.data);
  close(fd1);
  close(fd2);
  exit(status);
}
