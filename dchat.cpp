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
#include <time.h>
#include <queue>
#include <mutex>

#define NOTIFICATION 1
#define DATA 2
#define INTERNAL 3

using namespace std;

int sock;

unsigned long int seq=0;
unsigned long int seqChk=1;
//std::mutex m;
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
	   unsigned long int seqNum;
};

//Variables if a client
 struct sockaddr_in newUser_si_other;
 int newUser_s, newUser_i;
 socklen_t newUser_slen=sizeof(newUser_si_other);
socklen_t slen;

struct sockaddr_in si_me, si_other;

//Variables for sequencer

//Variables for client_Bind
struct sockaddr_in si_user;
socklen_t newJoinUser_slen=sizeof(si_user);


string welcome_mess;

struct Messageobj
{
 	Message newmsg;	
}msg;

struct Messageobj mess1;

 char client_name[50];

queue<Messageobj> msgOrder;
//bool recvFlag;

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

ChatUser initSeq;
ChatUser newUser;


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
void *printMessages(Messageobj newMessage,bool recvFlag);
void *addToMultiCastDS(Messageobj newIncomingMessage, sockaddr_in si_other);
void *multicast(Messageobj newMessage);
void *removefrommulticast(int client_remove);
void *seq_indirect_joining_handler(void *);



string ToString(int val);
int check=0;
sockaddr_in clientList[10]; 
int clientListCtr=0;

//Class for users in a group chat

struct NewJoin
{
	Messageobj newmsg;	
	struct sockaddr_in newJ;
};




//Method to print error messages
void error(const char *msg)
{
    perror(msg);
    exit(-1);
}

