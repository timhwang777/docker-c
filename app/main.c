#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#define BUFFER_SIZE 4096

int main(int argc, char *argv[]) {
	setbuf(stdout, NULL);
	
	// Set the output and error pipes
	int out_pipe[2];
	int err_pipe[2];
	pipe(out_pipe);
	pipe(err_pipe);

	char *command = argv[3];
	int child_pid = fork();
	if (child_pid == -1) {
	    printf("Error forking!");
	    return 1;
	}
	
	if (child_pid == 0) { // Child process
		// Redirect the stdout and stderr
		dup2(out_pipe[1], STDOUT_FILENO);
		dup2(err_pipe[1], STDERR_FILENO);

		// Close the read end of the pipes
		close(out_pipe[0]);
		close(err_pipe[0]);

		// Execute the command
	    execv(command, &argv[3]);
	} 
	else { // Parent process
		// Close the write end of the pipes
		close(out_pipe[1]);
		close(err_pipe[1]);

		// Read the output and error
		char out[BUFFER_SIZE];
		char err[BUFFER_SIZE];
		int out_bytes_read = read(out_pipe[0], out, sizeof(out));
		int err_bytes_read = read(err_pipe[0], err, sizeof(err));
		// Write the output and error
		if (out_bytes_read != errno) {
			out[out_bytes_read] = '\0';
			write(STDOUT_FILENO, out, out_bytes_read);
		}
		if (err_bytes_read != errno) {
			err[err_bytes_read] = '\0';
			write(STDERR_FILENO, err, err_bytes_read);
		}

		// Examines the exit status of the child process
		int status, exit_status;
		waitpid(child_pid, &status, 0);
		exit_status = WEXITSTATUS(status);

		exit(exit_status);
	}	

	return 0;
}
