#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

void check(int ok, char *name) {
  if(!ok){ printf("FAIL: %s\n", name); exit(1); }
  printf("PASS: %s\n", name);
}
void fixture(char *path, int size) {
  int fd = open(path, O_CREATE|O_WRONLY|O_TRUNC);
  if(fd < 0) exit(2);
  char buf[512];
  for(int off=0; off<size;){
    int n=size-off;
    if(n>sizeof(buf)) n=sizeof(buf);
    for(int j=0;j<n;j++) buf[j]=(off+j)%251;
    if(write(fd,buf,n)!=n) exit(2);
    off+=n;
  }
  close(fd);
}
int valid(char *path, int size) {
  int fd = open(path, O_RDONLY);
  struct stat st;
  if(fd < 0) return 0;
  if(fstat(fd,&st)<0 || st.size != size){ close(fd); return 0; }
  for(int i=0; i<size; i++){
    uchar b;
    if(read(fd,&b,1)!=1 || b != i%251){ close(fd); return 0; }
  }
  char b;
  int eof = read(fd,&b,1);
  close(fd);
  return eof == 0;
}
int run(char **args, int freezero) {
  int pid=fork(), status;
  if(pid<0) exit(2);
  if(pid==0){
    if(freezero) close(0);
    exec("cp",args);
    exit(99);
  }
  if(wait(&status)!=pid) exit(2);
  return status;
}
int main(void) {
  char *copy[]={"cp","src","dst",0};
  int sizes[]={0,7,512,1024,1200,4097};
  for(int i=0;i<6;i++){
    fixture("src",sizes[i]); unlink("dst");
    check(run(copy,0)==0 && valid("dst",sizes[i]) && valid("src",sizes[i]), "copy size/content");
    printf("size=%d\n", sizes[i]);
  }
  fixture("src",7); fixture("dst",1200);
  check(run(copy,0)==0 && valid("dst",7),"truncate longer destination");
  fixture("src",0);
  check(run(copy,0)==0 && valid("dst",0),"empty source truncates destination");
  fixture("src",1200);
  check(run(copy,1)==0 && valid("dst",1200),"descriptor zero accepted");
  char *missing[]={"cp","src",0};
  check(run(missing,0)==1,"missing argument");
  char *extra[]={"cp","src","dst","extra",0};
  check(run(extra,0)==1,"extra argument");
  char *absent[]={"cp","absent","dst",0};
  check(run(absent,0)==1 && valid("dst",1200),"missing source preserves destination");
  char *same[]={"cp","src","src",0};
  check(run(same,0)==1 && valid("src",1200),"same path preserves source");
  unlink("alias"); check(link("src","alias")==0,"create hard link");
  char *alias[]={"cp","src","alias",0};
  check(run(alias,0)==1 && valid("src",1200) && valid("alias",1200),"hard link preserves source");
  mkdir("adir");
  char *dirsrc[]={"cp","adir","dst",0};
  check(run(dirsrc,0)==1 && valid("dst",1200),"reject source directory");
  char *dirdst[]={"cp","src","adir",0};
  check(run(dirdst,0)==1 && valid("src",1200),"reject destination directory");
  char *badpath[]={"cp","src","missing/out",0};
  check(run(badpath,0)==1 && valid("src",1200),"destination open failure");
  printf("ALL CP TESTS PASSED\n");
  exit(0);
}