//Method to enable a user join a existing chat
void existGrpChat(ChatUser newUser){
		 
	    
		strcpy(client_name, newUser.name.c_str());

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
          	cout<<"Sorry, no chat is active on "<<newUser.seqIpAddr<<":"<<newUser.leaderPortNum<<" try again later.Bye."<<endl;
	 		exit(-1);
    	}
	
		tv.tv_sec       = 0;
		tv.tv_usec      = 0;
		
		if (setsockopt(newUser_s, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
    	perror("Error");
		}

	     if(strcmp(newIncomingMessage.newmsg.mess,"INDIRECT")==0)
			{
				string leadIP(newIncomingMessage.newmsg.ip);
				newUser.seqIpAddr=leadIP;
    			newUser.leaderPortNum= newIncomingMessage.newmsg.port;
    			close(newUser_s);
				existGrpChat(newUser);
			}
			else 
			{
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
	 	
		for(int i=0;i<number;i++)
		{
			
		
		  if (recvfrom(newUser_s, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &newUser_si_other, &newUser_slen) == -1) {
          error("recvfrom()");
    			}
			
			if(strcmp(newIncomingMessage.newmsg.mess, ":"))
			 mPrint.lock();
			 printMessages(newIncomingMessage,false);
			 mPrint.unlock();
		}
	
	
		pthread_t thread_1,thread_2, thread_3;
		pthread_create( &thread_1 , NULL , receiver_handler,(void*) 0);
		pthread_create( &thread_2, NULL , sender_handler,(void*) 0);
		//pthread_create( &thread_3, NULL , receiver_handler_newJoin,(void*) 0);
	
		pthread_join(thread_1,NULL);
		pthread_join(thread_2,NULL);
		//pthread_join(thread_3,NULL);
	
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


    	pthread_t thread_1,thread_2,thread_3,thread_4;
		pthread_create( &thread_1, NULL , seq_receiver_handler,(void*) 0);
		pthread_create( &thread_3, NULL , seq_mess_sender_handler,(void*) 0);
		//pthread_create( &thread_4, NULL , seq_indirect_joining_handler,(void*) 0);
		
		pthread_join(thread_1,NULL);
		pthread_join(thread_3,NULL);
		//pthread_join(thread_4,NULL);

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
 
	srand(time(NULL));
 if(argc==2)
	 {
		 initSeq.isSequencer=true;
		 initSeq.ipAddr=getIP();
		 initSeq.portNumber=5000;
		 initSeq.name=argv[1];
		 initSeq.leaderPortNum=initSeq.portNumber;
		 initSeq.seqIpAddr=initSeq.ipAddr;
		 initSeq.UIRI++;
	 	  
	  	 //initSeq.seqIpAddr="130.91.141.49";
		 //initSeq.ipAddr="130.91.141.49";
	 	 
	 newGrpChat(initSeq);
	
 }
	 else if(argc==3)
	 {
		 //ChatUser newUser;
		 newUser.isSequencer=false;
		 newUser.ipAddr=getIP();
		 newUser.portNumber=1023+rand()%1000;
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

		  //newUser.seqIpAddr="130.91.141.49";
		  //newUser.ipAddr="130.91.141.49";
		 
		 existGrpChat(newUser);
		 return 0;
    }
		 
		  
	 else
	 {
		 cout<<"Error:Invalid arguments"<<endl;
		 return -1;
	 }

	 
}
void *printMessages(Messageobj newMessage, bool recvFlag)
{
	
	if(newMessage.newmsg.messageType==DATA)
	   {
	   	   //cout<<"seq "<<newMessage.newmsg.seqNum<<" seqChk "<<seqChk<<endl;
	   	   
	   	   if(newMessage.newmsg.seqNum==seqChk){
		   cout<<"seq "<<newMessage.newmsg.seqNum<<" "<<newMessage.newmsg.name<<":: "<<newMessage.newmsg.mess<<endl;

		   if(recvFlag){
	   	   		msgOrder.pop();
	   	   		//recvFlag=false;
	   	   	}
		   seqChk=seqChk+1;
		   }
		   else
		   {
		   	msgOrder.push(newMessage);
		   }

		   seqNext=msgOrder.front().newmsg.seqNum;

		   if(seqNext==seqChk){
		   	//recvFlag=true;
		   	mPrint.lock();
		   	printMessages(msgOrder.front(),true);
		   	mPrint.unlock();
		   }
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
		
		string newMessageArrived(newIncomingMessage.newmsg.mess);
		string newMessage;
		Message msgg;
		if(newMessageArrived == "JOIN")
			{
			  //cout<<"New User Trying to join"<<endl;
			   
			Message giveSEQIPData;
			
			
			strcpy(giveSEQIPData.mess, "INDIRECT");
			strcpy(giveSEQIPData.ip, newUser.seqIpAddr.c_str());
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
	while(1){
		
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
		//cout<<initSeq.name<<":"<<newmess.newmsg.mess<<endl;
		mPrint.lock();
		printMessages(newmess,false);
		mPrint.unlock();
	    
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
		Message msgg;
		getline(cin,m);
		if(cin.eof()==1){
			strcpy(send, "group_leave");
			msgg.messageType=INTERNAL;
			check=1;
		}
		else{
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
	  	mPrint.lock();
		printMessages(newNotifMessage,false);
		mPrint.unlock();
			
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

void *removefrommulticast(int client_remove,string name){
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
		string user_name=newIncomingMessage.newmsg.name;
		string newMessage;
		Message msgg;
		if(newMessageArrived == "JOIN")
			{	
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
					
				}
		else if(newMessageArrived == "group_leave"){
			int clientid = (int)ntohs(si_other.sin_port);
			removefrommulticast(clientid,user_name);		
		}
		else{

		if(newIncomingMessage.newmsg.messageType==DATA){
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

/*

void *seq_indirect_joining_handler(void *)
{
	struct NewJoin newIncomingJoin; 
	while(1){
		int recv_len;
	if ((recv_len = recvfrom(sock, &newIncomingJoin, sizeof(struct NewJoin), 0, (struct sockaddr *) &si_other, &slen)) == -1) {
              error("recvfrom()");
       	 	}
	cout<<"New join request forwarded from client..ALOHA " <<endl;
		
		socklen_t newJSlen;
		newJSlen=sizeof(newIncomingJoin.newJ);
		
		string newMessage;
		Message msgg;
		newMessage="Succeeded, current users:\n";	
		newMessage += welcome_mess;
		strcpy(msgg.mess ,newMessage.c_str());
				
		struct Messageobj newmess;
		newmess.newmsg=msgg;
		newmess.newmsg.messageType=NOTIFICATION;
		if (sendto(sock, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &newIncomingJoin.newJ, &newJSlen)==-1) {
         	 error("sendto()");
		}
			
		addToMultiCastDS(newIncomingJoin.newmsg, si_other);
		printMessages(newIncomingJoin.newmsg);
		
		
	}
}
*/
