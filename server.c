#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
 
#define BUFLEN 512  //Max length of buffer
#define SEPARATOR "|"

int SEQ_NUM;
int ACK_NUM;
char SYN_BIT[2];
char ACK_BIT[2];
int WINDOW_SIZE;

char header[100];

int fd;
struct addrinfo* res;
struct sockaddr_in address;
socklen_t size;

void addHeader(int seq_num, int ack_num, char syn_bit[2], char ack_bit[2], int window_size){    
    char sn[100], an[100], ws[100];

    sprintf(sn, "%d", seq_num);
    sprintf(an, "%d", ack_num);
    sprintf(ws, "%d", window_size);

    strcpy(header, sn);
    strcat(header, SEPARATOR);
    strcat(header, an);
    strcat(header, SEPARATOR);
    strcat(header, syn_bit);
    strcat(header, SEPARATOR);
    strcat(header, ack_bit);
    strcat(header, SEPARATOR);
    strcat(header, ws);
    strcat(header, SEPARATOR);
}

char *parseHeader(char *message){
    int i;
    char temp[20];
    char *pch;

    pch = strtok (message, SEPARATOR);
    for (i = 0; pch != NULL; i++){
        strcpy(temp, pch);
        switch(i){
            case 0:
                SEQ_NUM = atoi(temp);
                break;
            case 1: 
                ACK_NUM = atoi(temp);
                break;
            case 2:
                SYN_BIT[0] = temp[0];
                break;
            case 3:
                ACK_BIT[0] = temp[0];
                break;
            case 4:
                WINDOW_SIZE = atoi(temp);
                break;
            case 5:
                return pch;
        }
        pch = strtok (NULL, SEPARATOR);
    }
}
 
void die(char *s)
{
    perror(s);
    exit(1);
}
 
int main(int argc, char *argv[])
{
    struct sockaddr_in si_me, si_other;
     
    int s, i, slen = sizeof(si_other) , numbytes;
    char buf[BUFLEN];
    char *message;
     
    if (argc != 2) {
        fprintf(stderr,"usage: %s <port>\n", argv[0]);
        exit(1);
    }

    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(atoi(argv[1]));
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket to port
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }
     
    //receive data
    if ((numbytes = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1) die("recvfrom()");
    parseHeader(buf);

    //packet #2
    if (strcmp(SYN_BIT, "1") == 0){
        addHeader(0,1,"1","1", 100);
        strcpy(buf, header);
        if (sendto(s, buf, strlen(buf), 0, (struct sockaddr*) &si_other, slen) == -1) die("sendto()");
    }

    if ((numbytes = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1) die("recvfrom()");

    if ((numbytes = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1) die("recvfrom()");
    message = parseHeader(buf);
    printf("server: received\n%s\n", message);

    //packet #5
    addHeader(0,1,"1","1", 100);
    strcpy(buf, header);
    strcat(buf, "Hello!");
    if (sendto(s, buf, strlen(buf), 0, (struct sockaddr*) &si_other, slen) == -1) die("sendto()");
 
    close(s);
    return 0;
}