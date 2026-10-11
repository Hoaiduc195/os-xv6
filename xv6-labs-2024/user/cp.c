#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int srcfd, dstfd, n;
  struct stat srcstat, dststat;
  char buf[512];

  if(argc != 3){
    fprintf(2, "Usage: cp src dst\n");
    exit(1);
  }

  srcfd = open(argv[1], O_RDONLY);
  if(srcfd < 0){
    fprintf(2, "cp: cannot open %s\n", argv[1]);
    exit(1);
  }
  if(fstat(srcfd, &srcstat) < 0){
    fprintf(2, "cp: cannot stat %s\n", argv[1]);
    close(srcfd);
    exit(1);
  }
  if(srcstat.type != T_FILE){
    fprintf(2, "cp: source must be a regular file\n");
    close(srcfd);
    exit(1);
  }

  // Check the existing destination BEFORE O_TRUNC erases its contents.
  if(stat(argv[2], &dststat) == 0){
    if(srcstat.dev == dststat.dev && srcstat.ino == dststat.ino){
      fprintf(2, "cp: source and destination are the same file\n");
      close(srcfd);
      exit(1);
    }
    if(dststat.type != T_FILE){
      fprintf(2, "cp: destination must be a regular file\n");
      close(srcfd);
      exit(1);
    }
  }

  dstfd = open(argv[2], O_CREATE | O_WRONLY | O_TRUNC);
  if(dstfd < 0){
    fprintf(2, "cp: cannot open %s\n", argv[2]);
    close(srcfd);
    exit(1);
  }

  while((n = read(srcfd, buf, sizeof(buf))) > 0){
    // Copy exactly n bytes, including the last, possibly shorter chunk.
    if(write(dstfd, buf, n) != n){
      fprintf(2, "cp: write error on %s\n", argv[2]);
      close(srcfd);
      close(dstfd);
      exit(1);
    }
  }
  if(n < 0){
    fprintf(2, "cp: read error on %s\n", argv[1]);
    close(srcfd);
    close(dstfd);
    exit(1);
  }

  close(srcfd);
  close(dstfd);
  exit(0);
}
