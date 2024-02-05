# Build Docker using C

![Static Badge](https://img.shields.io/badge/C-Solutions-Blue?logo=c
)

## Table of Contents
1. [About the Project](#about-the-project)
2. [Getting Started](#getting-started)
    - [Prerequisites](#prerequisites)
3. [Author](#author)

## About the Project
This project is about building a simple Docker in C. I have built some underlying Docker tools like getting the token from the Docker Registry, enumerating the Docker file system layers, and authorizing with tokens to download files from the Docker Registry. Moreover, I also developed a `child process` with an independent `PID`, e.g., `PID = 1` with `_GNU_SOURCE` and C's built-in library.

Please note that this project is only used for understanding the framework of Docker, Docker Registry, Internet theory, and Union File Systems used by Docker. Do not use this project for other purposes.

## Getting Started
To run the project, please execute the shell script:
```shell
./your_docker.sh
```

### Prerequisites
Make sure you have install all the dependencies and libraries for the project. Inspect the `/src` folder, `your_docker.sh` and source codes for more information.
Here are some libraries you may use:
- _GNU_SOURCE
- Signal
- Sys/wait, Sys/stat ...
- Libgen
- Curl

## Author
Timothy Hwang