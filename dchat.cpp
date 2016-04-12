/**
Description: CIS 505 Project 3
Authors: Karthik Anantha Ram, Sanjeet Phatak and Sarath Vadakkepat
**/

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
#include<chrono>
#include <mutex>

#define NOTIFICATION 1
#define DATA 2
#define INTERNAL 3
#define ACK 4
#define ackport 6000
#define LE_Reply 5
#define LE_PortSend 6


//Method to print error messages
void error(const char *msg)
{
    perror(msg);
    exit(-1);
}

#include "ChatServer.h"


using namespace std;

int sock;
int sockack;
int flag=0;
unsigned long int seq=0;
unsigned long int seqChk=1;

int seqNext=0;
mutex mPrint;
class Message
{
public:
    int messageType;
    char mess[1024];
    char name[100];
    char ip[100];
    int port;
    int hbport;
    unsigned long int seqNum;
};

//Variables if a client
struct sockaddr_in newUser_si_other;
int newUser_s, newUser_i;
int client_s;
socklen_t newUser_slen=sizeof(newUser_si_other);
socklen_t slen;

struct sockaddr_in si_me, si_other;

//Variables for sequencer

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

//variables for Leader Election
int socket_leaderElection;
struct sockaddr_in si_leaderElection;
socklen_t si_leaderElection_slen=sizeof(si_leaderElection);


string welcome_mess;



char client_name[50];



int lifecheck;

class ChatUser
{
public:
	
	char name[32];
    //string name;
    int UIRI;
    int portNumber;
    int hbportNumber;
    char ipAddr[32];
	
	//string ipAddr;
    bool isSequencer=false;
    int leaderPortNum;
   char seqIpAddr[32];
	
	int leaderElectionPort;
};

struct Messageobj {
    Message newmsg;
	ChatUser CUser;
	
} msg;

struct Messageobj mess1;

queue<Messageobj> msgOrder;
ChatUser initSeq;
ChatUser newUser;

ChatUser currentUser;

struct UserInGroup {
    char name[32];
    char ip[32];
    int port;
    int hbport;
    int UIRI;
	
	//int LE_Port;

};
struct UserInGroup usersInGroup[10];
int usersInGroupCtr=0;

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


string ToString(int val);
int check=0;
sockaddr_in clientList[10];
sockaddr_in hbList[10];
int hbcount=0;
int clientListCtr=0;
int check_client=0;

int leaderElectionCount;

//Class for users in a group chat
struct ChatUser usrInGrpClientRecord[10];
int usrInGrpClientRecordCtr=0;

struct ChatUser usrInGrpSeqRecord[10];
int usrInGrpSeqRecordCtr=0;

void testPrint()
{
	cout<<"Initiating Test Print "<<endl;
  cout<<usrInGrpClientRecordCtr<<endl;
	
	for(int i=0;i<usrInGrpClientRecordCtr;i++)
	{
		
		ChatUser temp=usrInGrpClientRecord[i];
		cout<<"Name = " <<temp.name<<"Port Num = "<<temp.portNumber <<"Leader Election port = "<<temp.leaderElectionPort<<endl;
	}
}




