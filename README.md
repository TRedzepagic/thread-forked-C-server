# thread-forked-C-server
C practice project - Multithreaded &amp; Multiprocess C server

## Program Description
This client-server pair uses two forms of concurrency (threading & forking-multiprocess).
We have a custom message format where the first byte of the buffer indicates the file name length (N bytes).
The next N bytes are the filename, and the rest is the file. The program uses TCP to send over the file in chunks of 512B after parsing its name length and name.
Multithreading and multiprocess techniques were employed.

Since this is a practice project any comments and criticisms are more than welcome, since the purpose of this program is to learn more about socket programming in C.

### Usage
First, start the server

```
./servername
```

Then, start the client with the filename as an argument. I've tried out two at the same time to test concurrency.

```
./client *filename*
```
Another terminal window
```
./client *filename2*
```

### Resources
    - Stack Overflow
    - Youtube
    - University Lectures