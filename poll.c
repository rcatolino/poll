#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define buf_sz 1024
#define debug(...) fprintf(stderr, __VA_ARGS__)

int forward(int fd) {
  char buffer[buf_sz];
  ssize_t size = 0;
  while (1) {
    size = read(fd, buffer, buf_sz);
    if ((size < 0 && errno == EAGAIN) || size == 0) {
      break;
    } else if (size < 0) {
      perror("read on fd %d failed ");
      return -1;
    } else {
      ssize_t written = 0;
      while (written < size) {
        ssize_t ret = write(1, buffer, size-written);
        if (ret < 0) {
          perror("write on standard output failed ");
          return -1;
        }
        written += ret;
      }
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
  struct pollfd fds[argc];
  fds[0].fd = 0;
  fcntl(0, F_SETFL, O_NONBLOCK);
  for (int i = 1; i < argc; ++i) {
    fds[i].fd = open(argv[i], O_RDONLY | O_NONBLOCK);
    fds[i].events = POLLIN | POLLPRI;
    if (fds[i].fd == -1) {
      fprintf(stderr, "Error opening %s : %s\n", argv[i], strerror(errno));
      return -1;
    }
  }

  while (1) {
    switch(poll(fds, argc, -1)) {
      case -1:
        perror("poll ");
        return -2;
      case 0:
        return 0;
      default:
        for (int i = 0; i < argc; ++i) {
          if (fds[i].revents & (POLLIN | POLLPRI | POLLRDBAND)) {
            if (forward(fds[i].fd) != 0) {
              return -3;
            }
          }
          if (fds[i].revents & POLLERR) {
            fprintf(stderr, "Error on %s\n",
                    i==0 ? "standard input" : argv[i]);
            return -4;
          }
          if (fds[i].revents & POLLHUP) {
            if (i == 0) {
              // Can't reopen standard input
              return 0;
            }
            close(fds[i].fd);
            fds[i].fd = open(argv[i], O_RDONLY | O_NONBLOCK);
            if (fds[i].fd == -1) {
              fprintf(stderr, "Error reopening %s : %s\n", argv[i], strerror(errno));
              return -5;
            }
          }
          if (fds[i].revents & POLLNVAL) {
            fprintf(stderr, "Invalid file descriptor for %s\n",
                    i==0 ? "standard input" : argv[i]);
            return -6;
          }
        }
    }
  }
}
