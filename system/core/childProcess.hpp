#pragma once
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#define PIPE_READ 0
#define PIPE_WRITE 1

auto inline createChild(const char *szCommand, char *const aArguments[],
                        char *const aEnvironment[], const char *szMessage)
    -> int {
  int aStdinPipe[2];
  int aStdoutPipe[2];
  int nChild;
  char nChar;
  int nResult;

  if (pipe(aStdinPipe) < 0) {
    perror("allocating pipe for child input redirect");
    return -1;
  }
  if (pipe(aStdoutPipe) < 0) {
    close(aStdinPipe[PIPE_READ]);
    close(aStdinPipe[PIPE_WRITE]);
    perror("allocating pipe for child output redirect");
    return -1;
  }

  nChild = fork();
  if (0 == nChild) {
    // child continues here

    // redirect stdin
    if (dup2(aStdinPipe[PIPE_READ], STDIN_FILENO) == -1) {
      exit(errno);
    }

    // redirect stdout
    if (dup2(aStdoutPipe[PIPE_WRITE], STDOUT_FILENO) == -1) {
      exit(errno);
    }

    // redirect stderr
    if (dup2(aStdoutPipe[PIPE_WRITE], STDERR_FILENO) == -1) {
      exit(errno);
    }

    // all these are for use by parent only
    close(aStdinPipe[PIPE_READ]);
    close(aStdinPipe[PIPE_WRITE]);
    close(aStdoutPipe[PIPE_READ]);
    close(aStdoutPipe[PIPE_WRITE]);

    // run child process image
    // replace this with any exec* function find easier to use ("man exec")
    nResult = execve(szCommand, aArguments, aEnvironment);

    // if we get here at all, an error occurred, but we are in the child
    // process, so just exit
    exit(nResult);
  } else if (nChild > 0) {
    // parent continues here

    // close unused file descriptors, these are for child only
    close(aStdinPipe[PIPE_READ]);
    close(aStdoutPipe[PIPE_WRITE]);

    // Include error check here
    if (NULL != szMessage) {
      write(aStdinPipe[PIPE_WRITE], szMessage, strlen(szMessage));
    }

    // Just a char by char read here, you can change it accordingly
    while (read(aStdoutPipe[PIPE_READ], &nChar, 1) == 1) {
      write(STDOUT_FILENO, &nChar, 1);
    }

    // done with these in this example program, you would normally keep these
    // open of course as long as you want to talk to the child
    close(aStdinPipe[PIPE_WRITE]);
    close(aStdoutPipe[PIPE_READ]);
  } else {
    // failed to create child
    close(aStdinPipe[PIPE_READ]);
    close(aStdinPipe[PIPE_WRITE]);
    close(aStdoutPipe[PIPE_READ]);
    close(aStdoutPipe[PIPE_WRITE]);
  }
  return nChild;
}
