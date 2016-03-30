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
using namespace std;

//#define PORT 5000

struct Message                                                           
  { 
    char IncomingMessage[1024];

  }msg;




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
//public:
		//other methods that are needed
		//getIpAddr(){
		//}	
		//getPortNum(){
		//}
};


//Declaration of global variables


//Method to print error messages
void error(const char *msg)
{
    perror(msg);
    exit(-1);
}

/*
int createSocket(int portNum)
{
	if ((sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        error("socket");
    }
     
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    if( bind(sock , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) {
        error("bind");
    }
	
	return sock;
}

*/
/*
void newGrpChat(void *sock, ChatUser initSeq){
	
	int sockfd = *(int*)sock;
	
	while(1)
    {
        if ((recv_len = recvfrom(sockfd, &msg, sizeof(struct Message), 0, (struct sockaddr *) &si_other, &slen)) == -1) {
            error("recvfrom()");
        }
		
		cout<<"Message : "<<endl;
		 
	 }
    return;
}*/

string getIP()	{
	
	return "192.168.1.2";
}


int main(int argc, char* argv[]){
 
 if(argc==2)
	 {
		 cout<<"New chat group Initiated"<<endl;
	 
		 ChatUser initSeq;
		 initSeq.isSequencer=true;
		 initSeq.ipAddr=getIP();
		 //initSeq.portNumber=1023+(rand()%1000);
		 initSeq.portNumber=5000;
		 initSeq.name=argv[1];
		 initSeq.leaderPortNum=initSeq.portNumber;
		 initSeq.UIRI++;
	 
	 
	     // TODO 1: move to function 
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
		 // END TODO 1
	   
	 
	 	while(1)
    	{
            if ((recv_len = recvfrom(sock, &msg, sizeof(struct Message), 0, (struct sockaddr *) &si_other, &slen)) == -1) {
              error("recvfrom()");
       	 	}
			 
		    cout << msg.IncomingMessage << endl; 
			string tmp(msg.IncomingMessage);
		    string newMessage=tmp+" Hello From the other side\0";	
			strcpy(msg.IncomingMessage, newMessage.c_str());	
		    if (sendto(sock, &msg, sizeof(struct Message), 0 , (struct sockaddr *) &si_other, slen)==-1) {
              error("sendto()");
            }
		 
			 
		}
	
 }
	 else if(argc==3)
	 {
		 cout<<"Attempting to join an existing chat......"<<endl;
		 ChatUser newUser;
		 newUser.isSequencer=false;
		 //newUser.ipAddr="sarathlogic";
		 //newUser.portNumber=1023+(rand()%1000);
		 newUser.name=argv[1];
           
		 
		 
		 // TODO 2: move to function 
		 
		 
		 struct sockaddr_in si_other;
  	     int s, i;
		 socklen_t slen=sizeof(si_other);
  
    	 if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        	error("socket");
    	 }
 
    	memset((char *) &si_other, 0, sizeof(si_other));
    	si_other.sin_family = AF_INET;
    	si_other.sin_port = htons(5000);
     
    	if (inet_aton("127.0.0.1", &si_other.sin_addr) == 0)  {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
		}
		 
		 //END TODO 2
		
		while(1){	
				
		cout<<"Enter message"<<endl;
		string m="";	
		getline(cin,m);
		strcpy(msg.IncomingMessage, m.c_str());	
		
	    
					
		if (sendto(s, &msg, sizeof(struct Message), 0 , (struct sockaddr *) &si_other, slen)==-1) {
           error("sendto()");
        }
				
		if (recvfrom(s, &msg, sizeof(struct Message), 0, (struct sockaddr *) &si_other, &slen) == -1) {
          error("recvfrom()");
    	}
	    cout << "The message was = " << msg.IncomingMessage<< endl;
			
			
		}
    }
		 
		 
		 
	 
	 
	 else
	 {
		 cout<<"Error:Invalid arguments"<<endl;
		 return -1;
	 }

	 //createSocket();
	 
}

