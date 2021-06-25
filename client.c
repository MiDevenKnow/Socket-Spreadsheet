// Dominique Reddicks - 620128588
// Abigail Mehabear - 620138889
// Kabian Davidson - 620129358

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>   //FD_SET, FD_ISSET, FD_ZERO macros

#define BUFFER_SIZE	    16348
#define SERVER_PORT	    67000
#define SERVER_IP     "127.0.0.1"

char letters[]= {'A','B','C','D','E','F','G','H','I'};
char numbers[]= {'1','2','3','4','5','6','7','8','9'};

bool isOnBoard(char *cell);

int main(){
    system("clear || cls");

    struct sockaddr_in clientAddr;
    int clientSocket;
    int bytes_recv;
    int i;    
    
    char input[10000];
    char text[10000];
    char cell[10000] = "";
    char buffer[BUFFER_SIZE];

    //Create the socket.
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(clientSocket<0){
        puts("socket() failed");
        exit(0);
    }


    /* setsockopt: Handy debugging trick that lets 
    * us rerun the server immediately after we kill it; 
    * otherwise we have to wait about 20 secs. 
    * Eliminates "ERROR on binding: Address already in use" error. 
    */
    int optval = 1;
    setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

    //Empty the structure. It's a safety measure.
    memset(&clientAddr,0,sizeof(clientAddr));

    // Configure settings of the server address struct
    // Address family = Internet 
    clientAddr.sin_family = AF_INET;

    //Set port number, using htons function to use proper byte order.
    //Cast as unsigned short int because thats the datatype htons accepts.
    clientAddr.sin_port = htons((unsigned short)SERVER_PORT);

    //Set IP address to localhost 
    clientAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    //Bind the address struct to the socket 
    i = connect(clientSocket, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
    if(i<0){
        puts("connect() failed");
        exit(0);
    }

    while(1){
        system("clear || cls");
        bytes_recv = recv(clientSocket,buffer,BUFFER_SIZE,0);
        buffer[bytes_recv]=0;
        
        if(strstr(buffer,"SHUT ME DOWN NOW")!=NULL){
            puts("The first user has quit, connection has been terminated...");
            break;
        }
        printf("%s\n\n",buffer);

        memset(buffer,0,strlen(buffer));

        printf("Enter the cell that is to store the input: ");
        scanf("%s",cell);
        if(!isOnBoard(cell)){
            do{
                printf("\nPlease enter a correct cell to store the input: ");
                scanf("%s",cell);
            }while(!isOnBoard(cell));
        }
        char c;
        scanf("%c",&c);
        puts("\nPlease note that if any errors are within your formulas.\nBe it an invalid range or misspelled formula, the formula will be treated as text.");
        puts("\nTo close your connection to the server, simply send input containing the word 'quit'");
        printf("\nInput: ");
        scanf("%[^\n]",input);

        strcat(text,cell);
        strcat(text,"||");
        strcat(text,input);

        strcpy(buffer,text);

        if(strstr(input,"quit")!=NULL){
            break;
        }else{
            send(clientSocket,buffer,strlen(text),0);
        }
        
        system("clear || cls");
        memset(buffer,0,strlen(buffer));
        memset(text,0,strlen(text));
    }
    close(clientSocket);
    return 0;
}

//Returns true if a given cell is on the board, false otherwise.
bool isOnBoard(char *cell){
    if(strlen(cell)!=2){
        return false;
    }
    if(strchr(letters,toupper(cell[0])) == NULL){
        return false;
    }
    if(strchr(numbers,cell[1]) == NULL){
        return false;
    }
    return true;
}