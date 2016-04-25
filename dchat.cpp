/**
Description: CIS 505 Project 3
Authors: Karthik Anantha Ram, Sanjeet Phatak and Sarath Vadakkepat
**/

#include "ChatServer.h"
#include <time.h>
//Compulsory needed variables and cross checked 

ChatUser currentUser;

pthread_t thread_1,thread_2, thread_3, thread_4 , thread_5, thread_6, getLine,multiCast,trafficCheck,monCliMsg;

vector<ChatUser> usrInGrpClientRecord;
vector<ChatUser> usrInGrpSeqRecord;

vector<Messageobj> priority_msg;

unsigned long int seq=0;
unsigned long int seqChk=1;
unsigned long int pktCount=0;

int seqNext=0;
mutex mPrint , priorityQueue, userInGrpSeq , cliQueue, seqUpdate , cliRecvPrint, cliRecvPrint1 , seqRecvPrint;

mutex ClientSendFile, seqSendFile;

vector<Messageobj> holdBack;
vector<Messageobj> cliMsgQueue;

int leaderElectionCount;
int seqShifted=0;
int seqFailed=0;
string mInput="";

int inputFlag=0;
unsigned long int seq_arr[10];
unsigned long int cli_arr[10];
int eofCheck=0;

int leaderElectionHappening=0;int numberOfMessageGot=0;
int potentialLeaderVanished=0;
int initializingNewClient_RH, initializingNewClient_SH, signalNewClient_RH;

int End_CR, End_CS,End_LE,End_CAckH, End_MC;

bool traffClntFlg=false;

int MULTICAST_STOP=0;
int MULTICAST_STOP_CLIENT=0;

int Mhour=0;
int Mmin=0;
int Msec=0;

//Extra Credit Enabler / Disabler 

bool ExCredit_1=false;    // Traffic Control
bool ExCredit_1a = false;


bool ExCredit_2=false;    // Fair Queing
bool ExCredit_2a =false;

//Concept:
/*
-Make multicast que sleep for 10 secs
-So that data messages and notice and leave messges can be filled into it
-in the next clearing, notice messages should be sent before data messages
*/
bool ExCredit_3=false;  bool ExCredit_3a=false;
// Message Priority
//Start with  ./dchat ExCredit_4 <ip:port>
bool ExCredit_4=false;    // Encrypted Chat messages
bool ExCredit_7b=false;   // GUI



bool inactiveChat=false;
bool beat=false;
bool cliMessageSend=true; //for client to send msgs

pthread_t androidRecv, androidSend;

int socket_Android;
struct sockaddr_in si_Android;
socklen_t si_Android_slen=sizeof(si_leaderElection);

int android=1;

ofstream file_seq;
ofstream file_client;
//ofstream file_recv_seq;
ofstream file_recv_cli;
ofstream file_recv_cli1;
ofstream holdBackPrint;

ofstream SeqMCPrint_File;
mutex SeqMCPrint;
//ofstream seqMultiCast;

mutex PARENTMUTEX_Client;
mutex PARENTMUTEX_Seq;
//End

string welcome_mess;

bool reStarting_AfterLE=false;



