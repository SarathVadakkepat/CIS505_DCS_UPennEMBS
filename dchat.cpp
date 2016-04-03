/**
Description: CIS 505 Project 3
Authors: Karthik Anantha Ram, Sanjeet Phatak, Sarath Vadakkepat
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

#define NOTIFICATION 1
#define DATA 2
#define INTERNAL 3

using namespace std;

int sock;
class Message
{
public:
	   int messageType;
	   char mess[1024];
	   char name[100];
	   char ip[100];
	   int port;
};

struct sockaddr_in si_me, si_other;

string welcome_mess;
struct Messageobj
{
 	Message newmsg;	
}msg;

struct Messageobj mess1;

 char client_name[50];
//Variables if a client
 struct sockaddr_in newUser_si_other;
 int newUser_s, newUser_i;
 socklen_t newUser_slen=sizeof(newUser_si_other);
socklen_t slen;

int lifecheck;

class ChatUser
{
public:
		string name;
		int UIRI;
		int portNumber;
		string ipAddr;
		bool isSequencer=false;
		int leaderPortNum;
		string seqIpAddr;
};

//struct Userobj
//{
 //	ChatUser newUsr;	
//};
//struct ChatUser usersInGroup[10];


struct UserInGroup
{
	char name[32];
	char ip[32];
	int port;
	int UIRI;

};
struct UserInGroup usersInGroup[10];
int usersInGroupCtr=0;

void *receiver_handler(void *);
void *sender_handler(void *);
void *seq_receiver_handler(void *);
void *seq_mess_sender_handler(void *);
void *printMessages(Messageobj newMessage);
void *addToMultiCastDS(Messageobj newIncomingMessage, sockaddr_in si_other);
void *multicast(Messageobj newMessage);
void *removefrommulticast(int client_remove);
string ToString(int val);
int check=0;
sockaddr_in clientList[10]; 
int clientListCtr=0;

//Class for users in a group chat






//Method to print error messages
void error(const char *msg)
{
    perror(msg);
    exit(-1);
}

//Method to enable a user join a existing chat
void existGrpChat(ChatUser newUser){
		 
	    
		cout<<newUser.name<<" joining a new chat on "<<newUser.seqIpAddr<<":"<<newUser.leaderPortNum<<", listening on "<<newUser.ipAddr<<":"<<newUser.portNumber<<endl;
		strcpy(client_name, newUser.name.c_str());

 		if ( (newUser_s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        	error("socket");
    	 }
		
    	memset((char *) &newUser_si_other, 0, sizeof(newUser_si_other));
    	newUser_si_other.sin_family = AF_INET;
    	newUser_si_other.sin_port = htons(newUser.leaderPortNum);
     
    	if (inet_aton(newUser.seqIpAddr.c_str(), &newUser_si_other.sin_addr) == 0)  {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
		}
	
		Message newMessage;
		strcpy(newMessage.mess ,"JOIN");
		strcpy(newMessage.name, newUser.name.c_str());
		strcpy(newMessage.ip, newUser.ipAddr.c_str());
	    newMessage.port=newUser.portNumber;
		newMessage.messageType=INTERNAL;
	
		struct Messageobj newMessageObj;
		newMessageObj.newmsg=newMessage;
	
	    if (sendto(newUser_s, &newMessageObj, sizeof(struct Messageobj), 0 , (struct sockaddr *) &newUser_si_other, newUser_slen)==-1) {
           error("sendto()");
        }
		 		
	
		struct Messageobj newIncomingMessage; 
		//Receiving Welcome Message
	 	if (recvfrom(newUser_s, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &newUser_si_other, &newUser_slen) == -1) {
          error("recvfrom()");
    			}
	 	printMessages(newIncomingMessage);
	
	
		//Receiving number of users
	    char userCount[32];
	    if (recvfrom(newUser_s, &userCount, sizeof(userCount), 0, (struct sockaddr *) &newUser_si_other, &newUser_slen) == -1) {
          error("recvfrom()");
    			}
	
	
	    int number=atoi(userCount);
	 	
		for(int i=0;i<number;i++)
		{
			
		
		  if (recvfrom(newUser_s, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &newUser_si_other, &newUser_slen) == -1) {
          error("recvfrom()");
    			}
			
			if(strcmp(newIncomingMessage.newmsg.mess, ":"))
			 printMessages(newIncomingMessage);
		}
	
	
		pthread_t thread_1,thread_2;
		pthread_create( &thread_1 , NULL , receiver_handler,(void*) 0);
		pthread_create( &thread_2, NULL , sender_handler,(void*) 0);
		pthread_join(thread_1,NULL);
		pthread_join(thread_2,NULL);
		cout<<"Hi"<<endl;
		 //Replace while with thread join
		while(check==1){
		return;		
	}
}

//Method to initiate sequencer
void newGrpChat(ChatUser initSeq){
		 
		cout<<initSeq.name<<" started a new chat, listening on "<<initSeq.ipAddr<<":"<<initSeq.portNumber<<endl;
	 	cout<<"Succeeded, current users:"<<endl;
		cout<<initSeq.name<<" "<<initSeq.ipAddr<<"."<<initSeq.portNumber<<" (Leader)"<<endl;
		
	    strcpy(client_name, initSeq.name.c_str());
		char samp[20];
		
		sprintf(samp, "%d", initSeq.portNumber);
		string sam(samp);
		
		welcome_mess = initSeq.name + " " + initSeq.ipAddr + ":" + sam + "(Leader)"; 
		
		cout<<"Waiting for others to join..."<<endl;
		 	 
		
		int i, recv_len;
		slen= sizeof(si_other);
	 
	 
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


    	pthread_t thread_1,thread_2,thread_3;
		pthread_create( &thread_1, NULL , seq_receiver_handler,(void*) 0);
		pthread_create( &thread_3, NULL , seq_mess_sender_handler,(void*) 0);

		pthread_join(thread_1,NULL);
		pthread_join(thread_3,NULL);
	

    return;
}
string getIP()	{
	
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
            
            if(strcmp(ifa->ifa_name,"em1") == 0)
            {
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


int main(int argc, char* argv[]){
 
 if(argc==2)
	 {
		 
		 ChatUser initSeq;
		 initSeq.isSequencer=true;
		 initSeq.ipAddr=getIP();
		 initSeq.portNumber=5000;
		 initSeq.name=argv[1];
		 initSeq.leaderPortNum=initSeq.portNumber;
		 initSeq.seqIpAddr=initSeq.ipAddr;
		 initSeq.UIRI++;
	 	    
	  	 initSeq.seqIpAddr="10.0.0.138";
		 initSeq.ipAddr="10.0.0.138";
	 	 
	 newGrpChat(initSeq);
	
 }
	 else if(argc==3)
	 {
		 ChatUser newUser;
		 newUser.isSequencer=false;
		 newUser.ipAddr=getIP();
		// newUser.portNumber=1023+rand()%1000;
		 newUser.name=argv[1];
		 newUser.UIRI++;

		 string ipPort=argv[2];
		 istringstream iss(ipPort);
		 string token;

		 int cnt=0;
		 while (getline(iss, token, ':'))
		 {
		 	if(cnt==0)
    			newUser.seqIpAddr=token;
    		else if(cnt==1)
    			newUser.leaderPortNum= atoi(token.c_str());
    		else
    			break;
    		cnt++;
		 }

		  newUser.seqIpAddr="10.0.0.138";
		 newUser.ipAddr="10.0.0.138";
		 
		 existGrpChat(newUser);
		return 0;
    }
		 
		  
	 else
	 {
		 cout<<"Error:Invalid arguments"<<endl;
		 return -1;
	 }

	 
}
void *printMessages(Messageobj newMessage)
{
	
	if(newMessage.newmsg.messageType==DATA)
	   {
		   cout<<newMessage.newmsg.name<<":: "<<newMessage.newmsg.mess<<endl;
	   }
	if(newMessage.newmsg.messageType==NOTIFICATION)
	   {
		   cout<<newMessage.newmsg.mess<<endl;
	   }
	
}


void *receiver_handler(void *)
{
	struct Messageobj newIncomingMessage; 
	while(1){
	if (recvfrom(newUser_s, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &newUser_si_other, &newUser_slen) == -1) {
		error("recvfrom()");
    	}
	    printMessages(newIncomingMessage);
		
		
		
	}
	
	
	
}

void *seq_mess_sender_handler(void *)
{
	while(1){
		
		string m="";	
		char send[1024];
	
		getline(cin,m);
		strcpy(send, m.c_str());
		
		Message msgg;
		msgg.messageType=DATA;
		strcpy(msgg.mess, send);
		strcpy(msgg.name, client_name);
		
		struct Messageobj newmess;
		newmess.newmsg=msgg;
		
	    for(int i=0;i<clientListCtr;i++){
	    if (sendto(sock, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &clientList[i], newUser_slen)==-1) {
           error("sendto()");
			}
		}
	}
}


void *sender_handler(void *)
{
	while(1){
		
		string m="";	
		char send[1024];
		getline(cin,m);
		if(cin.eof()==1){
			Message msgg;
			strcpy(send, "group_leave");
			cout<<send<<endl;
			strcpy(msgg.mess, send);
			check=1;
			struct Messageobj newmess;
			newmess.newmsg=msgg;
			if (sendto(newUser_s, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &newUser_si_other, newUser_slen)==-1) {
         			  error("sendto()");
        		}
			exit(0);
		}
		strcpy(send, m.c_str());
		
		Message msgg;
		msgg.messageType=DATA;
		strcpy(msgg.mess, send);
		strcpy(msgg.name, client_name);
		
		struct Messageobj newmess;
		newmess.newmsg=msgg;
		
	    if (sendto(newUser_s, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &newUser_si_other, newUser_slen)==-1) {
           error("sendto()");
        }
	}
}

void *addToMultiCastDS(Messageobj newMessage, sockaddr_in si_other)
{
		bool isExisting=false;
		for(int i=0;i<clientListCtr;i++)
				if((int)ntohs(si_other.sin_port)==(int)ntohs(clientList[i].sin_port)) isExisting=true;
			
		if(!isExisting)	{
			
		Message newNotice;
	    newNotice.messageType=NOTIFICATION;
	
	    string name(newMessage.newmsg.name);
	    string ip(newMessage.newmsg.ip);
	  			
		string notif_msg="";
	    notif_msg="NOTICE "+name+" joined on "+ip+":"+ToString(newMessage.newmsg.port);//+port;
	
	    strcpy(newNotice.mess ,notif_msg.c_str()); 
	
		struct Messageobj newNotifMessage; 
		newNotifMessage.newmsg=newNotice;
	
	  	multicast(newNotifMessage);
		printMessages(newNotifMessage);
			
			clientList[clientListCtr]=si_other;
			clientListCtr++;
		}
	
	
		strcpy(usersInGroup[usersInGroupCtr].name, newMessage.newmsg.name);
		strcpy(usersInGroup[usersInGroupCtr].ip, newMessage.newmsg.ip);
	     usersInGroup[usersInGroupCtr].port= newMessage.newmsg.port;
	    usersInGroup[usersInGroupCtr].UIRI= (int)ntohs(si_other.sin_port);
	    usersInGroupCtr++;
	    
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
	    for(int i=0;i<usersInGroupCtr;i++)
		{
			 string name(usersInGroup[i].name);
	    	 string ip(usersInGroup[i].ip);
						
			string notif="";
			notif=name+ " "+ip+":"+ToString(usersInGroup[i].port);
			strcpy( existUserList.newmsg.mess ,notif.c_str()); 
		     
			existUserList.newmsg.messageType=NOTIFICATION;
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
void *multicast(Messageobj newMessage) {
	
		for(int i=0;i<clientListCtr;i++){
			
	    if (sendto(sock, &newMessage, sizeof(struct Messageobj), 0 , (struct sockaddr *) &clientList[i], slen)==-1) {
           error("sendto()");
			}
		}
		
}

void *removefrommulticast(int client_remove){
	int t=0;
	for(int i=0;i<clientListCtr;i++){
		if((int)ntohs(clientList[i].sin_port)==client_remove){
			clientList[i]=clientList[i+1];	
			t=1;
		}
		else if(t==1){
			clientList[i]=clientList[i+1];	
		}
	}
	clientListCtr--;
	t=0;
	for(int i=0;i<usersInGroupCtr;i++){
		if(usersInGroup[i].port==client_remove){
			usersInGroup[i]=usersInGroup[i+1];	
			t=1;
		}
		else if(t==1){
			usersInGroup[i]=usersInGroup[i+1];	
		}
	}
	usersInGroupCtr--;
 }

void *seq_receiver_handler(void *)
{
	struct Messageobj newIncomingMessage; 
	while(1){
		int recv_len;
	if ((recv_len = recvfrom(sock, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_other, &slen)) == -1) {
              error("recvfrom()");
       	 	}

		//Sequencer Welcoming new client
		string newMessageArrived(newIncomingMessage.newmsg.mess);
		string newMessage;
		Message msgg;
		if(newMessageArrived == "JOIN")
			{	
					newMessage="Succeeded, current users:\n";	
					newMessage += welcome_mess;
					strcpy(msgg.mess ,newMessage.c_str());
				
					struct Messageobj newmess;
			        newmess.newmsg=msgg;
			        newmess.newmsg.messageType=NOTIFICATION;
			        if (sendto(sock, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &si_other, newUser_slen)==-1) {
          				 error("sendto()");
					}
			
			        addToMultiCastDS(newIncomingMessage, si_other);
				printMessages(newIncomingMessage);
			         
					
				}
		else if(newMessageArrived == "group_leave"){
			int clientid = (int)ntohs(si_other.sin_port);
			removefrommulticast(clientid);		
		}
		else{
		printMessages(newIncomingMessage);
		multicast(newIncomingMessage);
		}	
	   
		
	}
}
