# Key Skills Demonstrated
* Using pipes to communicate between a server and client process
* Using fork() to create a child process
* Using exec() to replace a process with another process
* Using wait() to wait for a child process to terminate
* Using dup2() to redirect stdin and stdout
* Using open() to open a file
* Using close() to close a file
* Using read() to read from a file
* Using write() to write to a file
* Using perror() to print an error message
* Using buffer to store data
  ## Purpose
  The purpose of this assignment is to demonstrate the use of pipes to communicate between a server and client process. The server process will read a file and send the contents to the client process. The client process will receive the contents and write them to a file. The server hosts electrocardiogram (ECG) data points of 15 patients suffering from cardiac diseases. The client communicates with the server to complete two tasks:
Obtain individual data points from the server.
Obtain a whole raw file of any size in one or more segments from the server.
These files implements a pipe-based communication channel. The client and server processes use this class to communicate with each other. This class has a read and a write function to receive and send data from/to another process, respectively. 