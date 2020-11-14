#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/wait.h>
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

    // Number of child processes
    unsigned int childProcCount = 0;  


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
 
        if((childPID = fork())<0){
            // Error
            perror("Fork failed!");
            exit(1);
        }   else if(childPID==0){
            // We are inside the child process
            /* Linux queues up pending connections. 
            A call to accept, from either the parent or child process, will poll that queue. 
            Not closing the socket in the child process is a resource leak.
            The parent will still grab all the incoming connections, because it's the only one that calls accept (StackOverflow).*/
            close (serverSocketDescriptor);
            serverFunc(clientSocketDescriptor);
            exit(0);
        }   else {
            // We are inside the parent process
            printf("with child process' %d\n", (int) childPID);
            // the parent process has already closes clientSocketDescriptor since the connection is being handled by the child.
            close(clientSocketDescriptor); 
            // Increment number of child processes
            childProcCount++; 
            
            while (childProcCount) // Clean up zombies (Processes that have died but in process table)
            {
                childPID = waitpid((pid_t)-1, NULL, WNOHANG); // Nonblocking wait
                if (childPID < 0){
                    perror("waitpid() failed");
                    exit(1);
            }else if (childPID == 0) 
                // No more zombies
                break;
            else
                // Cleaned up after a child
                childProcCount--; 
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
  
    fileNameSize = recvBuff[0];
    char fileName[fileNameSize];
    fileName[fileNameSize]='\0';

    // Fill fileName with transferred name
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