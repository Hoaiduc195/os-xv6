#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

// All fixtures and output live in the runner's disposable disk image.
static char output[3072];

static void
check(int ok, char *name)
{
  if(!ok){
    printf("FAIL: %s\n", name);
    exit(1);
  }
  printf("PASS: %s\n", name);
}

static int
run(char **args)
{
  int pid = fork(), status;
  if(pid < 0) exit(2);
  if(pid == 0){
    close(1);
    if(open("core-output", O_CREATE | O_WRONLY | O_TRUNC) != 1)
      exit(99);
    close(2);
    if(dup(1) != 2) exit(99);
    exec(args[0], args);
    exit(99);
  }
  if(wait(&status) != pid) exit(2);
  int fd = open("core-output", O_RDONLY), used = 0, n;
  if(fd < 0) exit(2);
  while((n = read(fd, output + used, sizeof(output) - 1 - used)) > 0){
    used += n;
    if(used == sizeof(output) - 1) exit(2);
  }
  close(fd);
  if(n < 0) exit(2);
  output[used] = 0;
  return status;
}

static void
expect(char *name, char **args, int status, char *text)
{
  int actual = run(args);
  check(actual == status && strcmp(output, text) == 0, name);
}

static void
put(char *path, char *data)
{
  int fd = open(path, O_CREATE | O_WRONLY | O_TRUNC);
  int len = strlen(data);
  if(fd < 0 || write(fd, data, len) != len) exit(2);
  close(fd);
}

static void
test_sleep(void)
{
  char *valid[] = {"sleep", "5", 0};
  int start = uptime(), status = run(valid);
  int elapsed = uptime() - start;
  check(status == 0 && output[0] == 0 && elapsed >= 5,
        "sleep: waits at least five ticks");
  printf("MEASURE: sleep 5 elapsed=%d ticks\n", elapsed);
  char *zero[] = {"sleep", "0", 0};
  expect("sleep: zero ticks", zero, 0, "");
  char *missing[] = {"sleep", 0};
  expect("sleep: missing argument", missing, 1, "Usage: sleep ticks\n");
  char *letters[] = {"sleep", "abc", 0};
  expect("sleep: nonnumeric argument", letters, 1, "sleep: invalid ticks 'abc'\n");
  char *negative[] = {"sleep", "-1", 0};
  expect("sleep: negative argument", negative, 1, "sleep: invalid ticks '-1'\n");
  char *extra[] = {"sleep", "1", "2", 0};
  expect("sleep: extra argument", extra, 1, "Usage: sleep ticks\n");
  printf("ALL SLEEP TESTS PASSED\n");
}

static int
ping_output(void)
{
  char *p = output;
  int child = atoi(p);
  if(*p < '0' || *p > '9') return 0;
  while(*p >= '0' && *p <= '9') p++;
  char *ping = ": received ping\n";
  if(memcmp(p, ping, strlen(ping)) != 0) return 0;
  p += strlen(ping);
  int parent = atoi(p);
  if(*p < '0' || *p > '9') return 0;
  while(*p >= '0' && *p <= '9') p++;
  return child > 0 && parent > 0 && child != parent &&
         strcmp(p, ": received pong\n") == 0;
}

static void
test_pingpong(void)
{
  char *args[] = {"pingpong", 0};
  for(int i = 0; i < 3; i++){
    int status = run(args);
    check(status == 0 && ping_output(), "pingpong: ordered messages and distinct PIDs");
  }
  printf("ALL PINGPONG TESTS PASSED\n");
}

static int
prime_output(void)
{
  char *p = output;
  for(int candidate = 2; candidate <= 280; candidate++){
    int prime = 1;
    for(int d = 2; d * d <= candidate; d++)
      if(candidate % d == 0) prime = 0;
    if(!prime) continue;
    if(memcmp(p, "prime ", 6) != 0) return 0;
    p += 6;
    if(atoi(p) != candidate) return 0;
    while(*p >= '0' && *p <= '9') p++;
    if(*p++ != '\n') return 0;
  }
  return *p == 0;
}

