// Dominique Reddicks - 620128588
// Abigail Mehabear - 620138889
// Kabian Davidson - 620129358


#include <stdio.h>      //Used for all the standard input/output functionalities
#include <string.h>     //Used for string manipulation & comparison
#include <stdlib.h>     //Used for console manipulation, clear screen and exit()
#include <ctype.h>      //Used for the single character comparison & manipulation, isdigit(), toupper() etc
#include <stdbool.h>    //Used for the inclusion of the boolean datatype
#include <sys/types.h>  //Used for system type definitions
#include <sys/socket.h> //Used for network system functions
#include <netinet/in.h> //Used for protocol & struct definitions
#include <arpa/inet.h>  //Used for the conversion of the server ip address
#include <unistd.h>     //Used for the closure of the sockets
#include <sys/time.h>   //FD_SET, FD_ISSET, FD_ZERO macros

#define BUFFER_SIZE	16348
#define PORT	    67000
#define HOST        "127.0.0.1"
#define NUM_RANGE 9

//NB. Remember to free variable for the isFormula function if and only if it is indeed a formula.

char *grid[NUM_RANGE][NUM_RANGE];

/*States the type of data stored
T - TEXT
N - NUMERIC DATA
F - FORMULA
none - no data 
*/
char *type[NUM_RANGE][NUM_RANGE];
char *print[NUM_RANGE][NUM_RANGE];

char* getValue(char *cell);
char* isFormula(char* input);

void serverFunction(int connection);
void getNewPrint();
void getNewBoard();
void drawBoard2();
void drawBoard(int connection);
void makeEntry();
void saveWorksheet();
void setPrint(char *cell, char *input);
void setCell(char *cell, char *input);
void setType(char *cell, char *input);

bool isOnBoard(char *cell);
bool isValidRange(char* cell_1, char* cell_2);
bool stringCmp(char *str1, char *str2);
bool isNumeric(char* input);

float average(char *x, char *y);
float sum(char *cell_1, char *cell_2);
float range(char *cell_1, char *cell_2);

char letters[]= {'A','B','C','D','E','F','G','H','I'};
char numbers[]= {'1','2','3','4','5','6','7','8','9'};

int serverSocket;
int clients[30];
char usr[30];
int bytes_recv;
char buffer[BUFFER_SIZE];
fd_set readfds;
int count =0;

