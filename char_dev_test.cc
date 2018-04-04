#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv) {
  if (argc < 3) {
    printf("Usage: %s [device path] [string value].\n", argv[0]);
    return 0;
  }

  const char* path = argv[1];
  int fd = open(path, O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "Failed to open device.\n");
    return errno;
  }

  const char* value = argv[2];
  printf("Writing %d bytes to device %s.\n", strlen(value), path);
  int ret = write(fd, value, strlen(value));
  if (ret < 0) {
    fprintf(stderr, "Failed to write to device.\n");
    return errno;
  }

  printf("Reading from device %s.\n", path);
  char buffer[256];
  ret = read(fd, buffer, sizeof(buffer));
  if (ret < 0) {
    fprintf(stderr, "Failed to read from device.\n");
    return errno;
  }

  printf("Received message [%s] from device.%s\n", buffer, path);

  return 0;
}
