#define _GNU_SOURCE
#include <sched.h>  // for clone
#include <signal.h> // for SIGCHLD
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <libgen.h>
#include <string.h>
#include <limits.h> 

#define BUFFER_SIZE 4096
char child_stack[1024*1024];

struct child_args {
	int out_pipe[2];
	int err_pipe[2];
	char* command;
	char** argv;
};

int copy_files(char* src, char* dest) {
	FILE* src_files = fopen(src, "rb");
	FILE* dest_files = fopen(dest, "wb");

	if (src_files == NULL || dest_files == NULL) {
		perror("Error opening files!\n");
		return EXIT_FAILURE;
	}

	// Read the source file and write to the destination file
	char buffer[BUFFER_SIZE];
	size_t bytes_read;
	while ((bytes_read = fread(buffer, 1, sizeof(buffer), src_files)) > 0) {
		fwrite(buffer, 1, bytes_read, dest_files);
	}

	fclose(src_files);
	fclose(dest_files);
	chmod(dest, S_IRWXU); // Set the permission of the file

	return EXIT_SUCCESS;
}

int create_and_change_docker_directory(char* curr_dir) {
	// Create a temporary directory
	char dir_name[] = "/tmp/mydockerXXXXXX";
	char* tmp_dir = mkdtemp(dir_name);
	if (tmp_dir == NULL) {
		perror("Error creating temporary directory!\n");
		return EXIT_FAILURE;
	}

	// Get the destination path
	char* file_name = basename(curr_dir);
	char* dest_path = malloc(strlen(tmp_dir) + strlen(file_name) + 2);
	sprintf(dest_path, "%s/%s", tmp_dir, file_name);

	// Copy the files to the temporary directory
	if (copy_files(curr_dir, dest_path) == EXIT_FAILURE) {
		perror("Error copying files!\n");
		return EXIT_FAILURE;
	}

	// Change the current directory to the temporary directory
	if (chdir(tmp_dir) == -1) {
		perror("Error changing directory!\n");
		return EXIT_FAILURE;
	}

	// Change the current root to the temporary directory using chroot
	char* new_dir = malloc(strlen(tmp_dir) + 2);
	getcwd(new_dir, strlen(tmp_dir) + 2);
	if (chroot(new_dir) != 0) {
		perror("Error changing root");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int child_function(void* arg) {
	struct child_args* args = (struct child_args*) arg;

	// Create and change the docker directory
	char curr_dir[PATH_MAX];
	getcwd(curr_dir, sizeof(curr_dir));
	printf("Current directory: %s\n", curr_dir);
	
	if (create_and_change_docker_directory(curr_dir) == EXIT_FAILURE) {
		perror("Error creating and changing docker directory!\n");
		return EXIT_FAILURE;
	}

	// Redirect the stdout and stderr
	dup2(args->out_pipe[1], STDOUT_FILENO);
	dup2(args->err_pipe[1], STDERR_FILENO);

	// Close the read end of the pipes
	close(args->out_pipe[0]);
	close(args->err_pipe[0]);

	/*printf("Executing %s\n",  (char*)args->command);
	printf("Command %s\n", (char*)args->argv[0]);
	printf("Command %s\n", (char*)args->argv[1]);
	printf("Command %s\n", (char*)args->argv[2]);
	printf("Command %s\n", (char*)args->argv[3]);*/

	// Execute the command
	if (execv(basename(args->command), args->argv[2]) == -1) {
		perror("execv failed");
		return EXIT_FAILURE;
	}
}

int main(int argc, char *argv[]) {
	setbuf(stdout, NULL);
	char *command = argv[3];

	// Set the output and error pipes
	int out_pipe[2];
	int err_pipe[2];
	pipe(out_pipe);
	pipe(err_pipe);


	// Revise the argv for the child process
	int len = argc - 3 + 2;
	char** new_args = calloc(len, sizeof(char*));
	memcpy(new_args, &argv[3], (len - 1) * sizeof(char*));

	// printf("Command in Main %s\n", command);

	struct child_args args = {out_pipe, err_pipe, command, new_args};

	// int child_pid = fork();
	int child_pid = clone(child_function, child_stack + (1024*1024), SIGCHLD, &args);
	if (child_pid == -1) {
	    perror("Error forking!");
	    return 1;
	}
	

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

	return EXIT_SUCCESS;
}