int main(){
    system("clear || cls");
    getNewBoard();
    getNewPrint();


    struct sockaddr_in serverAddr;
    struct sockaddr_in connectionAddr;

    int connection;
    int i;
    int max_clients = 30;
    int max_sd; //Highest File descriptor number
    int sd; //File descriptor number

    
    //initialise all clients[] to 0 so not checked 
    for (int x = 0; x < max_clients; x++)  
    {  
        clients[x] = 0;  
    }  

    //Create the socket.
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(serverSocket<0){
        puts("socket() failed");
        exit(0);
    }

    /* setsockopt: Handy debugging trick that lets 
    * us rerun the server immediately after we kill it; 
    * otherwise we have to wait about 20 secs. 
    * Eliminates "ERROR on binding: Address already in use" error. 
    */
    int optval = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

    //Empty the structure. It's a safety measure.
    memset(&serverAddr,0,sizeof(serverAddr));

    //bzero is an in-built function that also can be used to empty the structure
    //bzero(&serverAddr,sizeof(serverAddr));

    // Configure settings of the server address struct
    // Address family = Internet 
    serverAddr.sin_family = AF_INET;

    //Set port number, using htons function to use proper byte order.
    //Cast as unsigned short int because thats the datatype htons accepts.
    serverAddr.sin_port = htons((unsigned short)PORT);

    //Set IP address to localhost 
    serverAddr.sin_addr.s_addr = inet_addr(HOST);

    //Bind the address struct to the socket 
    i = bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if(i<0){
        puts("bind() failed");
        exit(0);
    }

    //Server is listening for incoming connection with a maximum of 5 queued
    i=listen(serverSocket, 5);
    if (i < 0){
        printf("listen() failed\n");
        exit(0);
    }else{
        printf("Server is active, running and listening on port %d\n",PORT);
    }
    
    socklen_t addr_size = sizeof(connectionAddr);

    while(1){
        //Clear the socket set
        FD_ZERO(&readfds);

        //Add server socket to set
        FD_SET(serverSocket,&readfds);
        max_sd = serverSocket;

        //add child sockets to set 
        for ( i = 0 ; i < max_clients ; i++)  
        {  
            //socket descriptor 
            sd = clients[i];  
                 
            //if valid socket descriptor then add to read list 
            if(sd > 0)  
                FD_SET( sd , &readfds);  
                 
            //highest file descriptor number, need it for the select function 
            if(sd > max_sd)  
                max_sd = sd;  
        }  

        //wait for an activity on one of the sockets , timeout is NULL , 
        //so wait indefinitely 
        if (select(max_sd+1, &readfds, NULL, NULL, NULL) < 0) {
            puts("ERROR in select");
            exit(0);
        }

        //If something happened on the server socket , 
        //then its an incoming connection 
        if(FD_ISSET(serverSocket,&readfds)){
            socklen_t addr_size = sizeof(connectionAddr);
            connection = accept(serverSocket, (struct sockaddr*) &connectionAddr, &addr_size);

            if(connection < 0){
                puts("ERROR on accept");
                exit(0);
            }
            if(count==0){
                usr[count]='Y';
            }else{
                usr[count]='N';
            }
            count++;
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , connection , inet_ntoa(serverAddr.sin_addr) , ntohs(serverAddr.sin_port));
            
            drawBoard(connection);
            
            //add new socket to array of sockets 
            for (int i = 0; i < max_clients; i++)  
            {  
                //if position is empty 
                if(clients[i] == 0 )  
                {  
                    clients[i] = connection;                       
                    break;  
                }  
            }   
        }
        
        //else its some IO operation on some other socket
        for (int i = 0; i < max_clients; i++)  
        {  
            sd = clients[i];  
                 
            if (FD_ISSET( sd , &readfds))  
            {  
                //Check if it was for closing , and also read the 
                //incoming message 
                if ((bytes_recv = read( sd , buffer, 1024)) == 0)  
                {  
                    //Somebody disconnected , get his details and print 
                    getpeername(sd , (struct sockaddr*)&serverAddr , \
                        (socklen_t*)&addr_size);  
                    printf("Host disconnected , ip %s , port %d \n" , 
                          inet_ntoa(serverAddr.sin_addr) , ntohs(serverAddr.sin_port));  
                         
                    //Close the socket and mark as 0 in list for reuse 
                    close( sd );  
                    clients[i] = 0;
                    count--;  
                    if(usr[i]=='Y'){
                        for(int x=1;x<max_clients;x++){
                            if(clients[x] != 0){
                                send(clients[x],"SHUT ME DOWN NOW",17,0);
                                close(clients[x]);
                            }
                        }
                        saveWorksheet();
                        close(serverSocket);
                        exit(0);
                    }
                }  
                     
                //Send back the board to all clients
                else 
                {  
                    //set the string terminating NULL byte on the end 
                    //of the data read 
                    buffer[bytes_recv] = '\0';  
                    if(usr[i]=='Y' && strstr(buffer,"quit")!=NULL){
                        for(int x=1;x<max_clients;x++){
                            if(clients[x] != 0){
                                send(clients[x],"SHUT ME DOWN NOW",17,0);
                                close(clients[x]);
                            }
                        }
                        saveWorksheet();
                        close(serverSocket);
                        exit(0);
                    }else{
                        makeEntry(buffer);
                        drawBoard2();
                        send(clients[i] , buffer , strlen(buffer) , 0 ); 

                    }
             
                }  
            }  
        }
    }

    close(serverSocket);
    return 0;
}

