#include <stdint.h>
#include <stdarg.h>
static union {
  int x;
  char endian;
}native = { 1 };

int is_native_LE() {
  return native.endian;
}

void reverse(void* begin, void*end) {
  //[s,e)
  uint8_t* s = (uint8_t*)begin;
  uint8_t* e = (uint8_t*)end;
  --e;
  while (s < e) {
    uint8_t t = *s;
    *s = *e;
    *e = t;
    ++s, --e;
  }
}

int unpack(char const* buf, char const* fmt, ...) {
  va_list args;
  int isLE = is_native_LE();
  int off = 0;
  uint16_t* u16;
  uint32_t* u32;
  uint64_t* u64;
  uint8_t prexfix;
  va_start(args, fmt);
  while (*fmt) {
    prexfix = 0;
    while (*fmt >= '0' && *fmt <= '9') {
      prexfix = prexfix * 10 + *fmt - '0';
      ++fmt;
    }
    if (prexfix == 0) {
      prexfix = 1;
    }
    while (prexfix) {
      switch (*fmt) {
      case '<': isLE = 1; break;
      case '>': isLE = 0; break;
      case 'x': ++off; break; //pad bye
      case 'c':
      case 'b':
      case 'B': *va_arg(args, uint8_t*) = buf[off++]; break;
      case 'h':
      case 'H':
      {
        u16 = va_arg(args, uint16_t*); *u16 = *(uint16_t*)(buf + off); off += sizeof(*u16);
        if (is_native_LE() != isLE) {
          reverse(u16, u16 + 1);
        }
        break;
      }
      case 'i':
      case 'I':
      {
        u32 = va_arg(args, uint32_t*); *u32 = *(uint32_t*)(buf + off); off += sizeof(*u32);
        if (is_native_LE() != isLE) {
          reverse(u32, u32 + 1);
        }
        break;
      }
      case 'q':
      case 'Q':
      {
        u64 = va_arg(args, uint64_t*); *u64 = *(uint64_t*)(buf + off); off += sizeof(*u64);
        if (is_native_LE() != isLE) {
          reverse(u64, u64 + 1);
        }
        break;
      }
      case 'f': *va_arg(args, float*) = *(float*)(buf + off); off += sizeof(float); break;
      case 'd': *va_arg(args, double*) = *(double*)(buf + off); off += sizeof(double); break;
      case 's':
        *va_arg(args, char**) = (char*)(buf + off);
        if (prexfix > 1) {
          off += prexfix;  prexfix = 1;
        }
        else
          while (buf[off++]);
        break;  //do not copy
      case 'p': *va_arg(args, void**) = *(void**)(buf + off); off += sizeof(void*); break;//only used inproc
      default: break;
      }
      --prexfix;
    }
    ++fmt;
  }
  va_end(args);
  return off;
}


int pack(char * buf, char const* fmt, ...) {
  va_list args;
  int isLE = is_native_LE();
  int off = 0;
  char *str;
  uint16_t u16;
  uint32_t u32;
  uint8_t prexfix;
  uint64_t u64;
  va_start(args, fmt);
  while (*fmt) {
    prexfix = 0;
    while (*fmt >= '0' && *fmt <= '9') {
      prexfix = prexfix * 10 + (*fmt - '0');
      ++fmt;
    }
    if (prexfix == 0) {
      prexfix = 1;
    }
    while (prexfix) {
      switch (*fmt) {
      case '<': isLE = 1; break;
      case '>': isLE = 0; break;
      case 'x': ++off; break; //pad bye
      case 'c':
      case 'b':
      case 'B':  buf[off++] = va_arg(args, uint32_t); break;
      case 'h':
      case 'H':
      {
        u16 = va_arg(args, uint32_t);
        if (is_native_LE() != isLE) {
          reverse(&u16, &u16 + 1);
        }
        *(uint16_t*)(buf + off) = u16; off += sizeof(u16);
        break;
      }
      case 'i':
      case 'I':
      {
        u32 = va_arg(args, uint32_t);
        if (is_native_LE() != isLE) {
          reverse(&u32, &u32 + 1);
        }
        *(uint32_t*)(buf + off) = u32; off += sizeof(u32);
        break;
      }
      case 'q':
      case 'Q':
      {
        u64 = va_arg(args, uint64_t);
        if (is_native_LE() != isLE) {
          reverse(&u64, &u64 + 1);
        }
        *(uint64_t*)(buf + off) = u64; off += sizeof(u64);
        break;
      }
      case 'f': *(float*)(buf + off) = va_arg(args, double); off += sizeof(float); break;
      case 'd':  *(double*)(buf + off) = va_arg(args, double); off += sizeof(double); break;
      case 's': str = va_arg(args, char*);
        if (prexfix > 1) {
          while ((--prexfix, buf[off++] = *str++));  off += prexfix;  prexfix = 1;
        }
        else
          while ((buf[off++] = *str++));
        break;
      case 'p': *(void**)(buf + off) = va_arg(args, void**); off += sizeof(void*); break;
      default: break;
      }
      --prexfix;
    }
    ++fmt;
  }
  va_end(args);
  return off;
}

int main() {
  char buf[1024];

  pack(buf, ">Q4xphI12sf", (uint64_t)0x9981, buf, (int16_t)0x5678, (int32_t)0x123456, "hello what?", 3.1415926f);
  uint64_t x;
  int16_t y;
  int32_t z;
  float d;
  char*s;
  void *p;
  unpack(buf, ">Q4xphI12sf", &x, &p, &y, &z, &s, &d);
  return 0;
}