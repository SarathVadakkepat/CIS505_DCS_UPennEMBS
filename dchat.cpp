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

using namespace std;

/*struct Message                                                           
  { 
    char IncomingMessage[2048];
	char name[50];

  }msg;
*/
class Message
{
public:
	   //MsgType msgType;
	   //sendInfo srcInfo;
	   //sendInfo dstInfo;
	   char mess[1024];
	   string name;
	
	   //string sendTimeStamp;
	   //string recvTimeStamp;
};


struct Messageobj
{
 	Message newmsg;	
}msg;

 char client_name[50];
//Variables if a client
 struct sockaddr_in newUser_si_other;
 int newUser_s, newUser_i;
 socklen_t newUser_slen=sizeof(newUser_si_other);


void *receiver_handler(void *);
void *sender_handler(void *);
sockaddr_in clientList[10]; int clientListCtr=0;

//Class for users in a group chat
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
	
		struct Messageobj newMessageObj;
		newMessageObj.newmsg=newMessage;
	
	    if (sendto(newUser_s, &newMessageObj, sizeof(struct Messageobj), 0 , (struct sockaddr *) &newUser_si_other, newUser_slen)==-1) {
           error("sendto()");
        }
		 		
		//TODO : Remove the junk field and figure to pass null
		int junk=0;
		pthread_t thread_1,thread_2;
		pthread_create( &thread_1 , NULL ,  receiver_handler,(void*) &junk);
		pthread_create( &thread_2, NULL ,  sender_handler,(void*) &junk);
		pthread_join(thread_1,NULL);
		pthread_join(thread_2,NULL);
		 //Replace while with thread join
		while(1){
		}
}

//Method to initiate sequencer
void newGrpChat(ChatUser initSeq){
		 
		cout<<initSeq.name<<" started a new chat, listening on "<<initSeq.ipAddr<<":"<<initSeq.portNumber<<endl;
	 	cout<<"Succeeded, current users:"<<endl;
		cout<<initSeq.name<<" "<<initSeq.ipAddr<<"."<<initSeq.portNumber<<" (Leader)"<<endl;
		cout<<"Waiting for others to join..."<<endl;
		 	 
		struct sockaddr_in si_me, si_other;
		int sock, i, recv_len;
		socklen_t slen= sizeof(si_other);
	 
	 
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


    	while(1)
    	{
			struct Messageobj mess1;
            
			if ((recv_len = recvfrom(sock, &mess1, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_other, &slen)) == -1) {
              error("recvfrom()");
       	 	}
		
		//Logic to add unique client data to array.
			bool isExisting=false;
			for(int i=0;i<clientListCtr;i++)
				if((int)ntohs(si_other.sin_port)==(int)ntohs(clientList[i].sin_port)) isExisting=true;
			
			if(!isExisting)	clientList[clientListCtr++]=si_other;
			//End Logic
			 
		    cout << mess1.newmsg.mess << endl; 
			//string tmp(mess1.newmsg.mess);
			
			string newMessageArrived(mess1.newmsg.mess);
			string newMessage;
			
			if(newMessageArrived == "JOIN")
				newMessage="Succeeded, current users:\0";	
			else
				newMessage=newMessageArrived;
			
			
			Message newMessageMultiCast;
			strcpy(newMessageMultiCast.mess ,newMessage.c_str());
	
			struct Messageobj newMessageObjMultiCast;
			newMessageObjMultiCast.newmsg=newMessageMultiCast;
			
			
			//mess1.newmsg.mess = newMessage;
			
			
			for(int i=0;i<clientListCtr;i++)
			{
		    if (sendto(sock, &newMessageObjMultiCast, sizeof(struct Messageobj), 0 , (struct sockaddr *) &clientList[i], slen)==-1) {
              error("sendto()");
            }
		}
		 	 
	}

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
	 	    
	 	 newGrpChat(initSeq);
	
 }
	 else if(argc==3)
	 {
		 ChatUser newUser;
		 newUser.isSequencer=false;
		 newUser.ipAddr=getIP();
		 newUser.portNumber=5001;
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

		 existGrpChat(newUser);
    }
		 
		  
	 else
	 {
		 cout<<"Error:Invalid arguments"<<endl;
		 return -1;
	 }

	 
}

void *receiver_handler(void *socket_desc)
{
	while(1){
	if (recvfrom(newUser_s, &msg, sizeof(struct Messageobj), 0, (struct sockaddr *) &newUser_si_other, &newUser_slen) == -1) {
          error("recvfrom()");
    	}
	    cout << msg.newmsg.mess<< endl;
	}
}


void *sender_handler(void *socket_desc)
{
	while(1){
		
		string m="";	
		char send[2048];
		strcpy(send, client_name);
		strcat(send,": ");
		getline(cin,m);
		strcat(send, m.c_str());
		//newmess.newmsg.mess= send;	
		
		Message msgg;
		strcpy(msgg.mess, send);
		
		struct Messageobj newmess;
		newmess.newmsg=msgg;
		
		
		
		
		
		
		
		
	    if (sendto(newUser_s, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &newUser_si_other, newUser_slen)==-1) {
           error("sendto()");
        }
	}
}