//Prints out the current spreadsheet into the file "Worksheet.txt". Returns void.
void saveWorksheet(){
    FILE *sheet = fopen("Worksheet.txt","w");

    int k,j;
    char * const NLINE = "          A               B               C               D               E               F               G               H               I";
    char * const HLINE = "  +---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+";
    char * const VLINE = "  |               |               |               |               |               |               |               |               |               |";

    fprintf(sheet,"%s\n",NLINE);
    fprintf(sheet,"%s\n",HLINE);
    for (j = 0; j < NUM_RANGE; j++)
    {
        fprintf(sheet,"%s\n",VLINE);
        fprintf(sheet,"%d ",j+1);
        for (k = 0; k < NUM_RANGE; k++)
        {  if(strcmp(print[k][j],"              ")==0){
                fprintf(sheet,"| %s",print[k][j]);
            }else{
                fprintf(sheet,"| %s  ",print[k][j]);
            }
        }
        fprintf(sheet,"%s","|");
        fprintf(sheet,"\n");
        fprintf(sheet,"%s\n",VLINE);
        fprintf(sheet,"%s\n",HLINE);
    }
    return;
}

//Creates a brand new blank board. Returns void.
void getNewBoard(){
    for(int x=0;x<NUM_RANGE;x++){
        for(int i=0;i<NUM_RANGE;i++){
            type[x][i]="none";
        }
    }
    int k,j;
    for (j = 0; j < NUM_RANGE; j++){
        for (k = 0; k < NUM_RANGE; k++)
        {
            grid[k][j]="              ";
        }
    }
    return;
}

//Creates a brand new blank print board. Returns void.
void getNewPrint(){
    int k,j;
    for (j = 0; j < NUM_RANGE; j++){
        for (k = 0; k < NUM_RANGE; k++)
        {
            print[k][j]="              ";
        }
    }
    return;
}

//Builds the board that was passed and send it to the given connection. Returns void.
void drawBoard(int connection){
    char text[BUFFER_SIZE];
    int k,j;
    char * const NLINE = "          A               B               C               D               E               F               G               H               I";
    char * const HLINE = "  +---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+";
    char * const VLINE = "  |               |               |               |               |               |               |               |               |               |";

    strcat(text,"\e[1;1H\e[2J");
    strcat(text,NLINE);
    strcat(text,"\n");
    strcat(text,HLINE);
    strcat(text,"\n");

    // printf("%s\n",NLINE);
    // printf("%s\n",HLINE);
    for (j = 0; j < NUM_RANGE; j++)
    {
        strcat(text,VLINE);
        strcat(text,"\n");

        char num[50];
        sprintf(num,"%d",j+1);

        strcat(text,num);
        strcat(text," ");

        // printf("%s\n",VLINE);
        // printf("%d ",j+1);

        for (k = 0; k < NUM_RANGE; k++)
        {  
            if(strcmp(print[k][j],"              ")==0){
                strcat(text,"| ");
                strcat(text,print[k][j]);
                //printf("| %s",print[k][j]);
            }else{
                strcat(text,"| ");
                strcat(text,print[k][j]);
                strcat(text,"  ");
                //printf("| %s  ",print[k][j]);
            }
        }
        strcat(text,"|\n");
        // printf("%s","|");
        // printf("\n");
        strcat(text,VLINE);
        strcat(text,"\n");
        //printf("%s\n",VLINE);
        strcat(text,HLINE);
        strcat(text,"\n");
        //printf("%s\n",HLINE);
    }
    strcat(text,"Active Clients: ");
    char amount[60];
    sprintf(amount,"%d",count);
    strcat(text,amount);
    
    strcpy(buffer,text);
    send(connection , buffer , strlen(text)+1 , 0 ); 
    //memset(buffer,0,strlen(buffer));
    memset(text,0,strlen(text));
    return;
}

