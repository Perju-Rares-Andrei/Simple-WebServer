
//Simple web server 

#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <pthread.h>

 // Portul standard HTTP pentru serverul web

#define SERVER_PORT 8080     

#define MAXLINE 4096

#define SA struct sockaddr

extern int errno;

void *raspuns(void *pclient);

int main(int argc , char **argv)
{
    int listenfd, client, n;

    // Structura serverului
    struct sockaddr_in servaddr; 

    uint8_t buff[MAXLINE+1];
    uint8_t recvline[MAXLINE+1];

     //Crearea unui socket

    if((listenfd=socket(AF_INET, SOCK_STREAM, 0))<0)        
       {
        perror ("[server]Eroare la socket().\n");
        return errno;
       }
    
    // Initializarea structuri de date a serverului
    bzero(&servaddr,sizeof(servaddr)); 

    // Stabilirea familiei de socket-uri
    servaddr.sin_family     =AF_INET;    

    // Acceptarea oricarei adrese
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);  

    // Uilizam un port utilizator
    servaddr.sin_port       =htons(SERVER_PORT);    

     //Atasam socketul
    if((bind(listenfd,(SA *) &servaddr , sizeof(servaddr)))<0)      
        {
        perror ("[server]Eroare la bind().\n"); 
        return errno;
        }                    

    // Punem serveul sa asculte daca vin clientii/utilizatorii sa se conecteze    
    if((listen(listenfd,10))<0)   
    {
        perror ("[server]Eroare la listen().\n");
        return errno;
    }

    // Servim in mod concurent clientii folosind thread-uri
    while(1) 
    {
        //int i=1;
        struct sockaddr_in addr;
        socklen_t addr_len;
        char client_addr[MAXLINE+1];


        printf("\nAsteptam conexiunea la portul %d\n\n", SERVER_PORT);
        fflush(stdout);

        // Asteptam un client intr-o stare blocanta pana acesta se conecteaza 
        client=accept(listenfd, (SA *) &addr, &addr_len);       

        inet_ntop(AF_INET,&addr,client_addr,MAXLINE);
        printf("Client connection: %s\n",client_addr);

        memset(recvline,0,MAXLINE);

        pthread_t t;
        int *pclient= malloc(sizeof(client+1));
        *pclient = client;
        pthread_create(&t , NULL , raspuns ,pclient);

        //printf("thread-ul cu numarul :");
        //printf("%d",i);
        //i++;
        //sleep(5);
    }
}

void *raspuns(void *pclient)
{       
        int client= *(int*)pclient;
        free(pclient);
        int n;
        uint8_t buff[MAXLINE+1];
        uint8_t recvline[MAXLINE+1];

        struct sockaddr_in addr;
        socklen_t addr_len;
        char client_addr[MAXLINE+1];

        memset(recvline,0,MAXLINE);

        // Afisam date relevante cu privire la cllinet 
        while((n=read(client,recvline,MAXLINE-1))>0)       
        {
           fprintf(stdout,"\n%s\n",recvline);
           if(recvline[n-1]=='\n')
           {
               break;
           }
           memset(recvline,0,MAXLINE);
        }
        if(n<0)
            perror ("[server]Eroare la read().\n");

        FILE *fp;
        long lSize;
        char *buffer;
        
        // pregatirea fisierului text pentru a fi afisat in browser

        fp = fopen ( "index.txt" , "rb" );
        if( !fp ) perror("index.txt"),exit(1);

        fseek( fp , 0L , SEEK_END);
        lSize = ftell( fp );
        rewind( fp );

        /* aloc memorie pentru continut*/
        buffer = calloc( 1, lSize+1 );
        if( !buffer ) fclose(fp),fputs("alocarea memoriei a esuat",stderr),exit(1);

        /* copiez fisierul in buffer*/
        if( 1!=fread( buffer , lSize, 1 , fp) )
        fclose(fp),free(buffer),fputs("toata citirea a esuat",stderr),exit(1);

        fclose(fp);


        //char message[] = "HTTP/1.1 200 Okay\r\nContent-Type: text/html; charset=ISO-8859-4 \r\n\r\n<h1>Hello, client! Welcome to the Virtual Machine Web..</h1><h2>poate iese asa</h2>";
        int length = strlen(buffer); // Plus 1 for  null terminator
        int send_res = send(client, buffer, length, 0); // Flag = 0

        free(buffer);

        //snprintf((char*)buff,sizeof(buff),"http/1.0 200 OK\r\n\r\n",s);
       
        write(client,(char*)buff,strlen((char *)buff));
    
        close(client);
        return NULL;
}