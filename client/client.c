// kompajlirajte sa: $gcc fileclient.c -o fileclient i startajte sa $./fileclient   ime-datoteke
#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include <stdbool.h>
#include<arpa/inet.h> //inet_addr

int main(int argc, char * argv[])
{
    int sockfd = 0;
    char sendBuffer[512];
    int bytesRead=0;
    struct sockaddr_in serv_addr;

    /* Create a socket first */
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }
    	puts("Socket created");

    /* Initialize sockaddr_in data structure */
    	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    	serv_addr.sin_family = AF_INET;
    	serv_addr.sin_port = htons( 8888 );

    /* Attempt a connection */
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }
    	puts("Connected\n");

        /* Open the file that we wish to transfer */
	    FILE *fp;        
	    fp = fopen(argv[1],"rb");
            if(fp==NULL)
            {
                printf("File open error");
                return 1;   
            }   
        
        sendBuffer[0]=strlen(argv[1])+1;
        strcpy(sendBuffer+1, argv[1]);
        
            if( send(sockfd , sendBuffer , sizeof(sendBuffer) , 0) < 0)
            {
                puts("Send failed");
                return 1;
            }

        /* Read data from file and send it*/ 
        while(!feof(fp))
        {
           /* First read file in chunks of 10 bytes */
            bytesRead=fread(sendBuffer,1,sizeof(sendBuffer),fp);
            /*Send a data in chunks of 10 bytes*/
            if( send(sockfd , sendBuffer , bytesRead , 0) < 0)
            {
        	    puts("Send failed");
        	    return 1;
            }


        }
       
    fclose(fp);
	close(sockfd);	
    return 0;
}