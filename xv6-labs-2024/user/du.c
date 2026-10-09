#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

int all_files = 0;
int summary = 0;

uint64 du(char *path)
{
  int fd;
  struct dirent de;
  struct stat st;
  char buf[256];
  char *p;

  if (stat(path, &st) < 0)
  {
    fprintf(2, "du: cannot stat %s\n", path);
    return 0;
  }

  if (st.type == T_FILE || st.type == T_DEVICE)
  {
    if (all_files && !summary)
    {
      printf("%d\t%s\n", (int)st.size, path);
    }
    return st.size;
  }

  if (st.type == T_DIR)
  {
    uint64 total = 0;

    if ((fd = open(path, O_RDONLY)) < 0)
    {
      fprintf(2, "du: cannot open %s\n", path);
      return 0;
    }

    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf))
    {
      fprintf(2, "du: path too long\n");
      close(fd);
      return 0;
    }

    strcpy(buf, path);
    p = buf + strlen(buf);
    if (p > buf && *(p - 1) != '/')
      *p++ = '/';

    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
      if (de.inum == 0)
        continue;
      if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue;

      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;

      uint64 child_size = du(buf);
      total += child_size;
    }

    close(fd);

    if (!summary)
    {
      printf("%d\t%s\n", (int)total, path);
    }

    return total;
  }

  return 0;
}

int main(int argc, char *argv[])
{
  char *path = ".";
  struct stat st;

  for (int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "-a") == 0)
    {
      all_files = 1;
    }
    else if (strcmp(argv[i], "-s") == 0)
    {
      summary = 1;
    }
    else if (argv[i][0] == '-')
    {
      fprintf(2, "du: invalid option -- %s\n", argv[i]);
      exit(1);
    }
    else
    {
      path = argv[i];
    }
  }

  if (stat(path, &st) < 0)
  {
    fprintf(2, "du: cannot stat %s\n", path);
    exit(1);
  }

  uint64 total = du(path);

  if (summary)
  {
    printf("%d\t%s\n", (int)total, path);
  }
  else if (st.type != T_DIR && !all_files)
  {
    printf("%d\t%s\n", (int)total, path);
  }

  exit(0);
}
