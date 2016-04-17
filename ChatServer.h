#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <sys/types.h>
#include <ifaddrs.h>
#include <sstream>
#include <time.h>
#include <queue>
#include <chrono>
#include <vector>
#include <mutex>

#define CHATUSERS_COUNT 10

#define NOTIFICATION 1
#define DATA 2
#define INTERNAL 3
#define ACK 4
#define ackport 6000
#define LE_Reply 5
#define LE_PortSend 6
#define LE_NewSeqMessage 7
#define RECVD 8

void error(const char *msg)
{
    perror(msg);
    exit(-1);
}
using namespace std;

class Message
{
public:
    int messageType;
    char mess[256];
    char name[32];
    char ip[32];
    int port;
    int hbport;
	int sendport;
    unsigned long int seqNum;
	unsigned long int noteNum;
};

class ChatUser
{
public:
    
    char name[32];
    int UIRI;
    int portNumber;
    int hbportNumber;
	int sendportNumber;
    char ipAddr[32];
    
    bool isSequencer=false;
    int leaderPortNum;
    char seqIpAddr[32];
   
    int leaderElectionPort;
	
	sockaddr_in multicastSockAddr;
	sockaddr_in hbSockAddr;
	
	int clientType=0;
};


struct Messageobj {
    Message newmsg;
    ChatUser CUser;
    
};

struct sendPar{
	sockaddr_in info;
	Messageobj m;
};


//Variables for client_Bind
struct sockaddr_in si_user;
socklen_t newJoinUser_slen=sizeof(si_user);

//variables for client hb
struct sockaddr_in si_hb;
socklen_t si_hb_slen=sizeof(si_hb);

//Variable for seq ack
struct sockaddr_in si_ack;
socklen_t ack_slen=sizeof(si_ack);

//Client ack
struct sockaddr_in si_ackclient;
socklen_t newJoinUserack_slen=sizeof(si_ackclient);

//send sequencer
struct sockaddr_in si_send;
int sendsock;
socklen_t send_slen=sizeof(si_send);

//variables for Leader Election
int socket_leaderElection;
struct sockaddr_in si_leaderElection;
socklen_t si_leaderElection_slen=sizeof(si_leaderElection);

int sock, sockack, client_ack;

int newUser_s, newUser_i, client_s;

struct sockaddr_in newUser_si_other;
socklen_t newUser_slen=sizeof(newUser_si_other);

struct sockaddr_in si_me, si_other;
socklen_t slen=sizeof(si_other);

void *SequencerVanished();
void *receiver_handler(void *);
void *sender_handler(void *);
void *seq_receiver_handler(void *);
void *seq_mess_sender_handler(void *);
void *printMessages(Messageobj newMessage,bool recvFlag);
void *addToMultiCastDS(Messageobj newIncomingMessage, sockaddr_in si_other);
void *multicast(Messageobj newMessage);
void *removefrommulticast(int client_remove, string name);
void *seq_indirect_joining_handler(void *);
void *client_ack_handler(void *);
void *seq_ack_handler(void *);
void *removeackclient(int client_remove);
void *leaderElection_handler(void *);
void *SendNewSeqMessageToClient(ChatUser initSeq);
void *InitiateReconnect(Messageobj newSeq);
void *seq_send(void* sendd);
string ToString(int val);
void *android_interface_receiver_handler(void *);

string getIP()
{
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);
    char key[]="em1";
    string ip;
    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) { 
           
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

            if(strcmp(ifa->ifa_name,"em1") == 0) {
                ip=addressBuffer;
            }
        } else if (ifa->ifa_addr->sa_family == AF_INET6) { 
            
            tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
        }
    }
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
    return ip;
}

sockaddr_in create_sock_struct(Messageobj newMessage)
{
    struct sockaddr_in si_ackcli;
    si_ackcli.sin_family = AF_INET;
    si_ackcli.sin_port = htons(newMessage.newmsg.hbport);

    if (inet_aton(newMessage.newmsg.ip, &si_ackcli.sin_addr) == 0)  {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
    return si_ackcli;
}

string ToString(int val)
{
    char message[1024];
    strcpy(message,"");
    char snum[5];
    sprintf(snum, "%d", val);
    strcpy(message,snum);
    string toReturn(message);
    return toReturn;

}
