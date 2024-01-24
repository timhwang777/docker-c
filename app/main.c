#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <libgen.h>
#include <string.h>

#define BUFFER_SIZE 4096

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

	char* new_dir = malloc(strlen(tmp_dir) + 2);
	getcwd(new_dir, strlen(tmp_dir) + 2);
	// Change the current root to the temporary directory using pivot_root
	if (chroot(new_dir) != 0) {
		perror("Error changing root");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
	setbuf(stdout, NULL);
	char *command = argv[3];

	// Set the output and error pipes
	int out_pipe[2];
	int err_pipe[2];
	pipe(out_pipe);
	pipe(err_pipe);

	int child_pid = fork();
	if (child_pid == -1) {
	    perror("Error forking!");
	    return 1;
	}
	
	if (child_pid == 0) { // Child process
		// Create and change the docker directory
		if (create_and_change_docker_directory(command) == EXIT_FAILURE) {
			perror("Error creating and changing docker directory!\n");
			return EXIT_FAILURE;
		}

		// Redirect the stdout and stderr
		dup2(out_pipe[1], STDOUT_FILENO);
		dup2(err_pipe[1], STDERR_FILENO);

		// Close the read end of the pipes
		close(out_pipe[0]);
		close(err_pipe[0]);

		// Execute the command
	    execv(basename(command), &argv[3]);
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

	return EXIT_SUCCESS;
}