//Builds the board that was passed.
void drawBoard2(){
    char text[BUFFER_SIZE];
    int k,j;
    char * const NLINE = "          A               B               C               D               E               F               G               H               I";
    char * const HLINE = "  +---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+---------------+";
    char * const VLINE = "  |               |               |               |               |               |               |               |               |               |";

    strcat(text,"\e[1;1H\e[2J");
    strcat(text,NLINE);
    strcat(text,"\n");
    strcat(text,HLINE);
    strcat(text,"\n");

    // printf("%s\n",NLINE);
    // printf("%s\n",HLINE);
    for (j = 0; j < NUM_RANGE; j++)
    {
        strcat(text,VLINE);
        strcat(text,"\n");

        char num[50];
        sprintf(num,"%d",j+1);

        strcat(text,num);
        strcat(text," ");

        // printf("%s\n",VLINE);
        // printf("%d ",j+1);

        for (k = 0; k < NUM_RANGE; k++)
        {  
            if(strcmp(print[k][j],"              ")==0){
                strcat(text,"| ");
                strcat(text,print[k][j]);
                //printf("| %s",print[k][j]);
            }else{
                strcat(text,"| ");
                strcat(text,print[k][j]);
                strcat(text,"  ");
                //printf("| %s  ",print[k][j]);
            }
        }
        strcat(text,"|\n");
        // printf("%s","|");
        // printf("\n");
        strcat(text,VLINE);
        strcat(text,"\n");
        //printf("%s\n",VLINE);
        strcat(text,HLINE);
        strcat(text,"\n");
        //printf("%s\n",HLINE);
    }
    strcat(text,"Active Clients: ");
    char amount[60];
    sprintf(amount,"%d",count);
    strcat(text,amount);
    strcpy(buffer,text);

    //memset(buffer,0,strlen(buffer));
    memset(text,0,strlen(text));
    return;
}

/*Main Program Execution. Prompts for the cell and the data to store in the cell.
Appropriately place the given input. */
void makeEntry(char str[]){
    char *formula;
    char *cell = strtok(str,"|");
    char *input = strtok(NULL,"|");

    formula = isFormula(input);

    if(isNumeric(input)){
        setType(cell,"N");
        char *data = malloc(strlen(input)*sizeof(char));
        for(int i=0;i<strlen(input);i++){
            *(data + i) = input[i];
        }
        setCell(cell,data);
        char *newstr = malloc(12*sizeof(char));
        int size = strlen(input);
        if(size>=9){
            for(int i=0;i<9;i++){
                *(newstr + i) = input[i];
            }
            *(newstr + 9) = '.';
            *(newstr + 10) = '.';
            *(newstr + 11) = '.';
        }else{
            for(int i=0;i<12;i++){
                if(i >= size){
                    *(newstr + i) = ' ';
                }else{
                    *(newstr + i) = input[i];
                }
            }
        }
        setPrint(cell,newstr);
    }
    
    else if(stringCmp(formula,"no" ) || stringCmp(formula,"#NAME?")){
        char *data = malloc(strlen(input)*sizeof(char));
        for(int i=0;i<strlen(input);i++){
            *(data + i) = input[i];
        }
        setCell(cell,data);
        setType(cell,"T");
        char *newstr = malloc(12*sizeof(char));
        int size = strlen(input);
        if(size>=9){
            for(int i=0;i<9;i++){
                *(newstr + i) = input[i];
            }
            *(newstr + 9) = '.';
            *(newstr + 10) = '.';
            *(newstr + 11) = '.';
        }else{
            for(int i=0;i<12;i++){
                if(i >= size){
                    *(newstr + i) = ' ';
                }else{
                    *(newstr + i) = input[i];
                }
            }
        }
        setPrint(cell,newstr);
    }

    else{
        setType(cell,"N");
        char lower[] = {formula[2],formula[3],'\0'};
        char upper[] = {formula[5],formula[6],'\0'};
        char str[100];
        if(formula[0] == 'A'){
            sprintf(str,"%.4f",average(lower,upper));
        } 
        else if(formula[0] == 'R'){
            sprintf(str,"%.4f",range(lower,upper));
        }
        else{
            sprintf(str,"%.4f",sum(lower,upper));
        }

        char *data = malloc(strlen(str)*sizeof(char));
        for(int i=0;i<strlen(str);i++){
            *(data + i) = str[i];
        }

        puts(str);
        setCell(cell,data);

        char *newstr = malloc(12*sizeof(char));
        int size = strlen(str);
        if(size>9){
            for(int i=0;i<9;i++){
                *(newstr + i) = str[i];
            }
            *(newstr + 9) = '.';
            *(newstr + 10) = '.';
            *(newstr + 11) = '.';
        }else{
            for(int i=0;i<12;i++){
                if(i >= size){
                    *(newstr + i) = ' ';
                }else{
                    *(newstr + i) = str[i];
                }
            }
        }
        setPrint(cell,newstr);
        puts(newstr);
    }
}


