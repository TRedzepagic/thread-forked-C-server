#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<stdlib.h>

#define SOCKETERROR (-1)
#define LISTENQUEUE 10

void* serverFunc(void*);
int check(int exp, const char *msg);

// Could pass socket descriptor only, but a structure is scalable for more information
struct ThreadArgs
{
    // Socket descriptor (client)
    int clntSock;           
};

int main(void)
{
    int serverSocketDescriptor, clientSocketDescriptor, addr_size = 0;
    struct sockaddr_in serv_addr,client;
    pthread_t threadID; 
    struct ThreadArgs *threadArgs; 

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

        // Create separate memory for client argument
        if ((threadArgs = (struct ThreadArgs *) malloc(sizeof(struct ThreadArgs))) == NULL){
            perror ("malloc() failed");
            exit(1);
        }
        threadArgs -> clntSock = clientSocketDescriptor;

        // Create client thread
        if (pthread_create(&threadID, NULL, serverFunc, (void *) threadArgs) != 0){
            perror ("pthread_create() failed");
            exit(1);
        }

    }
    return 0;
}

void* serverFunc(void* threadArgs)
{   
    int clientSocketDescriptor = ((struct ThreadArgs *) threadArgs) -> clntSock;
    free(threadArgs);

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
         close(clientSocketDescriptor);
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

// Easy to use check function to eliminate code duplication
int check(int exp, const char *msg){
    if (exp == SOCKETERROR){
        perror(msg);
        exit(1);
    }
    return exp;
}