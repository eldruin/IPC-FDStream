/* This is a basic example of inter-process communication
 * using fork() and pipe() in C++
 *
 * Includes an example of creating a C++ stream from a file descriptor
 * and how to read/write from/to it using << and >> operators
 *
 * Needs GCC's libstdc++ and a POSIX environment
 *
 * Resources:
 *      YoLinux Tutorial: Fork, Exec and Process control
 *      Beginning Linux programming, Matthew & Stones, Wrox Press
 *      The GNU C Library manual
 *      The GNU C++ Library manual
 *      fileno(3) on C++ Streams: A Hacker's Lament, Richard B. Kreckel
 *
 * Author: Diego Barrios Romero
 * Public domain Copyleft 2011
 */

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <csignal>

#include "fdstream.hpp"

using namespace std;

#define CHILD_STDIN_READ pipefds_input[0]
#define CHILD_STDIN_WRITE pipefds_input[1]
#define CHILD_STDOUT_READ pipefds_output[0]
#define CHILD_STDOUT_WRITE pipefds_output[1]
#define CHILD_STDERR_READ pipefds_error[0]
#define CHILD_STDERR_WRITE pipefds_error[1]

// Child exec error signal
void exec_failed (int sig)
{
  cerr << "Exec failed. Child process couldn't be launched." << endl;
  exit (EXIT_FAILURE);
}

int main()
{
  // Child error signal install
  // sigaction is the replacement for the traditional signal() method
  struct sigaction action;
  action.sa_handler = exec_failed;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  if (sigaction(SIGUSR1, &action, NULL) < 0)
    {
      perror("SIGUSR1 install error");
      exit(EXIT_FAILURE);
    }

  int pipe_status;
  int pipefds_input[2], pipefds_output[2], pipefds_error[2];

  // Create the pipes
  // We do this before the fork so both processes will know about
  // the same pipe and they can communicate.

  pipe_status = pipe(pipefds_input);
  if (pipe_status == -1)
    {
      perror("Error creating the pipe");
      exit(EXIT_FAILURE);
    }

  pipe_status = pipe(pipefds_output);
  if (pipe_status == -1)
    {
      perror("Error creating the pipe");
      exit(EXIT_FAILURE);
    }

  pipe_status = pipe(pipefds_error);
  if (pipe_status == -1)
    {
      perror("Error creating the pipe");
      exit(EXIT_FAILURE);
    }

  pid_t pid;
  // Create child process; both processes continue from here
  pid = fork();

  if (pid == pid_t(0))
    {
      // Child process

      // When the child process finishes sends a SIGCHLD signal
      // to the parent

      // Tie the standard input, output and error streams to the
      // appropiate pipe ends
      // The file descriptor 0 is the standard input
      // We tie it to the read end of the pipe as we will use
      // this end of the pipe to read from it
      dup2 (CHILD_STDIN_READ,0);
      dup2 (CHILD_STDOUT_WRITE,1);
      dup2 (CHILD_STDERR_WRITE,2);
      // Close in the child the unused ends of the pipes
      close(CHILD_STDIN_WRITE);
      close(CHILD_STDOUT_READ);
      close(CHILD_STDERR_READ);

      // Execute the program
      execl("./program", "program", (char*)NULL);

      // We should never reach this point
      // Tell the parent the exec failed
      kill(getppid(), SIGUSR1);
      exit(EXIT_FAILURE);
    }
  else if (pid > pid_t(0))
    {
      // Parent

      // Close in the parent the unused ends of the pipes
      close(CHILD_STDIN_READ);
      close(CHILD_STDOUT_WRITE);
      close(CHILD_STDERR_WRITE);

      string message;

      cout << "Parent: I'll send the child a message." << endl;

      ofdstream in_stream(CHILD_STDIN_WRITE);
      in_stream << "Hello Child!\n";

      ifdstream out_stream(CHILD_STDOUT_READ);
      getline(out_stream, message);

      cout << "Parent: Child just said through stdout: " << endl
	   << "\t\"" << message  << "\"" << endl;

      ifdstream err_stream(CHILD_STDERR_READ);
      getline(err_stream, message);

      cout << "Parent: Child just said through stderr: " << endl
	   << "\t\"" << message  << "\""<< endl;

      close(CHILD_STDIN_WRITE);
      close(CHILD_STDOUT_READ);
      close(CHILD_STDERR_READ);
      exit(EXIT_SUCCESS);
    }
  else
    {
      perror("Error: fork failed");
      exit(EXIT_FAILURE);
    }

  // Code here is shared
}