//Method to enable a user join a existing chat
void existGrpChat(ChatUser newUser)
{
	currentUser=newUser;

    strcpy(client_name, newUser.name);

    if ( (newUser_s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        error("socket");
    }

    memset((char *) &newUser_si_other, 0, sizeof(newUser_si_other));
    newUser_si_other.sin_family = AF_INET;
    newUser_si_other.sin_port = htons(newUser.leaderPortNum);

    memset((char *) &si_user, 0, sizeof(si_user));
    si_user.sin_family = AF_INET;
    si_user.sin_port = htons(newUser.portNumber);
    si_user.sin_addr.s_addr = htonl(INADDR_ANY);

    struct timeval tv;
    tv.tv_sec       = 1;
    tv.tv_usec      = 1000;

    if (setsockopt(newUser_s, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
        perror("Error");
    }

    if( bind(newUser_s , (struct sockaddr*)&si_user, sizeof(si_user) ) == -1) {
        error("bind");
    }

    //cout<<"seq ip "<<newUser.seqIpAddr<<endl;
    if (inet_aton(newUser.seqIpAddr, &newUser_si_other.sin_addr) == 0)  {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    if ( (client_s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        error("socket");
    }

    memset((char *) &si_ackclient, 0, sizeof(si_ackclient));
    si_ackclient.sin_family = AF_INET;
    si_ackclient.sin_port = htons(6000);


    if (inet_aton(newUser.seqIpAddr, &si_ackclient.sin_addr) == 0)  {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }


    memset((char *) &si_hb, 0, sizeof(si_hb));
    si_hb.sin_family = AF_INET;
    si_hb.sin_port = htons(newUser.hbportNumber);
    si_hb.sin_addr.s_addr = htonl(INADDR_ANY);

    if( bind(client_s , (struct sockaddr*)&si_hb, sizeof(si_hb) ) == -1) {
        error("bind");
    }

    struct timeval tv1;
    tv1.tv_sec       = 5;
    tv1.tv_usec      = 1000;

    if (setsockopt(client_s, SOL_SOCKET, SO_RCVTIMEO, &tv1, sizeof(tv1)) < 0) {
        perror("Error");
    }

    Message newMessage;
    strcpy(newMessage.mess ,"JOIN");
    strcpy(newMessage.name, newUser.name);
    strcpy(newMessage.ip, newUser.ipAddr);
    newMessage.port=newUser.portNumber;
    newMessage.hbport=newUser.hbportNumber;
    newMessage.messageType=INTERNAL;

    struct Messageobj newMessageObj;
    newMessageObj.newmsg=newMessage;
	newMessageObj.CUser=newUser;

    if (sendto(newUser_s, &newMessageObj, sizeof(struct Messageobj), 0 , (struct sockaddr *) &newUser_si_other, newUser_slen)==-1) {
        error("sendto()");
    }

    struct Messageobj newIncomingMessage;
    //Receiving Welcome Message
    if (recvfrom(newUser_s, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &newUser_si_other, &newUser_slen) == -1) {
        cout<<"Sorry, no chat is active on "<<newUser.seqIpAddr<<":"<<newUser.leaderPortNum<<" try again later.Bye."<<endl;
        exit(-1);
    }

    tv.tv_sec       = 0;
    tv.tv_usec      = 0;

    if (setsockopt(newUser_s, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
        perror("Error");
    }

	
	// Struct and binding for Leader Election
	
	if ( (socket_leaderElection=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        error("socket");
    }
	
	memset((char *) &si_leaderElection, 0, sizeof(si_leaderElection));
    si_leaderElection.sin_family = AF_INET;
    si_leaderElection.sin_port = htons(newUser.leaderElectionPort);
    si_leaderElection.sin_addr.s_addr = htonl(INADDR_ANY);

    if( bind(socket_leaderElection , (struct sockaddr*)&si_leaderElection, sizeof(si_leaderElection) ) == -1) {
        error("bind");
    }
	
	//Strcut for LE complete.
	
	
	
	
    if(strcmp(newIncomingMessage.newmsg.mess,"INDIRECT")==0) {
        string leadIP(newIncomingMessage.newmsg.ip);
        
		strcpy(newUser.seqIpAddr, leadIP.c_str());
		//newUser.seqIpAddr=leadIP;
        newUser.leaderPortNum= newIncomingMessage.newmsg.port;
        close(newUser_s);
        close(client_s);
        existGrpChat(newUser);
    } else {
        cout<<newUser.name<<" joining a new chat on "<<newUser.seqIpAddr<<":"<<newUser.leaderPortNum<<", listening on "<<newUser.ipAddr<<":"<<newUser.portNumber<<endl;
        seqChk=newIncomingMessage.newmsg.seqNum+1;
        mPrint.lock();
        printMessages(newIncomingMessage,false);
        mPrint.unlock();
    }

    //Receiving number of users
    char userCount[32];
    if (recvfrom(newUser_s, &userCount, sizeof(userCount), 0, (struct sockaddr *) &newUser_si_other, &newUser_slen) == -1) {
        error("recvfrom()");
    }


    int number=atoi(userCount);
	cout<<"CP 1"<<endl;
	
	
    for(int i=0; i<number; i++) {


        if (recvfrom(newUser_s, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &newUser_si_other, &newUser_slen) == -1) {
            error("recvfrom()");
        }

        if(strcmp(newIncomingMessage.newmsg.mess, ":"))
			
			usrInGrpClientRecord[i]=newIncomingMessage.CUser;
			usrInGrpClientRecordCtr++;
			
            mPrint.lock();
        printMessages(newIncomingMessage,false);
        mPrint.unlock();
    }


	//testPrint();
	
	
    pthread_t thread_1,thread_2, thread_3, thread_4;
    pthread_create( &thread_1 , NULL , receiver_handler,(void*) 0);
    pthread_create( &thread_2, NULL , sender_handler,(void*) 0);
    pthread_create( &thread_3, NULL , client_ack_handler,(void*) 0);
	pthread_create(&thread_4, NULL, leaderElection_handler, (void*) 0);
	
    //pthread_create( &thread_3, NULL , receiver_handler_newJoin,(void*) 0);

    pthread_join(thread_1,NULL);
    pthread_join(thread_2,NULL);
    pthread_join(thread_3, NULL);
	pthread_join(thread_4, NULL);
    //pthread_join(thread_3,NULL);

    //Replace while with thread join
    while(check==1) {
        return;
    }
}



//Method to initiate sequencer
void newGrpChat(ChatUser initSeq)
{
	currentUser=initSeq;
    
	cout<<initSeq.name<<" started a new chat, listening on "<<initSeq.ipAddr<<":"<<initSeq.portNumber<<endl;
    cout<<"Succeeded, current users:"<<endl;
    cout<<initSeq.name<<" "<<initSeq.ipAddr<<"."<<initSeq.portNumber<<" (Leader)"<<endl;

    strcpy(client_name, initSeq.name);
    char samp[20];

    sprintf(samp, "%d", initSeq.portNumber);
    string sam(samp);

	string t_name(initSeq.name);
	string t_ip(initSeq.ipAddr);
	
    welcome_mess = t_name + " " + t_ip + ":" + sam + "(Leader)";

    cout<<"Waiting for others to join..."<<endl;


    int i, recv_len;
    slen= sizeof(si_other);
    ack_slen= sizeof(si_ack);


    if ((sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        error("socket");
    }

    memset((char *) &si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(5000);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    if( bind(sock , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) {
        error("bind");
    }

    if ((sockack=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        error("socket");
    }

    memset((char *) &si_ack, 0, sizeof(si_ack));

    si_ack.sin_family = AF_INET;
    si_ack.sin_port = htons(6000);
    si_ack.sin_addr.s_addr = htonl(INADDR_ANY);

    if( bind(sockack , (struct sockaddr*)&si_ack, sizeof(si_ack) ) == -1) {
        error("bind");
    }

    struct timeval tv1;
    tv1.tv_sec       = 2;
    tv1.tv_usec      = 1000;

    if (setsockopt(sockack, SOL_SOCKET, SO_RCVTIMEO,&tv1,sizeof(tv1)) < 0) {
        perror("Error");
    }
    /*if (setsockopt(sockack, SOL_SOCKET, SO_SNDTIMEO,&tv1,sizeof(tv1)) < 0) {
    perror("Error");
    }*/
    pthread_t thread_1,thread_2,thread_3,thread_4;
    pthread_create( &thread_1, NULL , seq_receiver_handler,(void*) 0);
    pthread_create( &thread_3, NULL , seq_mess_sender_handler,(void*) 0);
    pthread_create( &thread_4, NULL , seq_ack_handler,(void*) 0);

    pthread_join(thread_1,NULL);
    pthread_join(thread_3,NULL);
    pthread_join(thread_4,NULL);

    return;
}
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
        if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

            if(strcmp(ifa->ifa_name,"em1") == 0) {
                ip=addressBuffer;
            }
        } else if (ifa->ifa_addr->sa_family == AF_INET6) { // check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
        }
    }
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
    return ip;
}


int main(int argc, char* argv[])
{

    srand(time(NULL));
    if(argc==2) {
        initSeq.isSequencer=true;
		
		strcpy(initSeq.ipAddr,getIP().c_str()); 
        //initSeq.ipAddr=getIP().c_str();
        initSeq.portNumber=5000;
		
		string temp_name=argv[1];
        strcpy(initSeq.name, temp_name.c_str()); 
		//initSeq.name=argv[1];
        initSeq.leaderPortNum=initSeq.portNumber;
		
		strcpy(initSeq.seqIpAddr,initSeq.ipAddr); 
        //initSeq.seqIpAddr=initSeq.ipAddr;
        initSeq.UIRI++;

		string temp_ipAddr="192.168.0.102";
		
		strcpy(initSeq.ipAddr,temp_ipAddr.c_str());
		 strcpy(initSeq.seqIpAddr,temp_ipAddr.c_str());
		
        newGrpChat(initSeq);

    } else if(argc==3) {
        //ChatUser newUser;
        newUser.isSequencer=false;
		
		strcpy(initSeq.ipAddr,getIP().c_str()); 
        //newUser.ipAddr=getIP();
        
		newUser.portNumber=1023+rand()%1000;
        // while(newUser.hbportNumber==newUser.portNumber){
        newUser.hbportNumber=newUser.portNumber+1;
        //}
        
		string temp_name=argv[1];
        strcpy(newUser.name, temp_name.c_str()); 
		//newUser.name=argv[1];
        
		newUser.UIRI++;
        string ipPort=argv[2];
        istringstream iss(ipPort);
        string token;

        int cnt=0;
        while (getline(iss, token, ':')) {
            if(cnt==0)
                strcpy(newUser.seqIpAddr,token.c_str());
            else if(cnt==1)
                newUser.leaderPortNum= atoi(token.c_str());
            else
                break;
            cnt++;
        }
		
		newUser.leaderElectionPort=10000+rand()%1000;
		
		string temp_ipAddr="192.168.0.102";
		
		strcpy(newUser.ipAddr,temp_ipAddr.c_str());
		strcpy(newUser.seqIpAddr,temp_ipAddr.c_str());
		
		
        existGrpChat(newUser);
        return 0;
    }


    else {
        cout<<"Error:Invalid arguments"<<endl;
        return -1;
    }


}

void *printMessages(Messageobj newMessage, bool recvFlag)
{

    if(newMessage.newmsg.messageType==DATA) {
        //cout<<"seq "<<newMessage.newmsg.seqNum<<" seqChk "<<seqChk<<endl;

        if(newMessage.newmsg.seqNum==seqChk) {
            cout<<"seq "<<newMessage.newmsg.seqNum<<" "<<newMessage.newmsg.name<<":: "<<newMessage.newmsg.mess<<endl;

            if(recvFlag) {
                msgOrder.pop();
                //recvFlag=false;
            }
            seqChk=seqChk+1;
        } else {
            msgOrder.push(newMessage);
        }

        seqNext=msgOrder.front().newmsg.seqNum;

        if(seqNext==seqChk) {
            //recvFlag=true;
            mPrint.lock();
            printMessages(msgOrder.front(),true);
            mPrint.unlock();
        }
    }
    if(newMessage.newmsg.messageType==NOTIFICATION) {
        cout<<newMessage.newmsg.mess<<endl;
		
		
		istringstream iss(newMessage.newmsg.mess);
        string token;

        int cnt=0;
        while (getline(iss, token, ' ')) {
            if(strcmp(token.c_str(), "joined")==0)
			{
				cout<<"We have a new joineee"<<endl;
				usrInGrpClientRecord[usrInGrpClientRecordCtr]=newMessage.CUser;
	 			   usrInGrpClientRecordCtr++;
				//testPrint();
				break;
			}
        }
		
		
		
    }

}


void *receiver_handler(void *)
{
    struct Messageobj newIncomingMessage;
    while(1) {
        if (recvfrom(newUser_s, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &newUser_si_other, &newUser_slen) == -1) {
            error("recvfrom()");
        }

        string newMessageArrived(newIncomingMessage.newmsg.mess);
        string newMessage;
        Message msgg;
        if(newMessageArrived == "JOIN") {

            Message giveSEQIPData;


            strcpy(giveSEQIPData.mess, "INDIRECT");
            strcpy(giveSEQIPData.ip, newUser.seqIpAddr);
            giveSEQIPData.port=newUser.leaderPortNum;
            giveSEQIPData.messageType=INTERNAL;

            int leaderPortNum;
            string seqIpAddr;

            struct Messageobj newmess;
            newmess.newmsg=giveSEQIPData;

            if (sendto(newUser_s, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &newUser_si_other, newUser_slen)==-1) {
                error("sendto()");
            }
            //cout<<"request forwarded to seq"<<endl;

        }

        else {
            mPrint.lock();
            printMessages(newIncomingMessage,false);
            mPrint.unlock();
        }
    }
}

void *seq_mess_sender_handler(void *)
{
    while(1) {

        string m="";
        char send[1024];
        getline(cin,m);
        strcpy(send, m.c_str());
        Message msgg;
        msgg.messageType=DATA;
        seq=seq+1;
        msgg.seqNum=seq;
        strcpy(msgg.mess, send);
        strcpy(msgg.name, client_name);

        struct Messageobj newmess;
        newmess.newmsg=msgg;
        mPrint.lock();
        printMessages(newmess,false);
        mPrint.unlock();

        for(int i=0; i<clientListCtr; i++) {
            if (sendto(sock, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &clientList[i], newUser_slen)==-1) {
                error("sendto()");
            }
        }
    }
}


void *sender_handler(void *)
{
    while(1) {

        string m="";
        char send[1024];
        Message msgg;
        getline(cin,m);
        if(cin.eof()==1) {
            strcpy(send, "group_leave");
            msgg.messageType=INTERNAL;
            check=1;
        } else {
            strcpy(send, m.c_str());
            msgg.messageType=DATA;
        }


        strcpy(msgg.mess, send);
        strcpy(msgg.name, client_name);

        struct Messageobj newmess;
        newmess.newmsg=msgg;
        if (sendto(newUser_s, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &newUser_si_other, newUser_slen)==-1) {
            error("sendto()");

        }
        if(check==1)
            exit(0);
    }
}

void *seq_ack_handler(void *)
{
    while(1) {


        string m="";
        char send[1024];
        strcpy(send, "ack");
        Message msgg;
        msgg.messageType=ACK;

        struct Messageobj newmess;
        newmess.newmsg=msgg;
        string clientname;
        if(flag==1) {
            for(int i=0; i<clientListCtr; i++) {

                //cout<<"clientListCtr= "<<clientListCtr<<endl;
                if (sendto(sockack, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &hbList[i], si_hb_slen)==-1) {
                    error("sendto()");
                }

                struct sockaddr_in si_ack_other;
                if (recvfrom(sockack, &newmess, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_ackclient, &ack_slen) == -1) {
                    int cnt=0;

                    for(int j=0; j<usersInGroupCtr; j++) {
                        if(usersInGroup[j].hbport==(int)ntohs(hbList[i].sin_port)) {
                            cnt=j;
                            clientname = usersInGroup[j].name;
                        }
                    }
                    int clientid1 = (int)ntohs(clientList[cnt].sin_port);
                    int clientid2 = (int)ntohs(hbList[cnt].sin_port);
                    
                    removefrommulticast(clientid1, clientname);
                    
                    removeackclient(clientid2);
                    cout<<clientListCtr<<endl;
                    for(int j=0; j<clientListCtr; j++) {
                        cout<<"port1:"<<(int)ntohs(hbList[j].sin_port)<<endl;
                        cout<<"port2:"<<(int)ntohs(clientList[j].sin_port)<<endl;
                    }
                }
            }

        }
    }
}

void SimplyPrint(Messageobj messa)
{
			mPrint.lock();
            cout<<messa.newmsg.mess<<endl;
            mPrint.unlock();
	
	
}

void *SequencerVanished()
{
	int leaderElectionSock;

	struct sockaddr_in si_leaderElectionSock;
	socklen_t leaderElectionSock_slen=sizeof(si_leaderElectionSock);
			
	if ( (leaderElectionSock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        		error("socket");
   			 }
			
	
	for(int i=0;i<usrInGrpClientRecordCtr;i++)
	{
		if(usrInGrpClientRecord[i].leaderElectionPort != currentUser.leaderElectionPort)
		{
			memset((char *) &si_leaderElectionSock, 0, sizeof(si_leaderElectionSock));
    		si_leaderElectionSock.sin_family = AF_INET;
   			si_leaderElectionSock.sin_port = htons(usrInGrpClientRecord[i].leaderElectionPort);
			
			Messageobj NewMessage;
			NewMessage.newmsg.messageType=LE_PortSend;
			
			string num=ToString(currentUser.portNumber);
			strcpy(NewMessage.newmsg.mess, num.c_str());
			
			if (sendto(leaderElectionSock, &NewMessage, sizeof(struct Messageobj), 0 , (struct sockaddr *) &si_leaderElectionSock, leaderElectionSock_slen)==-1) {
            error("sendto()");
        	}
			
			if (recvfrom(leaderElectionSock, &NewMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_leaderElectionSock, &leaderElectionSock_slen) == -1) {
             error("recv()");
        	}
			
			if(NewMessage.newmsg.messageType==LE_Reply)
			{
				if(strcmp(NewMessage.newmsg.mess, "LESSER")==0) leaderElectionCount++;
				
			if(leaderElectionCount==(usrInGrpClientRecordCtr-1))
			{
				Messageobj t1;
				strcpy(t1.newmsg.mess, "We have a winner whose port is = ");
				strcat(t1.newmsg.mess, ToString(currentUser.portNumber).c_str());
				SimplyPrint(t1);
				break;
				
				
			}
			}
			
			
			
		}
	}
	
	/*
	
	
	-7. All the existing threads in the client is suspended.
	-8. Client restarts as sequencer ( with preserving some state of data , not all will be reset )
	-9. Client sends a message to everyone else saying, im the new seq and my ipport is this
	-10. each of the other clients, terminates thier existing threads and establishes new connection to sequencer 
	
	
	*/
	
}

void *leaderElection_handler(void *)
{
	
	while(1) {

        struct Messageobj newmess;
        
		
        if (recvfrom(socket_leaderElection, &newmess, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_leaderElection, &si_leaderElection_slen) == -1) {
            
        }
		
		if(newmess.newmsg.messageType==LE_PortSend)
		{
			int clientPortToCompare=atoi(newmess.newmsg.mess);
			
			if(clientPortToCompare>currentUser.portNumber)
			{
				
				Messageobj replyMsg;
				replyMsg.newmsg.messageType=LE_Reply;
				strcpy(replyMsg.newmsg.mess, "LESSER");
				replyMsg.newmsg.port=currentUser.portNumber;
				
				if (sendto(socket_leaderElection, &replyMsg, sizeof(struct Messageobj), 0 , (struct sockaddr *) &si_leaderElection, si_leaderElection_slen)==-1) {
           			 error("sendto()");
        		}
				
				
			}
			else
			{
				Messageobj replyMsg;
				replyMsg.newmsg.messageType=LE_Reply;
				strcpy(replyMsg.newmsg.mess, "HIGHER");
				replyMsg.newmsg.port=currentUser.portNumber;
				
				if (sendto(socket_leaderElection, &replyMsg, sizeof(struct Messageobj), 0 , (struct sockaddr *) &si_leaderElection, si_leaderElection_slen)==-1) {
           			 error("sendto()");
        		}
				
			}
		}
		
		
		

    }
}

void *client_ack_handler(void *)
{
	int seqFailed=0;
	
    while(1) {

        string m="";
        char send[1024];
        strcpy(send, "ack");
        Message msgg;
        msgg.messageType=ACK;

        struct Messageobj newmess;
        newmess.newmsg=msgg;

        if (sendto(client_s, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &si_ackclient, sizeof(si_ackclient))==-1) {
            error("sendto()");
        }
        if (recvfrom(client_s, &newmess, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_ackclient, &ack_slen) == -1) {
            cout<<"seq failed"<<endl;
			seqFailed=1;
			break;
        }

    }
	
	if(seqFailed==1) SequencerVanished();
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

void *addToMultiCastDS(Messageobj newMessage, sockaddr_in si_other)
{
    bool isExisting=false;
    for(int i=0; i<clientListCtr; i++)
        if((int)ntohs(si_other.sin_port)==(int)ntohs(clientList[i].sin_port)) isExisting=true;

    if(!isExisting) {

        Message newNotice;
        newNotice.messageType=NOTIFICATION;

        string name(newMessage.newmsg.name);
        string ip(newMessage.newmsg.ip);

        string notif_msg="";
        notif_msg="NOTICE "+name+" joined on "+ip+":"+ToString(newMessage.newmsg.port);//+port;

        strcpy(newNotice.mess ,notif_msg.c_str());

        struct Messageobj newNotifMessage;
        newNotifMessage.newmsg=newNotice;
        newNotifMessage.CUser=newMessage.CUser;
		
        multicast(newNotifMessage);
        mPrint.lock();
        printMessages(newNotifMessage,false);
        mPrint.unlock();
        hbList[clientListCtr]= create_sock_struct(newMessage);

        flag=1;
        clientList[clientListCtr]=si_other;
        clientListCtr++;
    }


    strcpy(usersInGroup[usersInGroupCtr].name, newMessage.newmsg.name);
    strcpy(usersInGroup[usersInGroupCtr].ip, newMessage.newmsg.ip);
    usersInGroup[usersInGroupCtr].port= newMessage.newmsg.port;
    usersInGroup[usersInGroupCtr].hbport = newMessage.newmsg.hbport;
    // usersInGroup[usersInGroupCtr].UIRI= (int)ntohs(si_other.sin_port);
    usersInGroupCtr++;

	usrInGrpSeqRecord[usrInGrpSeqRecordCtr]=newMessage.CUser;
	usrInGrpSeqRecordCtr++;
	
    //Send Existing Info

    //Sending number of users
    char message[1024];
    strcpy(message,"");
    char snum[5];
    sprintf(snum, "%d", usersInGroupCtr);
    strcpy(message,snum);


    if (sendto(sock, &message, sizeof(message), 0 , (struct sockaddr *) &si_other, newUser_slen)==-1) {
        error("sendto()");
    }


    //Sending User Data
    struct Messageobj existUserList;
    for(int i=0; i<usersInGroupCtr; i++) {
        string name(usersInGroup[i].name);
        string ip(usersInGroup[i].ip);

        string notif="";
        notif=name+ " "+ip+":"+ToString(usersInGroup[i].port);
        strcpy( existUserList.newmsg.mess ,notif.c_str());

        existUserList.newmsg.messageType=NOTIFICATION;
		
		existUserList.CUser=usrInGrpSeqRecord[i];
        
		if (sendto(sock, &existUserList, sizeof(struct Messageobj), 0 , (struct sockaddr *) &si_other, newUser_slen)==-1) {
            error("sendto()");
        }
    }
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

void *multicast(Messageobj newMessage)
{

    for(int i=0; i<clientListCtr; i++) {

        if (sendto(sock, &newMessage, sizeof(struct Messageobj), 0 , (struct sockaddr *) &clientList[i], slen)==-1) {
            error("sendto()");
        }
    }

}

void *removefrommulticast(int client_remove,string name)
{
    int t=0;
    for(int i=0; i<clientListCtr; i++) {
        if((int)ntohs(clientList[i].sin_port)==client_remove) {
            clientList[i]=clientList[i+1];
            t=1;
        } else if(t==1) {
            clientList[i]=clientList[i+1];
        }
    }


    clientListCtr--;
    t=0;
    for(int i=0; i<usersInGroupCtr; i++) {
        if(usersInGroup[i].port==client_remove) {
            usersInGroup[i]=usersInGroup[i+1];
            t=1;
        } else if(t==1) {
            usersInGroup[i]=usersInGroup[i+1];
        }
    }
    usersInGroupCtr--;

    Message newNotice;
    newNotice.messageType=NOTIFICATION;

    string notif_msg="";
    notif_msg="NOTICE "+name+" left the chat or crashed ";
    strcpy(newNotice.mess ,notif_msg.c_str());

    struct Messageobj newNotifMessage;
    newNotifMessage.newmsg=newNotice;
    mPrint.lock();
    printMessages(newNotifMessage,false);
    mPrint.unlock();
    multicast(newNotifMessage);

}

void *removeackclient(int client_remove)
{
    int t=0;
    for(int i=0; i<clientListCtr+1; i++) {
        if((int)ntohs(hbList[i].sin_port)==client_remove) {
            hbList[i]=hbList[i+1];
            t=1;
        } else if(t==1) {
            hbList[i]=hbList[i+1];
        }
    }
}

void *seq_receiver_handler(void *)
{
    struct Messageobj newIncomingMessage;
    while(1) {
        int recv_len;
        if ((recv_len = recvfrom(sock, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_other, &slen)) == -1) {
            error("recvfrom()");
        }

        //Sequencer Welcoming new client
        string newMessageArrived(newIncomingMessage.newmsg.mess);
        string user_name=newIncomingMessage.newmsg.name;
        string newMessage;
        Message msgg;
        if(newMessageArrived == "JOIN") {
            newMessage="Succeeded, current users:\n";
            newMessage += welcome_mess;
            strcpy(msgg.mess ,newMessage.c_str());
            msgg.seqNum=seq;
            struct Messageobj newmess;
            newmess.newmsg=msgg;
            newmess.newmsg.messageType=NOTIFICATION;
            if (sendto(sock, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &si_other, newUser_slen)==-1) {
                error("sendto()");
            }

            addToMultiCastDS(newIncomingMessage, si_other);
            mPrint.lock();
            printMessages(newIncomingMessage,false);
            mPrint.unlock();

        } else if(newMessageArrived == "group_leave") {
            int clientid2;
            int clientid = (int)ntohs(si_other.sin_port);
            for(int i=0; i<usersInGroupCtr; i++) {
                if(usersInGroup[i].port==(int)ntohs(si_other.sin_port)) {
                    clientid2 = usersInGroup[i].hbport;
                    break;
                }
            }

            removefrommulticast(clientid,user_name);
            removeackclient(clientid2);
        } else {

            if(newIncomingMessage.newmsg.messageType==DATA) {
                seq=seq+1;
                newIncomingMessage.newmsg.seqNum=seq;
            }
            mPrint.lock();
            printMessages(newIncomingMessage,false);
            mPrint.unlock();
            multicast(newIncomingMessage);
        }


    }
}