//Store the given input into the given print cell. Returns void.
void setPrint(char *cell, char *input){
    char *ptr = strchr(letters,toupper(cell[0]));
    int column = ptr - letters;

    char *ptr2 = strchr(numbers,cell[1]);
    int row = ptr2 - numbers;
    print[column][row] = input;
}

//Store the given input into the given cell. Returns void.
void setCell(char *cell, char *input){
    char *ptr = strchr(letters,toupper(cell[0]));
    int column = ptr - letters;

    char *ptr2 = strchr(numbers,cell[1]);
    int row = ptr2 - numbers;
    grid[column][row] = input;
}

//Store the given input type into a reference location equivalent to the cell. Returns void.
void setType(char *cell, char *input){
    char *ptr = strchr(letters,toupper(cell[0]));
    int column = ptr - letters;

    char *ptr2 = strchr(numbers,cell[1]);
    int row = ptr2 - numbers;
    type[column][row] = input;
}

//Returns the value in the given cell.
char* getValue(char *cell){
    char *ptr = strchr(letters,toupper(cell[0]));
    int column = ptr - letters;

    char *ptr2 = strchr(numbers,cell[1]);
    int row = ptr2 - numbers;
    return grid[column][row];
}

//Returns the type of data in the given cell.
char* getType(char *cell){
    char *ptr = strchr(letters,toupper(cell[0]));
    int column = ptr - letters;

    char *ptr2 = strchr(numbers,cell[1]);
    int row = ptr2 - numbers;
    return type[column][row];
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

/*Returns true if the given range is valid, false otherwise.
cell_1 - Lower Bound.
cell_2 - Upper Bound.*/
bool isValidRange(char* cell_1, char* cell_2){

    if(!isOnBoard(cell_1) || !isOnBoard(cell_2)){
        return false;
    }

    if(toupper(cell_1[0]) != toupper(cell_2[0]) && cell_1[1] != cell_2[1]){ // Nothing is similar. eg A9:E8
        return false;
    }

    if(toupper(cell_1[0]) == toupper(cell_2[0])){ //Same column, different numbers
        if(cell_1[1] > cell_2[1]){ //eg trying to work a formula on the range A5:A2 is not possible
            return false;
        }
    }

    if(cell_1[1] == cell_2[1]){ //Same row, different letters
        if(toupper(cell_1[0]) > toupper(cell_2[0])){ //eg trying to work a formula on the range E1:A1 is not possible
            return false;
        }
    }
    return true;
}

//Returns true if the passed string is numeric, false otherwise.
bool isNumeric(char* input){
    int i;
    if(strlen(input)==1 && input[0] == '.'){
        return false;
    }

    for(i=0;i<strlen(input);i++){
        if(isdigit(input[i]) == 0){ // zero means not a digit
            if(input[i] != '.'){
                return false;
            }else{
                break;
            }
        }
    }

    for(int j=i+1;j<strlen(input);j++){
        if(isdigit(input[j]) == 0){
            return false;
        }
    }
    return true;
}

/*Checks if the string passed is a formula. Returns "no" if it is not.
"#NAME? if it is a formula but not present within the system or of invalid format or range.
A letter symbolizing the type of formula followed by a space then the lower bound, another space then the uppper bound if it is a formula present within the system and of valid range.
E.g: A A2 A5 - Symbolized that the string passed is to find the average of the cells from A2 to A5.
A - Average
R - Range
S - Sum*/
char* isFormula(char* input){
    char *avg = "=AVERAGE(";
    char *range = "=RANGE(";
    char *sum = "=SUM(";

    if(input[0] != '='){
        return "no";
    }

    if(strlen(input) == 15){  //AVERAGE
        for(int i=0;i<9;i++){
            if(toupper(input[i]) != avg[i]){
                return "#NAME?";
            }
        }

        if(strchr(letters,toupper(input[9])) == NULL || strchr(numbers,input[10]) == NULL){
            return "#NAME?";
        }

        if(input[11] != ','){
            return "#NAME?";
        }

        if(strchr(letters,toupper(input[12])) == NULL || strchr(numbers,input[13]) == NULL){
            return "#NAME?";
        }

        if(input[14] != ')'){
            return "#NAME?";
        }

        if(!isValidRange((char[]){input[9],input[10],'\0'}, (char[]){input[12],input[13],'\0'})){
            return "#NAME?";
        }

        char* formula = malloc(8);
        formula[0] = 'A';
        formula[1] = ' ';
        formula[2] = toupper(input[9]);
        formula[3] = input[10];
        formula[4] = ' ';
        formula[5] = toupper(input[12]);
        formula[6] = input[13];
        formula[7] = '\0';
        return formula;
    }

    if(strlen(input) == 11){ //Sum
        for(int i=0;i<5;i++){
            if(toupper(input[i]) != sum[i]){
                return "#NAME?";
            }
        }

        if(strchr(letters,toupper(input[5])) == NULL || strchr(numbers,input[6]) == NULL){
            return "#NAME?";
        }

        if(input[7] != ','){
            return "#NAME?";
        }

        if(strchr(letters,toupper(input[8])) == NULL || strchr(numbers,input[9]) == NULL){
            return "#NAME?";
        }

        if(input[10] != ')'){
            return "#NAME?";
        }

        if(!isValidRange((char[]){input[5],input[6],'\0'}, (char[]){input[8],input[9],'\0'})){
            return "#NAME?";
        }

        char* formula = malloc(8);
        formula[0] = 'S';
        formula[1] = ' ';
        formula[2] = toupper(input[5]);
        formula[3] = input[6];
        formula[4] = ' ';
        formula[5] = toupper(input[8]);
        formula[6] = input[9];
        formula[7] = '\0';
        return formula;
    }

    if(strlen(input) == 13){ //Range
        for(int i=0;i<7;i++){
            if(toupper(input[i]) != range[i]){
                return "#NAME?";
            }
        }

        if(strchr(letters,toupper(input[7])) == NULL || strchr(numbers,input[8]) == NULL){
            return "#NAME?";
        }

        if(input[9] != ','){
            return "#NAME?";
        }

        if(strchr(letters,toupper(input[10])) == NULL || strchr(numbers,input[11]) == NULL){
            return "#NAME?";
        }


        if(input[12] != ')'){
            return "#NAME?";
        }

        if(!isValidRange((char[]){input[7],input[8],'\0'}, (char[]){input[10],input[11],'\0'})){
            return "#NAME?";
        }

        char* formula = malloc(8);
        formula[0] = 'R';
        formula[1] = ' ';
        formula[2] = toupper(input[7]);
        formula[3] = input[8];
        formula[4] = ' ';
        formula[5] = toupper(input[10]);
        formula[6] = input[11];
        formula[7] = '\0';
        return formula;
    }
    return "#NAME?";
}

/*Returns true if two strings are the same,ignoring case, false otherwise.*/
bool stringCmp(char *str1, char *str2){
    if(strlen(str1) != strlen(str2)){
        return false;
    }

    for(int i=0;i<strlen(str1);i++){
        if(tolower(str1[0]) != tolower(str2[0])){
            return false;
        }
    }
    return true;
}

//Returns the average of the cells in the given range.
float average(char *cell_1, char *cell_2){
    float avg = 0.0;
    int n;
    if(cell_1[0] == cell_2[0]){
        char lower[] = {cell_1[1],'\0'};
        char upper[] = {cell_2[1],'\0'};
        int start = atoi(lower);
        int end = atoi(upper);
        n = 0;
        for(int x = start;x<=end;x++){
            char cell[] = {cell_1[0],x+'0','\0'};
            char *new = getValue(cell);
            char *dataType = getType(cell);
            if(stringCmp(dataType,"N")){
                avg += atof(new);
                n++;
            }
        }
        if(n==0){
            return 0.0;
        }
        avg = avg/n;
    }else{        
        int n = (int)toupper(cell_2[0]) - (int)toupper(cell_1[0]) + 1;
        int div = 0;
        for(int x = 0;x<n;x++){
            char cell[] = {toupper(cell_1[0])+x,cell_1[1],'\0'};
            char *new = getValue(cell);
            char *dataType = getType(cell);
            if(stringCmp(dataType,"N")){
                avg += atof(new);
                div++;
            }
        }
        if(n==0){
            return 0.0;
        }
        avg = avg/div;
    }
    return avg;
}

//Returns the sum of the cells in the given range.
float sum(char *cell_1, char *cell_2){
    float sum = 0.0;
    if(cell_1[0] == cell_2[0]){
        char lower[] = {cell_1[1],'\0'};
        char upper[] = {cell_2[1],'\0'};
        int start = atoi(lower);
        int end = atoi(upper);
        for(int x = start;x<=end;x++){
            char cell[] = {cell_1[0],x+'0','\0'};
            char *new = getValue(cell);
            char *dataType = getType(cell);
            if(stringCmp(dataType,"N")){
                sum += atof(new);
            }
        }
    }else{        
        int n = (int)toupper(cell_2[0]) - (int)toupper(cell_1[0]) + 1;
        for(int x = 0;x<n;x++){
            char cell[] = {toupper(cell_1[0])+x,cell_1[1],'\0'};
            char *new = getValue(cell);
            char *dataType = getType(cell);
            if(stringCmp(dataType,"N")){
                sum += atof(new);
            }
        }
    }
    return sum;
}

//Returns the range of the cells in the given range.
float range(char *cell_1, char *cell_2){
    float nums[9];
    int point = 0; 
    if(cell_1[0] == cell_2[0]){
        char lower[] = {cell_1[1],'\0'};
        char upper[] = {cell_2[1],'\0'};
        int start = atoi(lower);
        int end = atoi(upper);
        for(int x = start;x<=end;x++){
            char cell[] = {cell_1[0],x+'0','\0'};
            char *new = getValue(cell);
            char *dataType = getType(cell);
            if(stringCmp(dataType,"N")){
                nums[point]= atof(new);
                point++;
            }
        }
    }else{        
        int n = (int)toupper(cell_2[0]) - (int)toupper(cell_1[0]) + 1;
        for(int x = 0;x<n;x++){
            char cell[] = {toupper(cell_1[0])+x,cell_1[1],'\0'};
            char *new = getValue(cell);
            char *dataType = getType(cell);
            if(stringCmp(dataType,"N")){
                nums[point]= atof(new);
                point++;
            }
        }
    }    
    if(point ==0){
        return 0.0;
    }

    float high = nums[0];
    float low = nums[0];

    for(int j=1;j<point;j++){
        if(nums[j]>high){
            high=nums[j];
        }
        if(nums[j]<low){
            low=nums[j];
        }
    }
    return high-low;
}