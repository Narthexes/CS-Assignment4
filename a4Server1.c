#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
/*

    CONCURRENT SERVER: THREAD EXAMPLE
    Must be linked with the "pthread" library also, e.g.:
       cc -o example example.c -lnsl -lsocket -lpthread 

    This program creates a connection socket, binds a name to it, then
    listens for connections to the sockect.  When a connection is made,
    it accepts messages from the socket until eof, and then waits for
    another connection...

    This is an example of a CONCURRENT server -- by creating threads several
    clients can be served at the same time...

    This program has to be killed to terminate, or alternately it will abort in
    120 seconds on an alarm...
*/
char sharedMessage[2000];
long counter = 1;

static int callback(void *data, int argc, char **argv, char **azColName){
      int i;
      for(i=0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        strcat(sharedMessage, azColName[i]);
        strcat(sharedMessage, " ");
        strcat(sharedMessage, argv[i] ? argv[i] : "NULL");
        strcat(sharedMessage, "\n");
      }
      printf("\n");
      strcat(sharedMessage, "\n");
      return 0;
    }
    
    
struct serverParm {
           int connectionDesc;
       };

void *serverThread(void *parmPtr) {
    FILE *fp;
    fp = fopen("a4ServerLog.txt", "r");
  
    long threadNum = counter;
    counter++;
    
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    char *sql;
    const char* data = "Callback function called";
    
    /* Open database */
    rc = sqlite3_open("test.db", &db);
    if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
     exit(0);
    }else{
      fprintf(stderr, "Opened database successfully\n");
    }
    
    #define PARMPTR ((struct serverParm *) parmPtr)
    int recievedMsgLen;
    char messageBuf[1025];

    /* Server thread code to deal with message processing */
    printf("DEBUG: connection made, connectionDesc=%d\n",
            PARMPTR->connectionDesc);
    if (PARMPTR->connectionDesc < 0) {
        printf("Accept failed\n");
        return(0);    /* Exit thread */
    }
    
    /* Receive messages from sender... */
    while ((recievedMsgLen=
            read(PARMPTR->connectionDesc,messageBuf,sizeof(messageBuf)-1)) > 0) 
    {
        recievedMsgLen[messageBuf] = '\0';
        printf("Message: %s\n",messageBuf);
        //Does the SQL call
        sql = messageBuf;
        
        /*SQL call*/
        rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
        if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        }else{
        fprintf(stdout, "Operation done successfully\n");
        }
        
        time_t rawtime;
        struct tm * timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        
        printf("Thread %ld in process %ld did %s at time %s\n", threadNum, getpid(), sql, asctime(timeinfo));
   
        
        /*Sends the client a message*/
        if (write(PARMPTR->connectionDesc,sharedMessage,2000) < 0) {
               perror("Server: write error");
               return(0);
        }

    }
    close(PARMPTR->connectionDesc);  /* Avoid descriptor leaks */
    free(PARMPTR);                   /* And memory leaks */
    sqlite3_close(db);
    fclose(fp);
    return(0);                       /* Exit thread */
}

main(int argc, char **argv) {
    strcpy(sharedMessage, "");
    int listenDesc;
    struct sockaddr_in myAddr;
    struct serverParm *parmPtr;
    int connectionDesc;
    pthread_t threadID;
    
    /* For testing purposes, make sure process will terminate eventually */
    alarm(120);  /* Terminate in 120 seconds */

    /* Create socket from which to read */
    if ((listenDesc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("open error on socket");
        exit(1);
    }

    /* Create "name" of socket */
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = INADDR_ANY;
    myAddr.sin_port = htons(atoi(argv[1]));
        
    if (bind(listenDesc, (struct sockaddr *) &myAddr, sizeof(myAddr)) < 0) {
        perror("bind error");
        exit(1);
    }

    /* Start accepting connections.... */
    /* Up to 5 requests for connections can be queued... */
    listen(listenDesc,5);

    while (1) /* Do forever */ {
        /* Wait for a client connection */
        connectionDesc = accept(listenDesc, NULL, NULL);

        /* Create a thread to actually handle this client */
        parmPtr = (struct serverParm *)malloc(sizeof(struct serverParm));
        parmPtr->connectionDesc = connectionDesc;
        if (pthread_create(&threadID, NULL, serverThread, (void *)parmPtr) 
              != 0) {
            perror("Thread create error");
            close(connectionDesc);
            close(listenDesc);
            exit(1);
        }

        printf("Parent ready for another connection\n");
    }
}