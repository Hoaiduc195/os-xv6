#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
void put(char *path, char *data, int len){
  int fd=open(path,O_CREATE|O_WRONLY|O_TRUNC);
  if(fd<0 || write(fd,data,len)!=len) exit(2);
  close(fd);
}
void check(char *name,char **args,int expected,char *output,int len){
  int pid=fork(),status;
  if(pid<0) exit(2);
  if(pid==0){
    close(1);
    if(open("result",O_CREATE|O_WRONLY|O_TRUNC)!=1) exit(99);
    close(2); dup(1);
    exec("diff",args); exit(99);
  }
  if(wait(&status)!=pid) exit(2);
  int fd=open("result",O_RDONLY),ok=status==expected;
  for(int i=0;i<len;i++){
    char c;
    if(read(fd,&c,1)!=1 || c!=output[i]) ok=0;
  }
  char c;
  if(read(fd,&c,1)!=0) ok=0;
  close(fd);
  if(!ok){printf("FAIL: %s status=%d\n",name,status);exit(1);}
  printf("PASS: %s\n",name);
}
void test(char *name,char **args,int status,char *output){
  check(name,args,status,output,strlen(output));
}
int main(void){
  char *normal[]={"diff","a","b",0};
  char *qfirst[]={"diff","-q","a","b",0};
  char *qlast[]={"diff","a","b","-q",0};
  put("a","",0);put("b","",0);
  test("both empty",normal,0,"");
  put("a","apple\nbanana\ncarrot\n",20);put("b","apple\nblueberry\ncarrot\ndate\n",28);
  test("sample changed line and EOF",normal,1,"a:2: < banana\nb:2: > blueberry\na:4: < EOF\nb:4: > date\n");
  test("quiet first once",qfirst,1,"diff: files differ\n");
  test("quiet last once",qlast,1,"diff: files differ\n");
  put("a","\nABC\n",5);put("b","\nABC\n",5);
  test("identical with empty line",normal,0,"");test("quiet identical",qfirst,0,"");
  put("a","ABC",3);put("b","ABC",3);
  test("identical no final newline",normal,0,"");
  put("b","ABC\n",4);
  test("newline difference",normal,1,"a:1: < ABC\n\\ No newline at end of file\nb:1: > ABC\n");
  put("a","",0);put("b","\n",1);
  test("empty versus blank line",normal,1,"a:1: < EOF\nb:1: > \n");
  put("a","x\ny\n",4);put("b","",0);
  test("second file ends first",normal,1,"a:1: < x\nb:1: > EOF\na:2: < y\nb:2: > EOF\n");
  char big[1201]; for(int i=0;i<1200;i++)big[i]='a';big[1200]='\n';
  put("a",big,1201);put("b",big,1201);test("long equal",normal,0,"");
  big[1199]='b';put("b",big,1201);test("long differs beyond buffer",qlast,1,"diff: files differ\n");
  char az[4]={'A',0,'B','\n'},bz[4]={'A',0,'C','\n'};
  put("a",az,4);put("b",az,4);test("binary zero equal",normal,0,"");
  put("b",bz,4);char expected[]={'a',':','1',':',' ','<',' ','A',0,'B','\n','b',':','1',':',' ','>',' ','A',0,'C','\n'};
  check("binary zero differing output",normal,1,expected,sizeof(expected));
  char *same[]={"diff","a","a",0};test("same file",same,0,"");
  unlink("b");test("second open fails",normal,2,"diff: cannot open b\n");
  unlink("a");test("first open fails",normal,2,"diff: cannot open a\n");
  mkdir("a");put("b","",0);test("reject directory",normal,2,"diff: inputs must be regular files\n");
  char *missing[]={"diff","a",0},*invalid[]={"diff","-x","a","b",0};
  char *usage="Usage: diff file1 file2 [-q] or diff -q file1 file2\n";
  test("missing argument",missing,2,usage);test("invalid option",invalid,2,usage);
  printf("ALL DIFF TESTS PASSED\n");exit(0);
}
