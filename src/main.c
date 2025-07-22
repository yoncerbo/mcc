#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, const char *argv[]) {
  assert(argc == 2);
  const char *filename = argv[1];
  printf("Reading file '%s'\n", filename);

  int fd = open(filename, O_RDONLY);
  assert(fd >= 0);

  struct stat st;
  assert(fstat(fd, &st) >= 0);

  char *file = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  assert(file != MAP_FAILED);

  return 0;
}
