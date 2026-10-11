#define main diff_main
#include "diff.c"
#undef main

void check(int ok, char *name) {
  if(!ok){ printf("FAIL: %s\n", name); exit(1); }
  printf("PASS: %s\n", name);
}
int fixture(char *data, int len) {
  int fd=open("lines", O_CREATE|O_WRONLY|O_TRUNC);
  if(fd<0 || write(fd,data,len)!=len) exit(2);
  close(fd);
  return open("lines",O_RDONLY);
}
int main(void) {
  struct line l={0};
  int fd=fixture("\nABC\n",5);
  check(readline(fd,&l)==1 && l.len==0 && l.has_newline==1,"empty line is not EOF");
  check(readline(fd,&l)==1 && l.len==3 && memcmp(l.data,"ABC",3)==0 && l.has_newline==1,"line with newline");
  check(readline(fd,&l)==0,"EOF after trailing newline");
  close(fd);
  fd=fixture("ABC",3);
  check(readline(fd,&l)==1 && l.len==3 && memcmp(l.data,"ABC",3)==0 && l.has_newline==0,"last line without newline");
  check(readline(fd,&l)==0,"EOF after last line"); close(fd);
  fd=fixture("",0);
  check(readline(fd,&l)==0,"empty file"); close(fd);
  char data[1201];
  for(int i=0;i<1200;i++) data[i]='a'+i%26;
  data[1200]='\n'; fd=fixture(data,1201);
  check(readline(fd,&l)==1 && l.len==1200 && l.has_newline && memcmp(l.data,data,1200)==0 && l.data[1200]==0,"grow buffer for 1200-byte line");
  close(fd);
  char binary[4]={'A',0,'B','\n'}; fd=fixture(binary,4);
  check(readline(fd,&l)==1 && l.len==3 && memcmp(l.data,binary,3)==0,"preserve embedded zero byte"); close(fd);
  check(readline(-1,&l)==-1,"read error");
  free(l.data);
  printf("ALL LINE TESTS PASSED\n"); exit(0);
}