static void
test_primes(void)
{
  char *args[] = {"primes", 0};
  for(int i = 0; i < 2; i++){
    int status = run(args);
    check(status == 0 && prime_output(), "primes: complete ordered sequence from 2 to 280");
  }
  printf("ALL PRIMES TESTS PASSED\n");
}

static void
fixtures(void)
{
  if(mkdir("tcase") < 0 || mkdir("tcase/sub") < 0 ||
     mkdir("tcase/sub/deep") < 0 || mkdir("tcase/empty") < 0)
    exit(2);
  // Creation order is deterministic in this fresh xv6 directory.
  put("tcase/a", "abc");
  put("tcase/sub/b", "12345");
  put("tcase/sub/deep/c", "xy");
}

static void
test_tree(void)
{
  char *normal[] = {"tree", "tcase", 0};
  expect("tree: full hierarchy", normal, 0,
         "tcase\n|__ sub\n   |__ deep\n      |__ c\n   |__ b\n|__ empty\n|__ a\n");
  char *dirs[] = {"tree", "-d", "tcase", 0};
  expect("tree: directories only", dirs, 0,
         "tcase\n|__ sub\n   |__ deep\n|__ empty\n");
  char *one[] = {"tree", "-L", "1", "tcase", 0};
  expect("tree: depth one", one, 0, "tcase\n|__ sub\n|__ empty\n|__ a\n");
  char *two[] = {"tree", "-L", "2", "tcase", 0};
  expect("tree: depth two", two, 0,
         "tcase\n|__ sub\n   |__ deep\n   |__ b\n|__ empty\n|__ a\n");
  char *file[] = {"tree", "tcase/a", 0};
  expect("tree: file path", file, 0, "tcase/a\n");
  char *dfile[] = {"tree", "-d", "tcase/a", 0};
  expect("tree: file hidden in directory mode", dfile, 0, "");
  char *empty[] = {"tree", "tcase/empty", 0};
  expect("tree: empty directory", empty, 0, "tcase/empty\n");
  char *missing[] = {"tree", "not-here", 0};
  expect("tree: missing path", missing, 1, "tree: cannot stat not-here\n");
  char *invalid[] = {"tree", "-x", 0};
  expect("tree: invalid option", invalid, 1, "tree: invalid option -- -x\n");
  char *depth[] = {"tree", "-L", "0", 0};
  expect("tree: invalid depth", depth, 1, "tree: invalid depth: 0\n");
  char *no_depth[] = {"tree", "-L", 0};
  expect("tree: missing depth", no_depth, 1, "tree: option requires an argument -- L\n");
  printf("ALL TREE TESTS PASSED\n");
}

static void
test_du(void)
{
  char *normal[] = {"du", "tcase", 0};
  expect("du: directory subtotals", normal, 0,
         "2\ttcase/sub/deep\n7\ttcase/sub\n0\ttcase/empty\n10\ttcase\n");
  char *all[] = {"du", "-a", "tcase", 0};
  expect("du: all files", all, 0,
         "2\ttcase/sub/deep/c\n2\ttcase/sub/deep\n5\ttcase/sub/b\n"
         "7\ttcase/sub\n0\ttcase/empty\n3\ttcase/a\n10\ttcase\n");
  char *summary[] = {"du", "-s", "tcase", 0};
  expect("du: summary only", summary, 0, "10\ttcase\n");
  char *both[] = {"du", "-a", "-s", "tcase", 0};
  expect("du: summary overrides detailed output", both, 0, "10\ttcase\n");
  char *file[] = {"du", "tcase/a", 0};
  expect("du: single file", file, 0, "3\ttcase/a\n");
  char *afile[] = {"du", "-a", "tcase/a", 0};
  expect("du: single file listed once", afile, 0, "3\ttcase/a\n");
  char *missing[] = {"du", "not-here", 0};
  expect("du: missing path", missing, 1, "du: cannot stat not-here\n");
  char *invalid[] = {"du", "-x", 0};
  expect("du: invalid option", invalid, 1, "du: invalid option -- -x\n");
  printf("ALL DU TESTS PASSED\n");
}

int
main(void)
{
  test_sleep();
  test_pingpong();
  test_primes();
  fixtures();
  test_tree();
  test_du();
  printf("ALL CORE TESTS PASSED\n");
  exit(0);
}