void AndroidInitialize() {
	if ( (socket_Android=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        	error("socket");
  	}
    
    memset((char *) &si_Android, 0, sizeof(si_Android));
    si_Android.sin_family = AF_INET;
    si_Android.sin_port = htons(80000);
    si_Android.sin_addr.s_addr = htonl(INADDR_ANY);
   
	struct timeval tv2;
    tv2.tv_sec       = 10;
    tv2.tv_usec      = 1000;
    if (setsockopt(socket_Android, SOL_SOCKET, SO_RCVTIMEO,&tv2,sizeof(tv2)) < 0) {
        perror("Error");
    }
     if( bind(socket_Android , (struct sockaddr*)&si_Android, sizeof(si_Android) ) == -1) {
       // error("bind 4");
    }
	
	struct Messageobj newIncomingMessage1;
    if (recvfrom(socket_Android, &newIncomingMessage1, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_Android, &si_Android_slen) == -1) {
       android=0;
    }
    
	if(android==1)
	{
		cout<<"This is an Android user"<<endl;
		pthread_t androidRecv, androidSend;
		pthread_create( &androidRecv , NULL , android_interface_receiver_handler,(void*) 0);
       
		currentUser.clientType=ANDROID;
		
		 tv2.tv_sec       = 0;
   		 tv2.tv_usec      = 0;
			if (setsockopt(socket_Android, SOL_SOCKET, SO_RCVTIMEO,&tv2,sizeof(tv2)) < 0) {
        		perror("Error");
    		}
	}
	
	else {
	    cout<<"This is Not an Android user"<<endl;
		close(socket_Android);
	}
}

void initialize() {
    
	inactiveChat=false;
	beat=false;
	
   	seq=0;
    seqChk=1;
	
	leaderElectionHappening=0;
	numberOfMessageGot=0;
	potentialLeaderVanished=0;
	
	inputFlag=0;
	eofCheck=0;
	
	usrInGrpClientRecord.clear();
	usrInGrpSeqRecord.clear();

	if(cliMsgQueue.empty())
	   cliMsgQueue.clear();

	leaderElectionCount=0;
	initializingNewClient_RH=0;
	initializingNewClient_SH=0;
	signalNewClient_RH=0;
	
	End_CR=0;
	End_CS=0;
	End_LE=0;
	End_CAckH=0;
	End_MC=0;
	
	for(int i=0;i<10;i++) {
		seq_arr[i]=0;
		cli_arr[i]=0;	
	}
	pktCount=0;
	
	
}

void wait(int s){
clock_t endwait;
endwait=clock()+s*CLOCKS_PER_SEC;
	while(clock()<endwait){}
}

void existGrpChat(ChatUser newUser) {
	
	if(reStarting_AfterLE){
		cout<<"Connecting to new sequencer"<<endl;
	}
    if(MULTICAST_STOP_CLIENT==1){
    }
	
	initialize();
    currentUser=newUser;
   
	if(ExCredit_7b) AndroidInitialize();
	
	if ( (newUser_s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        error("socket");
    }

    memset((char *) &newUser_si_other, 0, sizeof(newUser_si_other));
    newUser_si_other.sin_family = AF_INET;
    newUser_si_other.sin_port = htons(currentUser.leaderPortNum);
    
    if ( (client_ack=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        error("socket");
    }

    memset((char *) &si_user, 0, sizeof(si_user));
    si_user.sin_family = AF_INET;
    si_user.sin_port = htons(currentUser.portNumber);
    si_user.sin_addr.s_addr = htonl(INADDR_ANY);

   	struct timeval tv_ack;
    tv_ack.tv_sec       = 4;
    tv_ack.tv_usec      = 1000;
	
	si_clientsend.sin_family = AF_INET;
    si_clientsend.sin_port = htons(newUser.sendportNumber);
    si_clientsend.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if( bind(client_ack , (struct sockaddr*)&si_clientsend, sizeof(si_clientsend) ) == -1) {
        error("bind old 5000");
    }

	if (setsockopt(client_ack, SOL_SOCKET, SO_RCVTIMEO,&tv_ack,sizeof(tv_ack)) < 0) {
        perror("Error");
    }
	
    if( bind(newUser_s , (struct sockaddr*)&si_user, sizeof(si_user) ) == -1) {
        error("bind 1");
    }

    if (inet_aton(currentUser.seqIpAddr, &newUser_si_other.sin_addr) == 0)  {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    if ( (client_s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        error("socket");
    }

    memset((char *) &si_ackclient, 0, sizeof(si_ackclient));
    si_ackclient.sin_family = AF_INET;
    si_ackclient.sin_port = htons(newUser.leaderHBPortNumber);

    if (inet_aton(currentUser.seqIpAddr, &si_ackclient.sin_addr) == 0)  {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    memset((char *) &si_hb, 0, sizeof(si_hb));
    si_hb.sin_family = AF_INET;
    si_hb.sin_port = htons(currentUser.hbportNumber);
	char *myIP=currentUser.ipAddr;
    si_hb.sin_addr.s_addr = inet_addr(myIP);

    if( bind(client_s , (struct sockaddr*)&si_hb, sizeof(si_hb) ) == -1) {
        error("bind 2");
    }
	
	if ( (socket_leaderElection=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        error("socket");
    }
    
    memset((char *) &si_leaderElection, 0, sizeof(si_leaderElection));
    si_leaderElection.sin_family = AF_INET;
    si_leaderElection.sin_port = htons(currentUser.leaderElectionPort);
    si_leaderElection.sin_addr.s_addr = htonl(INADDR_ANY);

    if( bind(socket_leaderElection , (struct sockaddr*)&si_leaderElection, sizeof(si_leaderElection) ) == -1) {
        error("bind 3");
    }
    
	
	
			Message newMessage;
    		strcpy(newMessage.mess ,"JOIN");
    		strcpy(newMessage.name, currentUser.name);
    		strcpy(newMessage.ip,currentUser.ipAddr);
    		newMessage.port=currentUser.portNumber;
    		newMessage.hbport=currentUser.hbportNumber;
    		newMessage.messageType=INTERNAL;
			//newMessage.CUser=currentUser;

    		struct Messageobj newMessageObj;
    		newMessageObj.newmsg=newMessage;
    		newMessageObj.CUser=currentUser;
    
	
			if(ExCredit_3a){
			
				if( (strcmp(currentUser.name, "ExCredit_3_JOIN_C1")==0) || (strcmp(currentUser.name, "ExCredit_3_JOIN_C2")==0)){
				time_t now = time(0);
				tm *ltm = localtime(&now);
				
				while(1){
		
					if(ltm->tm_hour==Mhour&&ltm->tm_min==Mmin&&ltm->tm_sec==Msec) {
						break;
					}
				now = time(0);
				ltm = localtime(&now);
				}
			 	}
			}
				
			pthread_create( &thread_3, NULL , client_ack_handler,(void*) 0);
	
	  		if (sendto(newUser_s, &newMessageObj, sizeof(struct Messageobj), 0 , (struct sockaddr *) &newUser_si_other, newUser_slen)==-1) {
        		error("sendto() 2");
    		}
			
			 struct timeval tv;
   			 tv.tv_sec       = 2;
    		 tv.tv_usec      = 1000;

    		 if (setsockopt(newUser_s, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
        			perror("Error");
    		}
			
			 struct Messageobj newIncomingMessage;

    		 if (recvfrom(newUser_s, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &newUser_si_other, &newUser_slen) == -1) {
      			 cout<<"Sorry, no chat is active on "<<currentUser.seqIpAddr<<":"<<currentUser.leaderPortNum<<" try again later.Bye."<<endl;
       			 
				 beat=true;
				 wait(2);
				 
				 exit(0);
				 
   			 }
			
			
			inactiveChat=true;
			//struct timeval tv;
   			 tv.tv_sec       = 0;
    		 tv.tv_usec      = 0;

    		 if (setsockopt(newUser_s, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
       	 		perror("Error");
  			  }
			
    
    		if(strcmp(newIncomingMessage.newmsg.mess,"INDIRECT")==0) {
				string leadIP(newIncomingMessage.newmsg.ip);
       			strcpy(currentUser.seqIpAddr, leadIP.c_str());
       			currentUser.leaderPortNum= newIncomingMessage.newmsg.port;
        		End_CAckH=1;
				
				struct sockaddr_in selfLoop;
		
    			memset((char *) &selfLoop, 0, sizeof(selfLoop));
    			selfLoop.sin_family = AF_INET;
				//cout<<"port hb "<<currentUser.hbportNumber<<" "<<currentUser.portNumber<<endl;
    			
				selfLoop.sin_port = htons(currentUser.hbportNumber);

  			 	if (inet_aton(currentUser.ipAddr, &selfLoop.sin_addr) == 0)  {
      		  	fprintf(stderr, "inet_aton() failed\n");
       		 	exit(1);
    			}
				
				//cout<<"port sending to "<<(int)ntohs(selfLoop.sin_port)<<endl;
				char *p=inet_ntoa(selfLoop.sin_addr);
				//cout<<"ip sending "<<p<<endl;
				Messageobj loopBack;	
				strcpy(loopBack.newmsg.mess,"FAKE");
				socklen_t len=sizeof(selfLoop);
		
			    if (sendto(newUser_s, &loopBack, sizeof(struct Messageobj), 0 , (struct sockaddr *) &selfLoop, len)==-1) {
                    error("sendto() self loop killing");
                 } 
				
		        if (recvfrom(newUser_s, &loopBack, sizeof(struct Messageobj), 0, NULL,NULL) == -1) {
                     //cout<<"failed"<<endl;

                }
				 close(newUser_s);       		
       			 close(socket_leaderElection);
				 close(client_ack);
				 close(client_s);
       			 existGrpChat(currentUser);
			  return;
				
    		 } else {
				
				if(!reStarting_AfterLE){
        			cout<<currentUser.name<<" joining a new chat on "<<currentUser.seqIpAddr<<":"<<currentUser.leaderPortNum<<", listening on "<<currentUser.ipAddr<<":"<<currentUser.portNumber<<endl;
				}
				seqChk=newIncomingMessage.newmsg.seqNum+1;
				
				if(!reStarting_AfterLE){
       		    mPrint.lock();
				printMessages(newIncomingMessage,false);
        		mPrint.unlock();
				}
    		}

   			 //Receiving number of users
    		char userCount[32];
    		if (recvfrom(newUser_s, &userCount, sizeof(userCount), 0, (struct sockaddr *) &newUser_si_other, &newUser_slen) == -1) {
    		    error("recvfrom()");
   				 }

   		    int number=atoi(userCount);
    
			for(int i=0; i<number; i++) {
				if (recvfrom(newUser_s, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &newUser_si_other, &newUser_slen) == -1) {
          			  error("recvfrom()");
        		}

				usrInGrpClientRecord.push_back(newIncomingMessage.CUser);
				string name(newIncomingMessage.CUser.name);
        		string ip(newIncomingMessage.CUser.ipAddr);
        		string notif=name+ " "+ip+":"+ToString(newIncomingMessage.CUser.portNumber);
        		strcpy(newIncomingMessage.newmsg.mess ,notif.c_str());
        
				if(!reStarting_AfterLE){
				mPrint.lock();      
        		printMessages(newIncomingMessage,false);
        		mPrint.unlock();
				}
    		}

	
	
	if(MULTICAST_STOP_CLIENT==1)
        MULTICAST_STOP_CLIENT=0;
	
	pthread_create( &thread_1 , NULL , receiver_handler,(void*) 0);
    pthread_create( &thread_2, NULL , sender_handler,(void*) 0);
    //pthread_create(&monCliMsg, NULL, monitorCliMessages, (void*) 0);
    pthread_create(&thread_4, NULL, leaderElection_handler, (void*) 0);
	
    pthread_join(thread_1,NULL);
    pthread_join(thread_2,NULL);
	
}

void *SendNewSeqMessageToClient(ChatUser initSeq, int tempUse_usrInGrpClientRecordCtr) {
    int leaderElectionSock; 
            
    if ( (leaderElectionSock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
                error("socket");
             }
            
    for(int i=0;i<tempUse_usrInGrpClientRecordCtr;i++) {
		
        if(usrInGrpClientRecord[i].leaderElectionPort != currentUser.leaderElectionPort) {
        
			struct sockaddr_in si_leaderElectionSock;
            socklen_t leaderElectionSock_slen=sizeof(si_leaderElectionSock);
            memset((char *) &si_leaderElectionSock, 0, sizeof(si_leaderElectionSock));
            si_leaderElectionSock.sin_family = AF_INET;
            si_leaderElectionSock.sin_port = htons(usrInGrpClientRecord[i].leaderElectionPort);
            
            if (inet_aton(usrInGrpClientRecord[i].ipAddr, &si_leaderElectionSock.sin_addr) == 0)  {
            fprintf(stderr, "inet_aton() failed\n");
            exit(1);
            }

            Messageobj NewMessage;
            NewMessage.newmsg.messageType=LE_NewSeqMessage;
            
            strcpy(NewMessage.newmsg.ip, currentUser.ipAddr);
            NewMessage.newmsg.port=currentUser.portNumber;
            
            if (sendto(leaderElectionSock, &NewMessage, sizeof(struct Messageobj), 0 , (struct sockaddr *) &si_leaderElectionSock, leaderElectionSock_slen)==-1) {
            error("sendto() 20");
            }
        }
    }
}

void *getInput(void *) {
	
  while(1){
    
	  if(inputFlag==0 && eofCheck==0){
		  
    	  getline(cin,mInput);  
      
		  if(cin.eof()==1){
          eofCheck=1;
          break;
      }
      else
	  {
		inputFlag=1;
	  }
    }
  }
}

void newGrpChat(ChatUser initSeq) {
	
	if(reStarting_AfterLE){
		cout<<"I am the new sequencer"<<endl;
	}
	
    int tempUse_usrInGrpClientRecordCtr=usrInGrpClientRecord.size();
    initialize();
    currentUser=initSeq;
    
	if(ExCredit_7b) AndroidInitialize();
	
	if(!reStarting_AfterLE){
	cout<<initSeq.name<<" started a new chat, listening on "<<initSeq.ipAddr<<":"<<initSeq.portNumber<<endl;
    cout<<"Succeeded, current users:"<<endl;
    cout<<initSeq.name<<" "<<initSeq.ipAddr<<"."<<initSeq.portNumber<<" (Leader)"<<endl;
	}
	
    string port=ToString(initSeq.portNumber);
    string t_name(initSeq.name);
    string t_ip(initSeq.ipAddr);
    
    welcome_mess = t_name + " " + t_ip + ":" + port + "(Leader)";
	if(!reStarting_AfterLE){
    cout<<"Waiting for others to join..."<<endl;
	}
    int i, recv_len;
    ack_slen= sizeof(si_ack);

    if ((sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        error("socket");
    }

    memset((char *) &si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(initSeq.portNumber);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	
    if( bind(sock , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) {
        error("bind old 5000");
    }

    if ((sockack=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        error("socket");
    }

    memset((char *) &si_ack, 0, sizeof(si_ack));

    si_ack.sin_family = AF_INET;
    si_ack.sin_port = htons(initSeq.hbportNumber);
    si_ack.sin_addr.s_addr = htonl(INADDR_ANY);

    if( bind(sockack , (struct sockaddr*)&si_ack, sizeof(si_ack) ) == -1) {
        error("bind 6000");
    }

    struct timeval tv1;
    tv1.tv_sec       = 1;
    tv1.tv_usec      = 1000;

    if (setsockopt(sockack, SOL_SOCKET, SO_RCVTIMEO,&tv1,sizeof(tv1)) < 0) {
        perror("Error");
    }
    
    if ((sendsock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        error("socket");
    }

    memset((char *) &si_send, 0, sizeof(si_send));

    si_send.sin_family = AF_INET;
    si_send.sin_port = htons(initSeq.sendportNumber);
    si_send.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if( bind(sendsock , (struct sockaddr*)&si_send, sizeof(si_send) ) == -1) {
        error("bind old 5000");
    }
    
    if (setsockopt(sendsock, SOL_SOCKET, SO_RCVTIMEO,&tv1,sizeof(tv1)) < 0) {
        perror("Error");
    }
    
    pthread_create( &thread_1, NULL , seq_receiver_handler,(void*) 0);
    pthread_create( &thread_2, NULL , seq_mess_sender_handler,(void*) 0);
    pthread_create( &thread_3, NULL , seq_ack_handler,(void*) 0);
	pthread_create( &multiCast, NULL ,multicast,(void*)0);
	
	if(ExCredit_1 || ExCredit_1a)
	{
	pthread_create( &trafficCheck, NULL ,TrafficCheck,(void*)0);
	}
	
	if(seqFailed==1){
        SendNewSeqMessageToClient(currentUser, tempUse_usrInGrpClientRecordCtr);
  }
	
   pthread_join(thread_1,NULL);
   pthread_join(thread_2,NULL);
   pthread_join(thread_3,NULL);
}

int main(int argc, char* argv[]) {

    pthread_create( &getLine , NULL , getInput,(void*) 0);
    srand(time(NULL));
    
    if(argc==2) {
		
		ChatUser initSeq;
        
        initSeq.isSequencer=true;
        strcpy(initSeq.ipAddr,getIP().c_str()); 
		
		int portStarting=5000;
		
        initSeq.portNumber=portStarting;
        initSeq.hbportNumber=portStarting+1;
        initSeq.sendportNumber=portStarting+2;
		
        string temp_name=argv[1];
        strcpy(initSeq.name, temp_name.c_str()); 
        
        initSeq.leaderPortNum=initSeq.portNumber;
        strcpy(initSeq.seqIpAddr,initSeq.ipAddr); 
        initSeq.UIRI++;

        string temp_ipAddr="127.0.0.1";
        strcpy(initSeq.ipAddr,temp_ipAddr.c_str());
        strcpy(initSeq.seqIpAddr,temp_ipAddr.c_str());
        
        newGrpChat(initSeq);

    } else if(argc==3) {
       
        pthread_create(&monCliMsg, NULL, monitorCliMessages, (void*) 0);

        ChatUser newUser;
        newUser.isSequencer=false;
        strcpy(newUser.ipAddr,getIP().c_str()); 
       
		int portStarting=1024+rand()%10000+rand()%1000+rand()%1000;
		
        newUser.portNumber=portStarting;
        newUser.hbportNumber=portStarting+1;
		newUser.sendportNumber=portStarting+2;
        newUser.leaderElectionPort=portStarting+3;
		
		string temp_name=argv[1];
        strcpy(newUser.name, temp_name.c_str()); 
               
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
        
        newUser.leaderHBPortNumber=newUser.leaderPortNum+1;
	    newUser.leaderSendPortNumber=newUser.leaderPortNum+2;
        
        string temp_ipAddr="127.0.0.1";
        strcpy(newUser.ipAddr,temp_ipAddr.c_str());
        strcpy(newUser.seqIpAddr,temp_ipAddr.c_str());
        
        existGrpChat(newUser);
        
		while(seqShifted==1 ||  seqFailed==1) {
		reStarting_AfterLE=true;
        if(seqShifted==1) {
            End_LE=1;
            close(newUser_s);       		
       	    close(socket_leaderElection);
			close(client_ack);
			close(client_s);
			
			existGrpChat(currentUser);            
        }
        
        if(seqFailed==1) {
            
            close(client_s);
            close(socket_leaderElection);
            close(newUser_s);
 			close(client_ack);
            
			End_CAckH=1;
            
            memset((char *) &newUser_si_other, 0, sizeof(newUser_si_other));
            memset((char *) &si_user, 0, sizeof(si_user));
            memset((char *) &si_ackclient, 0, sizeof(si_ackclient));
            memset((char *) &si_hb, 0, sizeof(si_hb));
            
			newGrpChat(currentUser);
         }
	 }
        
       return 0;
 }

    else {
        cout<<"Error:Invalid arguments"<<endl;
        return -1;
    }
}

//Common threads 

void *printMessages(Messageobj newMessage, bool recvFlag) {
    if(newMessage.newmsg.messageType==DATA) {
        
        if(newMessage.newmsg.seqNum==seqChk) {
            
			if(ExCredit_4 && (strcmp(currentUser.name, "ExCredit_4")!=0 )) {
					
				string encryptedMessage(newMessage.newmsg.mess);
				string decryptedMessage=decrypt(encryptedMessage);
				strcpy(newMessage.newmsg.mess, decryptedMessage.c_str());	
			}
		
			cout<<newMessage.newmsg.seqNum<<" - "<<newMessage.newmsg.name<<":: "<<newMessage.newmsg.mess<<endl;
            seqChk=seqChk+1;
			
			if(currentUser.clientType==ANDROID)
			{
				if (sendto(socket_Android, &newMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_Android, si_Android_slen) == -1) {
          		  error("recvfrom()");
       			 }
			}
			
			if(recvFlag) 
            {
            	int j;
             	for(j=0; j < holdBack.size(); j++) 
				{
                	if(holdBack[j].newmsg.seqNum==seqChk-1)
                    holdBack.erase(holdBack.begin()+j);
             	}
            }
        } 

        else {
            holdBack.push_back(newMessage); 
        }

            if(!holdBack.empty()) {
            
			 int i;
             for(i=0; i < holdBack.size(); i++) {
                if(holdBack[i].newmsg.seqNum==seqChk) {
                    printMessages(holdBack[i],true);
                }
             }
		}
	}
    if(newMessage.newmsg.messageType==NOTIFICATION) {
		
        cout<<newMessage.newmsg.mess<<endl;
        if(currentUser.clientType==ANDROID) {
				if (sendto(socket_Android, &newMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_Android, si_Android_slen) == -1) {
          		  error("recvfrom()");
       			 }
			}
        istringstream iss(newMessage.newmsg.mess);
        string token;

        int cnt=0;
        while (getline(iss, token, ' ')) {
            if(strcmp(token.c_str(), "joined")==0) {
				usrInGrpClientRecord.push_back(newMessage.CUser);
                break;
            }
			
			if(strcmp(token.c_str(), "left")==0) {
                for(int i=0;i<usrInGrpClientRecord.size();i++) {
					if(usrInGrpClientRecord[i].portNumber==newMessage.CUser.portNumber) {
						usrInGrpClientRecord.erase(usrInGrpClientRecord.begin()+i);
						break;
					}
				}
			}
        }
    }
}

void *SequencerVanished() {
	
	int becomeSeqFlag=0;
    leaderElectionCount=0;
	int leaderElectionSock;
	bool clientFailed=false;
	struct sockaddr_in si_leaderElectionSock;
    socklen_t leaderElectionSock_slen=sizeof(si_leaderElectionSock);
            
    if ( (leaderElectionSock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
                error("socket");
             }
	if(usrInGrpClientRecord.size()>1) {
	
   

    
    
	struct timeval LE_REP;
    LE_REP.tv_sec       = 2;
    LE_REP.tv_usec      = 1000;
		
	if (setsockopt(leaderElectionSock, SOL_SOCKET, SO_RCVTIMEO,&LE_REP,sizeof(LE_REP)) < 0) {
        perror("Error");
	}
		
		
	for(int i=0;i<usrInGrpClientRecord.size();i++) {
        if(usrInGrpClientRecord[i].leaderElectionPort != currentUser.leaderElectionPort) {
			
            memset((char *) &si_leaderElectionSock, 0, sizeof(si_leaderElectionSock));
            si_leaderElectionSock.sin_family = AF_INET;
            si_leaderElectionSock.sin_port = htons(usrInGrpClientRecord[i].leaderElectionPort);
            
            if (inet_aton(usrInGrpClientRecord[i].ipAddr, &si_leaderElectionSock.sin_addr) == 0)  {
            fprintf(stderr, "inet_aton() failed\n");
            exit(1);
            }

            Messageobj NewMessage;
            NewMessage.newmsg.messageType=LE_PortSend;
            
            string num=ToString(currentUser.portNumber);
            strcpy(NewMessage.newmsg.mess, num.c_str());
            NewMessage.CUser=currentUser;
            if (sendto(leaderElectionSock, &NewMessage, sizeof(struct Messageobj), 0 , (struct sockaddr *) &si_leaderElectionSock, leaderElectionSock_slen)==-1) {
            error("sendto() 9");
				
            }
            
           	if (recvfrom(leaderElectionSock, &NewMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_leaderElectionSock, &leaderElectionSock_slen) == -1) {
             	//error("recv()");
				usrInGrpClientRecord.erase(usrInGrpClientRecord.begin()+i);
				i--;
				clientFailed=true;
			}
			
			if(NewMessage.newmsg.messageType==LE_Reply || clientFailed) {
				if(strcmp(NewMessage.newmsg.mess, "LESSER")==0) leaderElectionCount++;
                if(leaderElectionCount==(usrInGrpClientRecord.size()-1)) {
                becomeSeqFlag=1;
                break;
               }
            }
         }
    }
	}
	else{
		
		becomeSeqFlag=1;
	}
    if(becomeSeqFlag==1) {
    
		//sleep(1); // To control testing for potential leader election crashing
		
		leaderElectionHappening=0;
        currentUser.isSequencer=true;
        currentUser.leaderPortNum=currentUser.portNumber;
        seqFailed=1;
        
		End_CR=1;
		End_CS=1;
		End_LE=1;
		End_MC=1;
	
		//self sending to kill thread	
		struct sockaddr_in selfLoop;
		
    	memset((char *) &selfLoop, 0, sizeof(selfLoop));
    	selfLoop.sin_family = AF_INET;
    	selfLoop.sin_port = htons(currentUser.portNumber);

    	if (inet_aton(currentUser.ipAddr, &selfLoop.sin_addr) == 0)  {
        	fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    	}
		
		Messageobj loopBack;	
		socklen_t len=sizeof(selfLoop);
		
		if (sendto(newUser_s, &loopBack, sizeof(struct Messageobj), 0 , (struct sockaddr *) &selfLoop, len)==-1) {
                error("sendto() self loop killing");
        }
		struct sockaddr_in selfLoopLE;
		
    	memset((char *) &selfLoopLE, 0, sizeof(selfLoopLE));
    	selfLoopLE.sin_family = AF_INET;
    	selfLoopLE.sin_port = htons(currentUser.leaderPortNum);

	    if (inet_aton(currentUser.ipAddr, &selfLoopLE.sin_addr) == 0)  {
    	    fprintf(stderr, "inet_aton() failed\n");
        	exit(1);
    	}
		
		Messageobj LE1;	
		len=sizeof(selfLoopLE);
		strcpy(LE1.newmsg.mess, "Killing LE");
		LE1.newmsg.messageType=-1;
		if (sendto(newUser_s, &LE1, sizeof(struct Messageobj), 0 , (struct sockaddr *) &selfLoopLE, len)==-1) {
                perror("sendto() self loop killing");
            }
		
		if (recvfrom(newUser_s, &LE1, sizeof(struct Messageobj), 0, NULL, NULL) == -1) {
             	perror("Recr from () leader thread");
        }   
    }
	else {
		
	}
	
	struct timeval LE_REP;
    LE_REP.tv_sec       = 0;
    LE_REP.tv_usec      = 0;
		
	if (setsockopt(leaderElectionSock, SOL_SOCKET, SO_RCVTIMEO,&LE_REP,sizeof(LE_REP)) < 0) {
        perror("Error");
	}
}

// Client Related threads
void *procCliRcvPkt(Messageobj processPkt,sockaddr_in recv_si) {
		
		int i;
		int isRepeat=0;
		int pointer=0;
		socklen_t sizerecv = sizeof(recv_si);

		string newMessageArrived(processPkt.newmsg.mess);
        if(newMessageArrived == "JOIN") {
            Message giveSEQIPData;
            strcpy(giveSEQIPData.mess, "INDIRECT");
            strcpy(giveSEQIPData.ip, currentUser.seqIpAddr);
            giveSEQIPData.port=currentUser.leaderPortNum;
            giveSEQIPData.messageType=INTERNAL;
			
			int leaderPortNum;
            string seqIpAddr;
            struct Messageobj newmess;
            newmess.newmsg=giveSEQIPData;

            if (sendto(newUser_s, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &recv_si, sizerecv)==-1) {
                error("sendto() 13");
            }
        }
		
		else if(newMessageArrived == "SLOW_DATA") {
			if(traffClntFlg==false)
				cout<<"NOTICE - > Sequencer requesting to slow down."<<endl;
			traffClntFlg=true;
		}
		else if(newMessageArrived == "RESUME_DATA")	{
			if(traffClntFlg==true)
				cout<<"NOTICE - > Sequencer requesting to resume."<<endl;
			traffClntFlg=false;
		}
        else {

            mPrint.lock();
            printMessages(processPkt,false);
			mPrint.unlock();
            	
		}
}

void *receiver_handler(void *) {	
   
	int pointer=0;
    int isRepeat=0;
    int i;
    struct Messageobj newIncomingMessage;
    struct sockaddr_in recv_si;
    struct Messageobj newmesg;
    Message acksend;
    acksend.messageType = RECVD;
    socklen_t sizerecv = sizeof(recv_si);
    
    while(End_CR==0) {
		
		isRepeat=0;

        if (recvfrom(newUser_s, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &recv_si, &sizerecv) == -1) {
          	break;
        }
		string name(currentUser.name);
		string ip(currentUser.ipAddr);
		string toCompare="NOTICE "+name+" joined on "+ip+":"+ToString(currentUser.portNumber);//+port;
		string newMessageArrivedChk(newIncomingMessage.newmsg.mess);
        sprintf( acksend.mess, "%ld", newIncomingMessage.newmsg.seqNum);
      
        newmesg.newmsg=acksend;
	
		if(newMessageArrivedChk!="JOIN"){
        if (sendto(newUser_s, &newmesg, sizeof(struct Messageobj), 0 , (struct sockaddr *) &recv_si, sizerecv)==-1) {
                error("sendto() 13");
            }
        }
		if(strcmp(newIncomingMessage.newmsg.mess, toCompare.c_str())==0) {
			   continue;
		   }
		
		if(newIncomingMessage.newmsg.messageType==DATA){
           
        if(newIncomingMessage.newmsg.tryNum>0){   
        	for(i=0;i<10;i++){
          	  if(seq_arr[i]==newIncomingMessage.newmsg.seqNum){
                isRepeat =1;
        		}
			}
		}
		if(isRepeat==0)	{
        	
			if(pointer<10){
            seq_arr[pointer] = newIncomingMessage.newmsg.seqNum;
            pointer++;
        }
        else{
            seq_arr[0] = newIncomingMessage.newmsg.seqNum;
            pointer=1;
            }
        }
      }
		if(!isRepeat)
            procCliRcvPkt(newIncomingMessage,recv_si);
	}
}

void* monitorCliMessages(void *) {
    
	int k;
    Messageobj sendMsg;
	int temp=0;

    while(cliMessageSend) {

    if(!cliMsgQueue.empty()){

        cliQueue.lock();

        for(k=0;k<cliMsgQueue.size();k++){

            //Logic to Stall Sending during Leader Election
            while(MULTICAST_STOP_CLIENT==1) {
                    if(temp==0 && MULTICAST_STOP_CLIENT==1) {
                        //cout<<"Stopping sending of messages"<<endl;
                        cliQueue.unlock();
                        temp=1;
                    }
            }
			
			if(MULTICAST_STOP_CLIENT==0 && temp==1) {
                cliQueue.lock();
                temp=0;
                //cout<<"Resuming sending of left over message"<<endl;
			}
	
    	    //End logic

            sendMsg=cliMsgQueue[k];
            
			if(ExCredit_4) {
			string msgToEncrypt(sendMsg.newmsg.mess);
			string encryptedMessage= encrypt(msgToEncrypt);
			strcpy(sendMsg.newmsg.mess, encryptedMessage.c_str());
			}
				
			client_send(sendMsg,newUser_si_other);
            cliMsgQueue.erase(cliMsgQueue.begin()+k);    
            
            k=-1; 
            nanosleep((const struct timespec[]){{0,100000000L}},NULL);
        }
        cliQueue.unlock();
	}
		nanosleep((const struct timespec[]){{0,500000000L}},NULL);
  }
}

void *sender_handler(void *) {
    
	int iteratorValue=0;
	if(ExCredit_1 ||ExCredit_1a || ExCredit_2) {
	 	iteratorValue=300;
	}
	if(strcmp(currentUser.name, "FLOOD_1")==0||strcmp(currentUser.name, "FLOOD_2")==0 || strcmp(currentUser.name, "FLOOD_3")==0)	 
		iteratorValue=5000;
	
	
	if(ExCredit_3a){
			
		
		if( (strcmp(currentUser.name, "ExCredit_3_DATA_C1")==0) || (strcmp(currentUser.name, "ExCredit_3_DATA_C2")==0) || (strcmp(currentUser.name, "ExCredit_3_DATA_C3")==0)){
		
		Message msgg;
		msgg.messageType=DATA;

        strcpy(msgg.mess, "DATA TO TEST ExCredit : ");
		strcat(msgg.mess, currentUser.name);
    	strcpy(msgg.name, currentUser.name);

		struct Messageobj newmess;
        newmess.newmsg=msgg;
		newmess.CUser=currentUser;
			
		time_t now = time(0);
		tm *ltm = localtime(&now);
				
			while(1){
		
			if(ltm->tm_hour==Mhour&&ltm->tm_min==Mmin&&ltm->tm_sec==Msec) {
					break;
			}
			now = time(0);
			ltm = localtime(&now);
			}
		 
			
			cliQueue.lock();
        	cliMsgQueue.push_back(newmess);
			cliQueue.unlock();
		}
	}
		
	
	
	if(strcmp(currentUser.name, "FLOOD_1")==0||strcmp(currentUser.name, "FLOOD_2")==0 || strcmp(currentUser.name, "FLOOD_3")==0)
	   {
	       sleep(10);
		   cout<<"Flooding in 3"<<endl;
		   sleep(3);
		   Message msgg1;
		   msgg1.messageType=DATA;
		   strcpy(msgg1.name, currentUser.name);
		   struct Messageobj newmess1;
           newmess1.CUser=currentUser;
		
       	   for(int i=0;i<iteratorValue;i++){
			
			strcpy(msgg1.mess, "Hi this is flooding  = ");
            strcat(msgg1.mess, currentUser.name);
            strcat(msgg1.mess, "  ");
			strcat(msgg1.mess, ToString(i).c_str());
		    newmess1.newmsg=msgg1;
           
            cliQueue.lock();
            cliMsgQueue.push_back(newmess1);
           	cliQueue.unlock();
            nanosleep((const struct timespec[]){{0,100000000L}},NULL);

        }
		
       End_CS==1;
}
    while(End_CS==0) {		
		
		char send[1024];
        Message msgg;
	
        if(inputFlag==1){
        
        	strcpy(send, mInput.c_str());
        	msgg.messageType=DATA;

        	if(strcmp(send, "")!=0) {
		     	strcpy(msgg.mess, send);
    	        strcpy(msgg.name, currentUser.name);

			struct Messageobj newmess;
        	newmess.newmsg=msgg;
			newmess.CUser=currentUser;
       
       		cliQueue.lock();
        	cliMsgQueue.push_back(newmess);
			cliQueue.unlock();
		}
        inputFlag=0;
	}
        
    if(eofCheck==1) {
        eofCheck=0;
        exit(0);
    } 
  }
}

void client_send(Messageobj newmess,sockaddr_in newUser_si_other){

    int t=0;
    struct sockaddr_in si_recv;
	socklen_t send_len=sizeof(newUser_si_other);
	int ret=-1;
	if(traffClntFlg){
	nanosleep((const struct timespec[]){{0,500000000L}},NULL);
	}

    pktCount=pktCount+1;
    newmess.newmsg.pktNum=atoi((ToString(pktCount)+ToString(currentUser.portNumber)).c_str());

    newmess.newmsg.tryNum=0;
    PARENTMUTEX_Client.lock();
   
	while(t<3){
      
		if (sendto(client_ack, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &newUser_si_other,send_slen)==-1) {
                error("sendto() 12 A");
            }
        if(recvfrom(client_ack, &newmess, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_recv, &send_slen) == -1){
               	newmess.newmsg.tryNum=newmess.newmsg.tryNum+1;
                //ClientSendFile.lock();
                //file_client.open("File_ClientSending.txt", ios::app);
                //file_client<<"Try = "<<ToString(t)<<" "<<newmess.newmsg.mess<<endl;
                //file_client.close();
                //ClientSendFile.unlock();
		}
		if((int)ntohs(si_recv.sin_port)==(int)ntohs(newUser_si_other.sin_port)&& newmess.newmsg.messageType==RECVD){
			break;
			}
		
		t++;
		nanosleep((const struct timespec[]){{0,250000000L}},NULL);
	}
	
	if(t==3 && newmess.newmsg.messageType!=RECVD){
        //ClientSendFile.lock();
        //cout<<"look for recvfrom 12 A"<<endl;
        //file_client.open("File_ClientSending.txt", ios::app);
        //file_client<<"TOTAL FAILURE_____________"<<endl;
        //file_client<<newmess.newmsg.mess<<endl;
        //file_client.close();
        //ClientSendFile.unlock();
    }

     PARENTMUTEX_Client.unlock();
}

void *client_ack_handler(void *) {

    struct sockaddr_in heartBeatChk;
	socklen_t hrtChkLen=sizeof(heartBeatChk);
	
    int sFailed=0;
	
	
	
	
	struct timeval tv1;
    tv1.tv_sec       = 4;
    tv1.tv_usec      = 1000;

    if (setsockopt(client_s, SOL_SOCKET, SO_RCVTIMEO, &tv1, sizeof(tv1)) < 0) {
        			perror("Error");
    }
	
    while(End_CAckH==0) {
		
		
		while(!inactiveChat){ 
			if(beat) { 
				cout<<"Terminating DCHAT. Clearing all resources. Going to abort"<<endl;
				abort();
				
				}
				
			
		}
		
		
		
		
		string m="";
        char send[1024];
        strcpy(send, "ack");
        Message msgg;
		strcpy(msgg.mess,"I AM ALIVE!");
        msgg.messageType=ACK;

        struct Messageobj newmess;
        newmess.newmsg=msgg;
		newmess.CUser=currentUser;
        
		if (recvfrom(client_s, &newmess, sizeof(struct Messageobj), 0, (struct sockaddr *) &heartBeatChk, &hrtChkLen) == -1) {
            //cout<<"seq failed"<<endl;
            MULTICAST_STOP_CLIENT=1;
            sFailed=1;
			leaderElectionHappening=1;
			break;
        }
		
		strcpy(msgg.mess,"I AM ALIVE!   = ");
		strcat(msgg.mess, currentUser.name);
        newmess.newmsg=msgg;
		newmess.CUser=currentUser;
		
		if(newmess.newmsg.mess!="FAKE"){
        if (sendto(client_s, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &heartBeatChk,hrtChkLen)==-1) {
            error("sendto() 6");
        }
	}
		
}
    if(sFailed==1) SequencerVanished();
	while(leaderElectionHappening==1){}
	while(potentialLeaderVanished==1){
		potentialLeaderVanished=0;
		SequencerVanished();
	}
}

void *leaderElection_handler(void *) {
	struct timeval tv_potentialCrashing;
    tv_potentialCrashing.tv_sec       = 2;
   	tv_potentialCrashing.tv_usec      = 1000;
	
	int numberOfMessageGot=0, beginSwitch=0;
    
    struct Messageobj newSeqData;
	
	//Potential Leader
	int potentialLeaderPort=0;
	ChatUser potentialLeader_User;
    
    while(End_LE==0) {

        struct Messageobj newmess;

		if (recvfrom(socket_leaderElection, &newmess, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_leaderElection, &si_leaderElection_slen) == -1) {
               
			//cout<<"THe potential leader has vanished i guess"<<endl;
			potentialLeaderVanished=1;
			
			tv_potentialCrashing.tv_sec       = 0;
   	        tv_potentialCrashing.tv_usec      = 0;
			if (setsockopt(socket_leaderElection, SOL_SOCKET, SO_RCVTIMEO,&tv_potentialCrashing,sizeof(tv_potentialCrashing)) < 0) {
       			 perror("Error");
   			  }	
			usrInGrpClientRecord.erase(usrInGrpClientRecord.begin()+potentialLeaderPort);
			numberOfMessageGot=0;
			beginSwitch=0;
    
    		potentialLeaderPort=0;
			leaderElectionHappening=0;
        }
		if(strcmp(newmess.newmsg.mess, "Killing LE")==0){
			if (sendto(socket_leaderElection, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &si_leaderElection, si_leaderElection_slen)==-1) {
                error("sendto() self loop killing");
            }
		}
        
		if(newmess.newmsg.messageType==LE_PortSend) {
			numberOfMessageGot++;
			int clientPortToCompare=atoi(newmess.newmsg.mess);
            Messageobj replyMsg;
			replyMsg.newmsg.messageType=LE_Reply;
			
			if(clientPortToCompare>currentUser.portNumber){
                strcpy(replyMsg.newmsg.mess, "LESSER");
            }
            else if (clientPortToCompare<currentUser.portNumber){
               strcpy(replyMsg.newmsg.mess, "HIGHER");
            }
			else if (clientPortToCompare==currentUser.portNumber){
				string ipAddressClient(newmess.CUser.ipAddr);
				string ipAddressMyself(currentUser.ipAddr);
				vector<string> IP_Client = split(ipAddressClient, '.');
				vector<string> IP_Myself = split(ipAddressClient, '.');
				if(atoi(IP_Client[3].c_str())>atoi(IP_Myself[3].c_str())) {
					strcpy(replyMsg.newmsg.mess, "LESSER");
				}
				else {
					 strcpy(replyMsg.newmsg.mess, "HIGHER");
				}
			}
			if (sendto(socket_leaderElection, &replyMsg, sizeof(struct Messageobj), 0 , (struct sockaddr *) &si_leaderElection, si_leaderElection_slen)==-1) {
                     error("sendto() 8");
                }
			
			if(numberOfMessageGot==(usrInGrpClientRecord.size()-1)){
			
				int leadPort=0;
				for(int i=0;i<usrInGrpClientRecord.size();i++){
        				if(usrInGrpClientRecord[i].portNumber>leadPort){
							potentialLeaderPort=i;
							leadPort=usrInGrpClientRecord[i].portNumber;
						    potentialLeader_User=usrInGrpClientRecord[i];
						}
				}
				
				if(potentialLeader_User.portNumber!=currentUser.portNumber){
					if (setsockopt(socket_leaderElection, SOL_SOCKET, SO_RCVTIMEO,&tv_potentialCrashing,sizeof(tv_potentialCrashing)) < 0) {
       			 	perror("Error");
   			  	}		
				}
			}
		}
        
        if(newmess.newmsg.messageType==LE_NewSeqMessage){
            newSeqData=newmess;
            seqShifted=1;
            break;
        }
    }

    if(seqShifted==1) {
    	currentUser.leaderPortNum=newSeqData.newmsg.port;
        strcpy(currentUser.seqIpAddr, newSeqData.newmsg.ip);
        seqShifted=1;
        
        End_CR=1;
		End_CS=1;
		End_CAckH=1;
		End_MC=1;
		
		//self sending to kill thread	
		struct sockaddr_in selfLoop;
		
    	memset((char *) &selfLoop, 0, sizeof(selfLoop));
    	selfLoop.sin_family = AF_INET;
    	selfLoop.sin_port = htons(currentUser.portNumber);

    	if (inet_aton(currentUser.ipAddr, &selfLoop.sin_addr) == 0)  {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    	}
		
		Messageobj loopBack;	
		socklen_t len=sizeof(selfLoop);
		
		if (sendto(newUser_s, &loopBack, sizeof(struct Messageobj), 0 , (struct sockaddr *) &selfLoop, len)==-1) {
                error("sendto() self loop killing");
            }
	}
}

//Sequencer Related threads

void procSeqRcvPkt(Messageobj processPkt,sockaddr_in recv_si){
	
	struct Messageobj newIncomingMessage;
    newIncomingMessage=processPkt;
	struct Messageobj newMsgForMulti;
	Message newNotice;
	int i, isRepeat=0,pointer=0;
	struct Messageobj newmesg;
	Message acksend;
	acksend.messageType = RECVD;
	socklen_t sizerecv = sizeof(recv_si);
	
	for(int ctr=0;ctr<usrInGrpSeqRecord.size();ctr++) {
		if(usrInGrpSeqRecord[ctr].portNumber==newIncomingMessage.CUser.portNumber) {
			usrInGrpSeqRecord[ctr].currentMsgCount=usrInGrpSeqRecord[ctr].currentMsgCount+1;
		}
	}
	
	if(isRepeat==0) {
	
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
			newmess.CUser=currentUser;
            if (sendto(sock, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &recv_si, newUser_slen)==-1) {
                error("sendto() 1 ");
            }
			
       		newNotice.messageType=NOTIFICATION;
       		string name(newIncomingMessage.newmsg.name);
        	string ip(newIncomingMessage.newmsg.ip);
        	string notif_msg="";
        	notif_msg="NOTICE "+name+" joined on "+ip+":"+ToString(newIncomingMessage.newmsg.port);//+port;
        	strcpy(newNotice.mess ,notif_msg.c_str());
       		newMsgForMulti.newmsg=newNotice;
        	newMsgForMulti.CUser=newIncomingMessage.CUser;
			
            addToMultiCastDS(newIncomingMessage, recv_si);
			
        } 
	
		else if(newMessageArrived == "group_leave") {
            int clientid2;
            int clientid = (int)ntohs(si_other.sin_port);
            for(int i=0; i<usrInGrpSeqRecord.size(); i++) {
                if(usrInGrpSeqRecord[i].portNumber==(int)ntohs(si_other.sin_port)) {
                    clientid2 = usrInGrpSeqRecord[i].hbportNumber;
                    break;
                }
            }
			removefrommulticast(clientid,user_name);
         
        } 
		else {
				if(newIncomingMessage.newmsg.messageType==DATA) {     
					seqUpdate.lock();
                	seq=seq+1;				 			 			 
              		newIncomingMessage.newmsg.seqNum=seq;
                	newIncomingMessage.newmsg.port=(int)ntohs(si_other.sin_port);
					newMsgForMulti=newIncomingMessage;
					seqUpdate.unlock();
				}
		}
		
		if(newMessageArrived!="group_leave"){
		  priorityQueue.lock();
		  priority_msg.push_back(newMsgForMulti);
		  priorityQueue.unlock();
        }
	}
}

void *TrafficCheck(void *){
	cout<<"Starting traffic control thread"<<endl;
	struct Messageobj trafficObj;
	
	while(1){
	for(int ctr=0;ctr<usrInGrpSeqRecord.size();ctr++){
	
		int currMesCount=usrInGrpSeqRecord[ctr].currentMsgCount;
		int prevMesCount=usrInGrpSeqRecord[ctr].prevMsgCount;
		
		if((currMesCount-prevMesCount)>15) {
					mPrint.lock();
					cout<<"User = "<<usrInGrpSeqRecord[ctr].name<<" has high incoming traffic."<<endl;
					mPrint.unlock();
				 	
					Message trafficMsg;
					trafficMsg.messageType=TRAFFIC_CNTRL;
					strcpy(trafficMsg.mess,"SLOW_DATA");
					trafficObj.newmsg=trafficMsg;
					
				    usrInGrpSeqRecord[ctr].trafficFlag=true;
					seq_send(trafficObj,usrInGrpSeqRecord[ctr].multicastSockAddr);
		}
		else {
					Message trafficMsg;
					trafficMsg.messageType=TRAFFIC_CNTRL;
					strcpy(trafficMsg.mess,"RESUME_DATA");
					trafficObj.newmsg=trafficMsg;
					usrInGrpSeqRecord[ctr].trafficFlag=false;
					seq_send(trafficObj,usrInGrpSeqRecord[ctr].multicastSockAddr);
		}
		usrInGrpSeqRecord[ctr].prevMsgCount=currMesCount;
	}
	sleep(5);
	}
}

void *seq_receiver_handler(void *){
    struct Messageobj newIncomingMessage;
	struct sockaddr_in recv_si;
	struct Messageobj newmesg;
    int isRepeat=0;
	Message acksend;
	acksend.messageType = RECVD;
	int pointer=0;
	int recv_len;
	socklen_t sizerecv = sizeof(recv_si);
	
    while(1) {
        isRepeat=0;

        if ((recv_len = recvfrom(sock, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &recv_si, &slen)) == -1) {
            error("recvfrom() 1");
        }

     	sprintf( acksend.mess, "%ld", newIncomingMessage.newmsg.seqNum);
		newmesg.newmsg=acksend;
		
		if(strcmp(newIncomingMessage.newmsg.mess,"JOIN")!=0){
			if (sendto(sock, &newmesg, sizeof(struct Messageobj), 0 , (struct sockaddr *) &recv_si, sizerecv)==-1) {
                error("sendto() 13");
            }
		}
		
		if(strcmp(newIncomingMessage.newmsg.mess,"JOIN")!=0){
	        if(newIncomingMessage.newmsg.tryNum>0){   

    		    for(i=0;i<10;i++){
            		if(cli_arr[i]==newIncomingMessage.newmsg.pktNum){
                		isRepeat =1;
        			}
    			}
			}	
        if(isRepeat==0) {
        if(pointer<10){
            cli_arr[pointer] = newIncomingMessage.newmsg.pktNum;
            pointer++;
        }
        else{
            cli_arr[0] = newIncomingMessage.newmsg.pktNum;
            pointer=1;
            }
        }

        }
      
    	if(isRepeat==0) {
            procSeqRcvPkt(newIncomingMessage,recv_si);
        }
    }
}

void *seq_ack_handler(void *){
    int flag=2;
	int cnt=0;
    while(1) {

        string m="";
        char send[1024];
        strcpy(send, "ack");
        Message msgg;
		strcpy(msgg.mess,"ALIVE1?");
        msgg.messageType=ACK;

        struct Messageobj newmess;
        newmess.newmsg=msgg;
        string clientname;
		newmess.CUser=currentUser;
        
		for(int i=0; i<usrInGrpSeqRecord.size(); i++) {
				
				if (sendto(sockack, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &usrInGrpSeqRecord[i].hbSockAddr, si_hb_slen)==-1) {
                    error("sendto() 19");
                }

                struct sockaddr_in si_ack_other;
				
                if (recvfrom(sockack, &newmess, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_ackclient, &ack_slen) == -1) {
					 int cnt=0;
					 for(int j=0; j<usrInGrpSeqRecord.size(); j++) {
                        if(usrInGrpSeqRecord[j].hbportNumber==(int)ntohs(usrInGrpSeqRecord[i].hbSockAddr.sin_port)) {
                            cnt=j;
                            clientname = usrInGrpSeqRecord[j].name;
                        }
                    }
                    int clientid1 = (int)ntohs(usrInGrpSeqRecord[cnt].multicastSockAddr.sin_port);
                    removefrommulticast(clientid1, clientname);
                    i--;
                }
            }
				cnt++;
        }
}

void *seq_mess_sender_handler(void *){
	int flag=0;
	
	while(1) {

        char send[1024];
		if(inputFlag==1) {
       
        strcpy(send, mInput.c_str());
        struct Messageobj newmess;
        if(strcmp(send, "")!=0) {
			
            Message msgg;
            msgg.messageType=DATA;
       		strcpy(msgg.mess, send);
            strcpy(msgg.name, currentUser.name);
            seqUpdate.lock();
            seq=seq+1;
            msgg.seqNum=seq;
            newmess.newmsg=msgg;
            newmess.newmsg=msgg;
			newmess.CUser=currentUser;
			seqUpdate.unlock();
			if(ExCredit_4)
			{
				string msgToEncrypt(newmess.newmsg.mess);
				string encryptedMessage= encrypt(msgToEncrypt);
				strcpy(newmess.newmsg.mess, encryptedMessage.c_str());		
			}
			priorityQueue.lock();
			priority_msg.push_back(newmess);
			priorityQueue.unlock();
			inputFlag=0;
        }
	 }
       if(eofCheck==1) {
        eofCheck=0;
        exit(0);
      }
    }
}

void seq_send(Messageobj newmess,sockaddr_in sendTo){

    struct Messageobj newAckSend;
    struct sockaddr_in si_recv;
    socklen_t slen=sizeof(sendTo);
    int t=0;
    
  	newmess.newmsg.tryNum=0;
  	PARENTMUTEX_Seq.lock();
 	
	while(t<3){

    if (sendto(sendsock, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &sendTo, slen)==-1) {
                error("sendto() 12 ");
    }

    if(recvfrom(sendsock, &newAckSend, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_recv, &slen) == -1){
          
				 //SeqMCPrint.lock();
                 //SeqMCPrint_File.open("File_SeqTries.txt", ios::app);
                 //SeqMCPrint_File<<"Seq num = "<<newmess.newmsg.seqNum<<" "<<newmess.newmsg.mess<<endl;
                 //SeqMCPrint_File.close();
                 //SeqMCPrint.unlock();
                 newmess.newmsg.tryNum=newmess.newmsg.tryNum+1;
	}
    if((int)ntohs(si_recv.sin_port)==(int)ntohs(sendTo.sin_port)&& newAckSend.newmsg.messageType==RECVD){
            break;
    }
        
	t++;
	nanosleep((const struct timespec[]){{0,100000000L}},NULL);
    }


	if(t==3 && newmess.newmsg.messageType!=RECVD){
       //seqSendFile.lock();
       //file_seq.open("File_SeqTries.txt", ios::app);
       //file_seq<<"TOAL FAILURE - "<<"Seq num = "<<newmess.newmsg.seqNum<<" "<<endl;
       //file_seq.close();
       //seqSendFile.unlock();
    }
 PARENTMUTEX_Seq.unlock();

	
}

void *addToMultiCastDS(Messageobj newMessage, sockaddr_in si_other) {
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
        
    newMessage.CUser.hbSockAddr=create_sock_struct(newMessage);
    newMessage.CUser.multicastSockAddr=si_other;
    	
	userInGrpSeq.lock();
	usrInGrpSeqRecord.push_back(newMessage.CUser);
	userInGrpSeq.unlock();
	
    //Sending number of users
    char message[1024];
    strcpy(message,ToString(usrInGrpSeqRecord.size()).c_str());

    if (sendto(sock, &message, sizeof(message), 0 , (struct sockaddr *) &si_other, newUser_slen)==-1) {
        error("sendto() 5");
    }

    //Sending User Data
    struct Messageobj existUserList;
    for(int i=0; i<usrInGrpSeqRecord.size(); i++) {
        
        existUserList.newmsg.messageType=NOTIFICATION;
        existUserList.CUser=usrInGrpSeqRecord[i];
        
        if (sendto(sock, &existUserList, sizeof(struct Messageobj), 0 , (struct sockaddr *) &si_other, newUser_slen)==-1) {
            error("sendto() 4");
        }
    }
}

void multicastSpawn(Messageobj newMessage){
		
		if(usrInGrpSeqRecord.size()==0) printMessages(newMessage,false);
	    for(int i=0; i<usrInGrpSeqRecord.size(); i++) {
			if(i==0) printMessages(newMessage,false);
			seq_send(newMessage,usrInGrpSeqRecord[i].multicastSockAddr);
    }
}

void *multicast(void *){
	
	//int Mhour=21;
	//int Mmin=52;
	//int Msec=30;
	int prevMsgSize=0;
	int currMsgSize=0;
	
	int FIFO_FAIR=0;
	
	while(End_MC==0 ) {
    
		Messageobj sendMsg;
		int k;
		bool chkFlag=false;
	
		if(!priority_msg.empty()){
	
     		if(ExCredit_2){

				currMsgSize=priority_msg.size();
				//cout<<"Current Msg Size = "<<currMsgSize<<endl;
				//cout<<"Prev Msg Size = "<<prevMsgSize<<endl;
				int diff=currMsgSize-prevMsgSize;
				//cout<<"Diffeence = "<<diff<<endl;
				if(diff>15) FIFO_FAIR=true;
				else FIFO_FAIR=false;
			}		
	if(!FIFO_FAIR){
		
        //cout<<"Entering FIFO queing "<<endl;
		
		priorityQueue.lock();
		
		for(k=0;k<priority_msg.size();k++){

		if(priority_msg[k].newmsg.messageType==NOTIFICATION){
			sendMsg=priority_msg[k];
			multicastSpawn(sendMsg);
			priority_msg.erase(priority_msg.begin()+k);
            nanosleep((const struct timespec[]){{0,100000000L}},NULL);
			k=-1;
		}
	}

	for(k=0;k<priority_msg.size();k++){
		if(priority_msg[k].newmsg.messageType==DATA){
			sendMsg=priority_msg[0];
			multicastSpawn(sendMsg);
		    priority_msg.erase(priority_msg.begin());
			printMessages(sendMsg,false);
			nanosleep((const struct timespec[]){{0,100000000L}},NULL);
			k=-1;
		}
	}
		priorityQueue.unlock();
	
	}
		
	else
	   {
		
		cout<<"Fair Queing in Progress"<<endl;
		
	    priorityQueue.lock();
		
		while(!priority_msg.empty()) {
	   
			int cnt=0;
	   		for(int l=0; l<usrInGrpSeqRecord.size(); l++) {
		   
	   			for(k=0;k<priority_msg.size();k++){
		   
					if(usrInGrpSeqRecord[l].portNumber==priority_msg[k].CUser.portNumber) 
			{
					sendMsg=priority_msg[k];
					printMessages(sendMsg,false);
					multicastSpawn(sendMsg);
					priority_msg.erase(priority_msg.begin()+k);
					k--;		
					cnt++;
			
					if(cnt==5)
					break;
				}
			}
		   cnt=0;
	 }
	}
		prevMsgSize=priority_msg.size();
		priorityQueue.unlock();
	}
		
		
		if(ExCredit_3a || ExCredit_2a){
			cout<<"Multicast paused for 10 secs to allow fill up"<<endl;
			sleep(10);
		}
		if(ExCredit_3 || ExCredit_2){
			cout<<"Multicast paused for 30 secs to allow fill up"<<endl;
			sleep(30);
		}
		if(ExCredit_1)	{
			cout<<"Multicast paused for 10 secs to allow fill up"<<endl;
			sleep(10);
		}
	 
	}
		sleep(1);
	}
	
}

void *removefrommulticast(int client_remove,string name){
	ChatUser temp;
	
    int t=0;
    for(int i=0; i<usrInGrpSeqRecord.size(); i++) {
        if((int)ntohs(usrInGrpSeqRecord[i].multicastSockAddr.sin_port)==client_remove) {
          temp=usrInGrpSeqRecord[i];
		  
		  userInGrpSeq.lock();
		  usrInGrpSeqRecord.erase(usrInGrpSeqRecord.begin()+i);
		  userInGrpSeq.unlock();
		  
		  break;	
        } 
    }

    Message newNotice;
    newNotice.messageType=NOTIFICATION;

    string notif_msg="";
    notif_msg="NOTICE "+name+" left the chat or crashed ";
    strcpy(newNotice.mess ,notif_msg.c_str());
    
    struct Messageobj newNotifMessage;
    newNotifMessage.newmsg=newNotice;
    newNotifMessage.CUser=temp;
	priorityQueue.lock();
	priority_msg.push_back(newNotifMessage);
	priorityQueue.unlock();
}

//Android
void *android_interface_receiver_handler(void *){
	while(1){
	struct Messageobj newIncomingMessage;
	if (recvfrom(socket_Android, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_Android, &si_Android_slen) == -1) {
            error("recvfrom()");
        }
		newIncomingMessage.CUser=currentUser;
		if(currentUser.isSequencer)	{
			seqUpdate.lock();
            seq=seq+1;
            newIncomingMessage.newmsg.seqNum=seq;
            seqUpdate.unlock();
			priorityQueue.lock();
			priority_msg.push_back(newIncomingMessage);
			priorityQueue.unlock();
		}
		
		else{
			cliQueue.lock();
       		cliMsgQueue.push_back(newIncomingMessage);
		    cliQueue.unlock();
		}
	}
}
