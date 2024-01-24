#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 4096;

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
		read(out_pipe[0], out, sizeof(out));
		read(err_pipe[0], err, sizeof(err));

		waitpid(child_pid, NULL, 0);
	}

	return 0;
}
