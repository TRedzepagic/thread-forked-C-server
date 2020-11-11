#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<pthread.h>
#include <sys/wait.h>
#include<stdlib.h>

#define SOCKETERROR (-1)
#define LISTENQUEUE 10

void* serverFunc(int);
int check(int exp, const char *msg);

int main(void)
{
    int serverSocketDescriptor, clientSocketDescriptor, addr_size = 0;
    struct sockaddr_in serv_addr,client;
    pid_t childPID; 
    unsigned int childProcCount = 0;  // Number of child processes


    check((serverSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0)), "Could not create socket!");
    puts("Socket created");

    //Prepare the sockaddr_in structure
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons( 8888 );

    // Binding the socket
    check((bind(serverSocketDescriptor, (struct sockaddr*)&serv_addr,sizeof(serv_addr))), "Bind failed!");
    puts("bind done");

    // Listening
    check(listen(serverSocketDescriptor,LISTENQUEUE),"Listening failed!");

    puts("Waiting for incoming connections...");
    addr_size = sizeof(struct sockaddr_in);

    while(1) {
        check((clientSocketDescriptor=accept(serverSocketDescriptor,(struct sockaddr *)&client, (socklen_t*)&addr_size)),"Accept failed!");

        // clientSocketDescriptor=accept(serverSocketDescriptor,(struct sockaddr *)&client, (socklen_t*)&addr_size);
        // if (clientSocketDescriptor<0)
        // {
        //         perror("ACCEPT ERROR");
        //         exit (1);
        //     }
 
        if((childPID = fork())<0){
            // Error
            perror("Fork failed!");
            exit(1);
        }   else if(childPID==0){
            // Child
            /* Linux queues up pending connections. 
            A call to accept, from either the parent or child process, will poll that queue. 
            Not closing the socket in the child process is a resource leak.
            The parent will still grab all the incoming connections, because it's the only one that calls accept (StackOverflow).*/
            close (serverSocketDescriptor);
            serverFunc(clientSocketDescriptor);
            exit(0);
        }   else {
            // Parent
            printf("with child process' %d\n", (int) childPID);
            close(clientSocketDescriptor); // the parent process has already closes clientSocketDescriptor since the connection is being handled by the child.
            childProcCount++; // Increment number of child processes

            while (childProcCount) // Clean up zombies (Processes that have died but in process table)
            {
            childPID = waitpid((pid_t)-1, NULL, WNOHANG); // Nonblocking wait
            if (childPID < 0){
                // waitpid() error?
                perror("waitpid() failed");
                exit(1);
            }
            else if (childPID == 0) // No more zombies
                break;
            else
                childProcCount--; // Cleaned up after a child
            }

        }
    }
    return 0;
}

void* serverFunc(int clientSocketDescriptor)
{   
    printf("Entered serverFunc, connection file desc is: %d\n", clientSocketDescriptor);

    char recvBuff[512];
    int bytesReceived = 0;
    int fileNameSize = 0;

    check((bytesReceived=recv(clientSocketDescriptor, recvBuff,sizeof(recvBuff),0)),"recv Failed!");
  
    char fileName[fileNameSize];
    strcpy(fileName, recvBuff+1);

     FILE *fp;
     fp = fopen(fileName, "wb"); 
     if(NULL == fp)
     {
         printf("Error opening file\n");
         return NULL;
     }  
    
    
    printf("Created filename: %s, filename length: %ld\n", fileName, strlen(fileName));
    
    // Sleep test, gives time to start one more client to test concurrency.
    puts("sleeping");
    sleep(10);

    // Receive data in chunks of 512B
    while((check(bytesReceived=recv(clientSocketDescriptor, recvBuff,sizeof(recvBuff),0),"recv failed!") > 0))
    {
        fwrite(recvBuff,1,bytesReceived,fp);
    }

	if(bytesReceived == 0)
        puts("Client disconnected");
    
    printf("Completed file transfer of %s\n", fileName);
        
    close(clientSocketDescriptor);
    fclose(fp);
    return NULL;
  
}

int check(int exp, const char *msg){
    if (exp == SOCKETERROR){
        perror(msg);
        exit(1);
    }
    return exp;
}