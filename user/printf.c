#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#include <stdarg.h>

static char digits[] = "0123456789ABCDEF";

struct printbuf {
  int fd;
  char buf[256];
  int i;
};

static void
bputc(struct printbuf *b, char c)
{
  b->buf[b->i++] = c;
  if(b->i == sizeof(b->buf) || c == '\n'){
    write(b->fd, b->buf, b->i);
    b->i = 0;
  }
}

static void
printint(struct printbuf *b, int xx, int base, int sgn)
{
  char buf[16];
  int i, neg;
  uint x;

  neg = 0;
  if(sgn && xx < 0){
    neg = 1;
    x = -xx;
  } else {
    x = xx;
  }

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);
  if(neg)
    buf[i++] = '-';

  while(--i >= 0)
    bputc(b, buf[i]);
}

static void
printptr(struct printbuf *b, uint64 x) {
  int i;
  bputc(b, '0');
  bputc(b, 'x');
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    bputc(b, digits[x >> (sizeof(uint64) * 8 - 4)]);
}

// Print to the given fd. Only understands %d, %x, %p, %s.
void
vprintf(int fd, const char *fmt, va_list ap)
{
  char *s;
  int c, i, state;
  struct printbuf b;
  b.fd = fd;
  b.i = 0;

  state = 0;
  for(i = 0; fmt[i]; i++){
    c = fmt[i] & 0xff;
    if(state == 0){
      if(c == '%'){
        state = '%';
      } else {
        bputc(&b, c);
      }
    } else if(state == '%'){
      if(c == 'd'){
        printint(&b, va_arg(ap, int), 10, 1);
      } else if(c == 'l') {
        printint(&b, va_arg(ap, uint64), 10, 0);
      } else if(c == 'x') {
        printint(&b, va_arg(ap, int), 16, 0);
      } else if(c == 'p') {
        printptr(&b, va_arg(ap, uint64));
      } else if(c == 's'){
        s = va_arg(ap, char*);
        if(s == 0)
          s = "(null)";
        while(*s != 0){
          bputc(&b, *s);
          s++;
        }
      } else if(c == 'c'){
        bputc(&b, va_arg(ap, uint));
      } else if(c == '%'){
        bputc(&b, c);
      } else {
        // Unknown % sequence.  Print it to draw attention.
        bputc(&b, '%');
        bputc(&b, c);
      }
      state = 0;
    }
  }
  if(b.i > 0)
    write(b.fd, b.buf, b.i);
}

void
fprintf(int fd, const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vprintf(fd, fmt, ap);
}

void
printf(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vprintf(1, fmt, ap);
}
