#ifndef YOKE_FAST_IO_H
#define YOKE_FAST_IO_H

#ifdef __cpp__
extern "C" {
#endif

struct FastIOOpen {
  uint32_t root;
  uint8_t* path;
  uint32_t path_len;
  uint32_t flags;
  uint32_t acc_mode;
  uint64_t handle;
};

struct FastIORead {
  uint32_t root;
  uint64_t handle;
  uint64_t offset;
  uint32_t count;
  uint8_t* buffer;
};


#ifdef __cpp__
}
#endif

#endif
