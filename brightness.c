#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define STR1(x) #x
#define STR(x) STR1(x)

#define SIZE(x) sizeof(x)/sizeof(x[0])

static void perr(const char* fcn_name, int line, const char* m) {
  fprintf(stderr,"%d: %s(): %s\n",line,fcn_name,m?m:strerror(errno));
}
#define ERRM(FCN_NAME,M) perr(FCN_NAME,__LINE__,M)
#define ERRN(FCN_NAME) perr(FCN_NAME,__LINE__,NULL)

const int lvls[] = {
  1, 250, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000,
  12000, 14000, 16000, 18000,
  19393
};
const int nlvls = SIZE(lvls);

int upper_bound(const int* a, int n, int x) {
  int l = 0, h = n;
  while (l < h) {
    int mid =  l + (h - l) / 2;
    if (x < a[mid])
      h = mid;
    else
      l = mid + 1;
  }
  return l;
}
int lower_bound(const int* a, int n, int x) {
  int l = 0, h = n;
  while (l < h) {
    int mid =  l + (h - l) / 2;
    if (x > a[mid])
      l = mid + 1;
    else
      h = mid;
  }
  return l;
}
int next(int x, bool up) {
  int i = up
  ? upper_bound(lvls,nlvls,x)
  : lower_bound(lvls,nlvls,x)-1;
  if (i < 0) i = 0;
  else if (i >= nlvls) i = nlvls-1;
  return lvls[i];
}

int fd;
void close_fd(void) { close(fd); }

void print_usage(const char* name) {
  printf("usage: %s [option]\n"
    "  no option  print current brightness\n"
    "  -u         raise brightness\n"
    "  -d         lower brightness\n"
    "  -v INT     set brightness\n"
    "  -m         print maximum brightness\n"
    "  -h         print help and exit\n",
    name);
}

enum opts { opt_none, opt_u, opt_d, opt_v, opt_m } opt = opt_none;
int val = 0;

int main(int argc, char* argv[]) {
  for (int o; (o = getopt(argc,argv,"udv:mh")) != -1; ) {
    if (opt != opt_none) {
      fprintf(stderr,"specify at most one option\n");
      return 1;
    }
    switch (o) {
      case 'h': print_usage(argv[0]); return 0;
      case 'u': opt = opt_u; break;
      case 'd': opt = opt_d; break;
      case 'v':
        opt = opt_v;
        val = atoi(optarg);
        break;
      case 'm': opt = opt_m; break;
      default : return 1;
    }
  }

  fd = open(
    (opt==opt_m)
    ? "/sys/class/backlight/intel_backlight/max_brightness"
    : "/sys/class/backlight/intel_backlight/brightness",
    (opt==opt_none || opt==opt_m) ? O_RDONLY : (O_RDWR | O_TRUNC)
  );
  if (fd == -1) { ERRN("open"); return 1; }
  atexit(close_fd);

  char str[16];
  ssize_t n = read(fd,str,sizeof(str)-1);
  if (n<0) { ERRN("read"); return 1; }
  if (n==0) ERRM("read","no value");
  str[n] = '\0';
  for (char *s=str, *end=str+sizeof(str); s<end; ++s) {
    char c = *s;
    if (c<'0'||'9'<c) {
      *s = '\0';
      break;
    }
  }

  if (opt==opt_none || opt==opt_m) {
    printf("%s\n",str);
  } else {
    if (opt==opt_u || opt==opt_d)
      val = next(atoi(str),opt==opt_u);
    snprintf(str,sizeof(str),"%d",val);
    printf("%s\n",str);
    n = strlen(str);

    n = write(fd,str,n);
    if (n<0) { ERRN("write"); return 1; }
  }
}
