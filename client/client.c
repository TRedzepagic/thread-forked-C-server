#include<stdio.h>
#include<string.h>
#include<sys/socket.h> 
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>


#define SOCKETERROR (-1)
#define LISTENQUEUE 10

int check(int exp, const char *msg);

int main(int argc, char * argv[])
{
    int sockfd = 0;
    char sendBuffer[512];
    int bytesRead=0;
    struct sockaddr_in serv_addr;

    // Create a socket first
    check(sockfd = socket(AF_INET, SOCK_STREAM, 0),"\n Error: Could not create socket \n");
  
    	puts("Socket created");

    //  Initialize sockaddr_in data structure
    	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    	serv_addr.sin_family = AF_INET;
    	serv_addr.sin_port = htons( 8888 );

    // Attempt a connection
    check(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)),"\n Error : Connect Failed \n");
   	puts("Connected\n");

        // Open file for transfer
	    FILE *fp;        
	    fp = fopen(argv[1],"rb");
            if(fp==NULL)
            {
                printf("File open error");
                return 1;   
            }   
        

        sendBuffer[0]=strlen(argv[1]);
        strcpy(sendBuffer+1, argv[1]);
        
        check((send(sockfd , sendBuffer , sizeof(sendBuffer) , 0)),"Send failed!");

        // Read data from file and send it
        while(!feof(fp))
        {
           // Read chunks of 512B
            bytesRead=fread(sendBuffer,1,sizeof(sendBuffer),fp);
            printf("bytesread: %d \n", bytesRead); // On big files this will output 512B until everything is read
           // Send chunk by chunk
           check(send(sockfd , sendBuffer , bytesRead , 0),"Send failed!");
        }
       
    fclose(fp);
	close(sockfd);	
    return 0;
}

// Easy to use check function to eliminate code duplication
int check(int exp, const char *msg){
    if (exp == SOCKETERROR){
        perror(msg);
        exit(1);
    }
    return exp;
}