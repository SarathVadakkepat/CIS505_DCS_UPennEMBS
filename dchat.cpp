
/**
Description: CIS 505 Project 3
Authors: Karthik Anantha Ram, Sanjeet Phatak and Sarath Vadakkepat
**/

#include "ChatServer.h"
#include <time.h>
//Compulsory needed variables and cross checked 

ChatUser currentUser;

pthread_t thread_1,thread_2, thread_3, thread_4 , thread_5, thread_6, getLine;
int killThread_A=0,killThread_B=0,killThread_C=0,killThread_D=0,killThread_E=0,killThread_F=0;

//Class for users in a group chat
//struct ChatUser usrInGrpClientRecord[CHATUSERS_COUNT];
//int usrInGrpClientRecordCtr=0;

struct ChatUser usrInGrpSeqRecord[CHATUSERS_COUNT];
int usrInGrpSeqRecordCtr=0;

vector<ChatUser> usrInGrpClientRecord;
vector<Messageobj> priority_msg;

unsigned long int seq=0;
unsigned long int seqChk=1;

int seqNext=0;
mutex mPrint;

vector<Messageobj> holdBack;

int leaderElectionCount;
int seqShifted=0;
int seqFailed=0;
string mInput="";

int inputFlag=0;
unsigned long int seq_arr[100];
unsigned long int notecnt=0;
int eofCheck=0;

int leaderElectionHappening=0;int numberOfMessageGot=0;

int initializingNewClient_RH, initializingNewClient_SH, signalNewClient_RH;

int End_CR, End_CS,End_LE,End_CAckH;
//End

string welcome_mess;

void *KillAllThreads(void *)
{
    if(killThread_A!=0) {
    while(pthread_kill(thread_1, 0) == 0) {
            pthread_cancel(thread_1);
         }
    }
    if(killThread_B!=0) {
        
        while(pthread_kill(thread_2, 0) == 0) {
            pthread_cancel(thread_2);
        }
    }
    if(killThread_C!=0) {
        while(pthread_kill(thread_3, 0) == 0) {
            pthread_cancel(thread_3);
            
        }
    }  
    
    if(killThread_D!=0) {
        while(pthread_kill(thread_4, 0) == 0) {
            pthread_cancel(thread_4);
            
        }
    }  
	
	if(killThread_E!=0) {
        while(pthread_kill(thread_4, 0) == 0) {
            pthread_cancel(thread_4);
            
        }
    }  
	
	
    killThread_A=0;killThread_B=0;killThread_D=0;killThread_C=0;
    pthread_exit(NULL);
}

void initialize() {
    
    usrInGrpSeqRecordCtr=0;
    
    seq=0;
    seqChk=1;
	leaderElectionHappening=0;
	numberOfMessageGot=0;
	
	inputFlag=0;
	eofCheck=0;
	
	usrInGrpClientRecord.clear();
	
	leaderElectionCount=0;
	initializingNewClient_RH=0;
	initializingNewClient_SH=0;
	signalNewClient_RH=0;
	
	End_CR=0;
	End_CS=0;
	End_LE=0;
	End_CAckH=0;	
	//mPrint.unlock();
}

void PrintClientRecord()
{

	 for(int i=0;i<usrInGrpClientRecord.size();i++)
	 {
	
		 cout<<usrInGrpClientRecord[i].portNumber<<endl;
	 }
}

void PrintSeqRecord()
{

	
	 for(int i=0;i<usrInGrpSeqRecordCtr;i++)
	 {
		
		 cout<<usrInGrpSeqRecord[i].portNumber<<endl;
		 cout<<usrInGrpSeqRecord[i].hbportNumber<<endl;
	 }
}


void wait(int s){
clock_t endwait;
endwait=clock()+s*CLOCKS_PER_SEC;
	while(clock()<endwait){}
}


//Method to enable a user join a existing chat
void existGrpChat(ChatUser newUser)
{
	
	cout<<"__________REJOININIG_____________"<<endl;
    initialize();
    currentUser=newUser;
   
	/*
	// Struct and binding for Android Clients
    
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
        
       cout<<"OOps didnt reveing in time"<<endl;
		android=0;
    }
    //Struct or Android
	
	
	if(android==1)
	{
		cout<<"android user"<<endl;
		
		pthread_t androidRecv, androidSend;
		pthread_create( &androidRecv , NULL , android_interface_receiver_handler,(void*) 0);
       
		currentUser.clientType=ANDROID;
		
		 tv2.tv_sec       = 0;
   		 tv2.tv_usec      = 0;
			if (setsockopt(socket_Android, SOL_SOCKET, SO_RCVTIMEO,&tv2,sizeof(tv2)) < 0) {
        perror("Error");
    		}
	}
	
	else
		
	{
	 cout<<"Not an android user"<<endl;
		close(socket_Android);
	}
	
	
	*/
	
	
	
	
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
    tv_ack.tv_sec       = 1;
    tv_ack.tv_usec      = 1000;
	
	si_clientsend.sin_family = AF_INET;
    si_clientsend.sin_port = htons(newUser.portNumber+2);
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
    si_ackclient.sin_port = htons(6000);

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
	
	int tru =1;
	setsockopt(client_s,SOL_SOCKET,SO_REUSEADDR,&tru,sizeof(int));

	// Struct and binding for Leader Election
    
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
    
		
	
    //Struct or LE complete.
	 pthread_create( &thread_3, NULL , client_ack_handler,(void*) 0);
	
	
	
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
    
    		if (sendto(newUser_s, &newMessageObj, sizeof(struct Messageobj), 0 , (struct sockaddr *) &newUser_si_other, newUser_slen)==-1) {
        		error("sendto() 2");
    		}
			//cout<<"Sending join to seq
			 struct timeval tv;
   			 tv.tv_sec       = 5;
    		 tv.tv_usec      = 1000;

    		 if (setsockopt(newUser_s, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
        			perror("Error");
    		}
			
			 struct Messageobj newIncomingMessage;

    		 if (recvfrom(newUser_s, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &newUser_si_other, &newUser_slen) == -1) {
      			 cout<<"Sorry, no chat is active on "<<currentUser.seqIpAddr<<":"<<currentUser.leaderPortNum<<" try again later.Bye."<<endl;
       			exit(-1);
   			 }
			
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
        		cout<<"indirect "<<endl;
				
				
				End_CAckH=1;
				
				struct sockaddr_in selfLoop;
		
    			memset((char *) &selfLoop, 0, sizeof(selfLoop));
    			selfLoop.sin_family = AF_INET;
				cout<<"port hb "<<currentUser.hbportNumber<<" "<<currentUser.portNumber<<endl;
    			
				selfLoop.sin_port = htons(currentUser.hbportNumber);

  			 	if (inet_aton(currentUser.ipAddr, &selfLoop.sin_addr) == 0)  {
      		  	fprintf(stderr, "inet_aton() failed\n");
       		 	exit(1);
    			}
				
				cout<<"port sending to "<<(int)ntohs(selfLoop.sin_port)<<endl;
				char *p=inet_ntoa(selfLoop.sin_addr);
				cout<<"ip sending "<<p<<endl;
				Messageobj loopBack;	
				strcpy(loopBack.newmsg.mess,"FAKE");
				socklen_t len=sizeof(selfLoop);
		
			    if (sendto(newUser_s, &loopBack, sizeof(struct Messageobj), 0 , (struct sockaddr *) &selfLoop, len)==-1) {
                    error("sendto() self loop killing");
                 } 
				
		        if (recvfrom(newUser_s, &loopBack, sizeof(struct Messageobj), 0, NULL,NULL) == -1) {
                     cout<<"failed"<<endl;

                }
				//int tru =1;
				//setsockopt(client_s,SOL_SOCKET,SO_REUSEADDR,&tru,sizeof(int));
				
				  /*while(pthread_kill(thread_3, 0) == 0) {
            		pthread_cancel(thread_1);
         		}*/
				
				//close(connected);
				//pthread_t thread_cli_kill;
				// pthread_create( &thread_cli_kill, NULL , clientClose,(void*) 0);
				 //wait(1);
				 close(newUser_s);       		
       			 close(socket_leaderElection);
				 close(client_ack);
				 close(client_s);
       			 existGrpChat(currentUser);
			  return;
				
    		 } else {
				
        		cout<<currentUser.name<<" joining a new chat on "<<currentUser.seqIpAddr<<":"<<currentUser.leaderPortNum<<", listening on "<<currentUser.ipAddr<<":"<<currentUser.portNumber<<endl;
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
    
			for(int i=0; i<number; i++) {

       			 if (recvfrom(newUser_s, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &newUser_si_other, &newUser_slen) == -1) {
          			  error("recvfrom()");
        		}

				usrInGrpClientRecord.push_back(newIncomingMessage.CUser);
		//PrintClientRecord();
		
        
        		string name(newIncomingMessage.CUser.name);
        		string ip(newIncomingMessage.CUser.ipAddr);
        		string notif=name+ " "+ip+":"+ToString(newIncomingMessage.CUser.portNumber);
        		strcpy(newIncomingMessage.newmsg.mess ,notif.c_str());
        
				mPrint.lock();      
        		printMessages(newIncomingMessage,false);
        		mPrint.unlock();
    		}

	
	
	
	
	
	
        
	pthread_create( &thread_1 , NULL , receiver_handler,(void*) 0);
    pthread_create( &thread_2, NULL , sender_handler,(void*) 0);
    pthread_create(&thread_4, NULL, leaderElection_handler, (void*) 0);
	
    pthread_join(thread_1,NULL);
    pthread_join(thread_2,NULL);
	
	cout<<" I reached back end of existgrpuserchaT"<<endl;
   
}

void *SendNewSeqMessageToClient(ChatUser initSeq, int tempUse_usrInGrpClientRecordCtr) {
    int leaderElectionSock; 
            
    if ( (leaderElectionSock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
                error("socket");
             }
            
    for(int i=0;i<tempUse_usrInGrpClientRecordCtr;i++) {
		
        if(usrInGrpClientRecord[i].leaderElectionPort != currentUser.leaderElectionPort) {
        
			//cout<<"Sending to = "<<i<<endl;
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

void *getInput(void *)
{
	
	
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

//Method to initiate sequencer
void newGrpChat(ChatUser initSeq)
{
	
	cout<<"_________________________________________________"<<endl;
    int tempUse_usrInGrpClientRecordCtr=usrInGrpClientRecord.size();
    initialize();
    currentUser=initSeq;
    
    cout<<initSeq.name<<" started a new chat, listening on "<<initSeq.ipAddr<<":"<<initSeq.portNumber<<endl;
    cout<<"Succeeded, current users:"<<endl;
    cout<<initSeq.name<<" "<<initSeq.ipAddr<<"."<<initSeq.portNumber<<" (Leader)"<<endl;

    string port=ToString(initSeq.portNumber);
    string t_name(initSeq.name);
    string t_ip(initSeq.ipAddr);
    
    welcome_mess = t_name + " " + t_ip + ":" + port + "(Leader)";
    cout<<"Waiting for others to join..."<<endl;

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
    tv1.tv_sec       = 2;
    tv1.tv_usec      = 1000;

    if (setsockopt(sockack, SOL_SOCKET, SO_RCVTIMEO,&tv1,sizeof(tv1)) < 0) {
        perror("Error");
    }
    
    if ((sendsock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        error("socket");
    }

    memset((char *) &si_send, 0, sizeof(si_send));

    si_send.sin_family = AF_INET;
    si_send.sin_port = htons(7000);
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

	if(seqFailed==1){
        SendNewSeqMessageToClient(currentUser, tempUse_usrInGrpClientRecordCtr);
  }
	
   pthread_join(thread_1,NULL);
   pthread_join(thread_2,NULL);
    pthread_join(thread_3,NULL);

	  
    
}

int main(int argc, char* argv[])
{
    pthread_create( &getLine , NULL , getInput,(void*) 0);
   
	srand(time(NULL));
    
    if(argc==2) {
		
		/* EC : 7
		string message = "gnome-terminal -e 'sh -c \"./guiInterface\"'";
	    system(message.c_str());
		
		*/
    
        ChatUser initSeq;
        
        initSeq.isSequencer=true;
        strcpy(initSeq.ipAddr,getIP().c_str()); 
        initSeq.portNumber=5000;
        initSeq.hbportNumber=6000;
        initSeq.sendportNumber=7000;
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
       
        ChatUser newUser;
        newUser.isSequencer=false;
        strcpy(newUser.ipAddr,getIP().c_str()); 
       
        newUser.portNumber=1023+rand()%1000;
        newUser.hbportNumber=newUser.portNumber+1;
        
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
        
        newUser.leaderElectionPort=10000+rand()%1000;
        
        string temp_ipAddr="127.0.0.1";
        strcpy(newUser.ipAddr,temp_ipAddr.c_str());
        strcpy(newUser.seqIpAddr,temp_ipAddr.c_str());
        
        
        existGrpChat(newUser);
        
		while(seqShifted==1 ||  seqFailed==1)
		{
			
        if(seqShifted==1) {
            
			
            close(client_s);
            close(socket_leaderElection);
            close(newUser_s);
            close(client_ack);
			
			//close(
				
				
				End_LE=1;
           // pthread_cancel(thread_4);
         //  while(pthread_kill(thread_4, 0) == 0) {
           // pthread_cancel(thread_4);
            //}
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
        
        //pthread_join(getLine,NULL);
        return 0;
    }

    else {
        cout<<"Error:Invalid arguments"<<endl;
        return -1;
    }
}


//Common threads 

void *printMessages(Messageobj newMessage, bool recvFlag)
{
    //mPrint.lock();
     
	//cout<<newMessage.newmsg.name<<":: "<<newMessage.newmsg.mess<<endl;
	
    if(newMessage.newmsg.messageType==DATA) {
        
        if(newMessage.newmsg.seqNum==seqChk) {
            
            cout<<newMessage.newmsg.name<<":: "<<newMessage.newmsg.mess<<endl;
            seqChk=seqChk+1;

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

        else
        {
            holdBack.push_back(newMessage); 
        }

            if(!holdBack.empty())
            {
            int i;
             for(i=0; i < holdBack.size(); i++) 
             {
                if(holdBack[i].newmsg.seqNum==seqChk)
                {
                    mPrint.lock();
                    printMessages(holdBack[i],true);
                    mPrint.unlock();
                }
             }

        }

    }
    if(newMessage.newmsg.messageType==NOTIFICATION) {
        cout<<newMessage.newmsg.noteNum<<" "<<newMessage.newmsg.mess<<endl;
        
        
        istringstream iss(newMessage.newmsg.mess);
        string token;

        int cnt=0;
        while (getline(iss, token, ' ')) {
            if(strcmp(token.c_str(), "joined")==0)
            {
				
                usrInGrpClientRecord.push_back(newMessage.CUser);
                //PrintClientRecord();
                //cout<<"Someone joined hence vector value = "<< usrInGrpClientRecord.size()<<endl;
                break;
            }
			
			if(strcmp(token.c_str(), "left")==0)
            {
                 //cout<<"Someone trying to leave= "<< newMessage.CUser.portNumber<<endl;
				for(int i=0;i<usrInGrpClientRecord.size();i++)
				{
					if(usrInGrpClientRecord[i].portNumber==newMessage.CUser.portNumber)
					{
						usrInGrpClientRecord.erase(usrInGrpClientRecord.begin()+i);
						// cout<<"Someone left hence vector value = "<< usrInGrpClientRecord.size()<<endl;
						break;
					}
				}
				
				
               // usrInGrpClientRecord[usrInGrpClientRecordCtr]=newMessage.CUser;
                //usrInGrpClientRecordCtr++;
                
               
            }
			
        }
		
		
    }
     
//mPrint.unlock();
}

void *SequencerVanished()
{
	//leaderElectionHappening=1;
    int becomeSeqFlag=0;
    
	
	if(usrInGrpClientRecord.size()>1) {
	
	cout<<"number of user in group client record = "<<usrInGrpClientRecord.size()<<endl;
	//cout << "Entering sequencer vanished "<<endl;
		
	
	
    int leaderElectionSock;

    struct sockaddr_in si_leaderElectionSock;
    socklen_t leaderElectionSock_slen=sizeof(si_leaderElectionSock);
            
    if ( (leaderElectionSock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
                error("socket");
             }
            
	
	
    for(int i=0;i<usrInGrpClientRecord.size();i++)
    {
        if(usrInGrpClientRecord[i].leaderElectionPort != currentUser.leaderElectionPort)
        {
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
            
            if (sendto(leaderElectionSock, &NewMessage, sizeof(struct Messageobj), 0 , (struct sockaddr *) &si_leaderElectionSock, leaderElectionSock_slen)==-1) {
            error("sendto() 9");
				
            }
            
           
			if (recvfrom(leaderElectionSock, &NewMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_leaderElectionSock, &leaderElectionSock_slen) == -1) {
             error("recv()");
				
				
            }
			cout<<"Recv in sequener vanished = "<<i<<endl;
            if(NewMessage.newmsg.messageType==LE_Reply)
            {
				
				cout<<"Compare message = "<<NewMessage.newmsg.mess<<endl;
				
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
		cout<<"Directly going to become sequencer "<<endl;
	becomeSeqFlag=1;
		
	}
    if(becomeSeqFlag==1) {
    
	
		
        currentUser.isSequencer=true;
        currentUser.leaderPortNum=currentUser.portNumber;
        seqFailed=1;
        
        //pthread_t abc;
        //killThread_A=1;killThread_B=1;killThread_D=1;
        //pthread_create( &abc , NULL , KillAllThreads,(void*) 0);
		
		
		End_CR=1;
		End_CS=1;
		End_LE=1;
		
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
                error("sendto() self loop killing");
            }
		
	//	wait(3);
		 if (recvfrom(newUser_s, &LE1, sizeof(struct Messageobj), 0, NULL, NULL) == -1) {
               
			perror("Recr from () leader thread");
        }
		
                
    }
	else
	{
		
	}
	
    
}

// Client Related threads


void *receiver_handler(void *)
{
	
	
	
	
   //pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    int pointer=0;
    int i;
    struct Messageobj newIncomingMessage;
    struct sockaddr_in recv_si;
    int isRepeat=0;
    struct Messageobj newmesg;
    Message acksend;
    acksend.messageType = RECVD;
   // struct sockaddr_in seq_sendsock;
    //seq_sendsock.sin_family = AF_INET;
    //int port=newUser.hbportNumber+1000;
    //seq_sendsock.sin_port = htons(7000);
    socklen_t sizerecv = sizeof(recv_si);
    
    /*if (inet_aton(currentUser.seqIpAddr, &seq_sendsock.sin_addr) == 0)  {
            fprintf(stderr, "inet_aton() failed\n");
            exit(1);
            }*/
    
    while(End_CR==0) {
		
		
		//if(initializingNewClient_RH==0)
		//{
			//while(signalNewClient_RH==0){}
			//initializingNewClient_RH=1;
		//}
		
		isRepeat=0;
        if (recvfrom(newUser_s, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &recv_si, &sizerecv) == -1) {
           // error("recvfrom()");
			cout<<"End_CR = "<<End_CR<<endl;
			break;
        }

        string newMessageArrivedChk(newIncomingMessage.newmsg.mess);
        sprintf( acksend.mess, "%ld", newIncomingMessage.newmsg.seqNum);
        
        //acksend.mess = (string)newIncomingMessage.newmsg.seqNum;
        newmesg.newmsg=acksend;
       // cout<<"just received "<<newIncomingMessage.newmsg.mess<<endl;
       
		if(newMessageArrivedChk!="JOIN"){
			// cout<<"not a join "<<endl;
        if (sendto(newUser_s, &newmesg, sizeof(struct Messageobj), 0 , (struct sockaddr *) &recv_si, sizerecv)==-1) {
                error("sendto() 13");
            }
        }
            
        for(i=0;i<100;i++){
            if(seq_arr[i]==newIncomingMessage.newmsg.seqNum){
                isRepeat =1;
        }
        else if(pointer<100){
            seq_arr[pointer] = newIncomingMessage.newmsg.seqNum;
            pointer++;
        }
        else{
            seq_arr[0] = newIncomingMessage.newmsg.seqNum;
            pointer=1;
        }
        }
       
        
        string newMessageArrived(newIncomingMessage.newmsg.mess);
        //string newMessage;
        //Message msgg;
        if(newMessageArrived == "JOIN") {
            Message giveSEQIPData;
            strcpy(giveSEQIPData.mess, "INDIRECT");
            strcpy(giveSEQIPData.ip, currentUser.seqIpAddr);
            giveSEQIPData.port=currentUser.leaderPortNum;
            giveSEQIPData.messageType=INTERNAL;
			
			cout<<"I GOT A JOIN YAYY"<<endl;
            int leaderPortNum;
            string seqIpAddr;
            struct Messageobj newmess;
            newmess.newmsg=giveSEQIPData;

            if (sendto(newUser_s, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &recv_si, sizerecv)==-1) {
                error("sendto() 13");
            }
        }

        else {
            mPrint.lock();
            printMessages(newIncomingMessage,false);
			mPrint.unlock();
            
			/* EC : 7
			if(currentUser.clientType==ANDROID)
			{
				cout<<"sending to android interface"<<endl;
				if (sendto(socket_Android, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_Android, si_Android_slen) == -1) {
          		  error("recvfrom()");
       			 }
					
			}
			*/
			
			
        }
    }
	
	
	
	
	
	
	
	
	
	
}

void *sender_handler(void *)
{
	
	
    //pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    
	
	
    while(End_CS==0) {
		
		
		/*
		
		if(initializingNewClient_SH==0)
		{
			cout<<"Entering init condition Sender"<<endl;
			
			
    
			
			
		    signalNewClient_RH=1;
			initializingNewClient_SH=1;
			
			
		}
		*/
		
		

        char send[1024];
        Message msgg;
      
        if(inputFlag==1){
        
        strcpy(send, mInput.c_str());
        msgg.messageType=DATA;

        if(strcmp(send, "")!=0)
		{
        strcpy(msgg.mess, send);
        strcpy(msgg.name, currentUser.name);

        struct Messageobj newmess;
        newmess.newmsg=msgg;
		newmess.CUser=currentUser;
       
        cout<<"The message = "<<msgg.mess<<endl;
        
        pthread_t clientsend;
		struct sendPar* sendp = (struct sendPar*) malloc(sizeof(struct sendPar));
		sendp->m = newmess;
		sendp->info = newUser_si_other;
		pthread_create( &clientsend , NULL , client_send, sendp);
        }
        inputFlag=0;
    }
        
        if(eofCheck==1) {
            cout<<"leaving "<<endl;
            strcpy(send, "group_leave");
            msgg.messageType=INTERNAL;
            //check=1;
            

        if(strcmp(send, "")!=0){
        strcpy(msgg.mess, send);
        strcpy(msgg.name, currentUser.name);

        struct Messageobj newmess;
        newmess.newmsg=msgg;
		newmess.CUser=currentUser;
			
		cout<<"Im goinna go from = "<<newmess.CUser.portNumber<<endl;	
        //newmess.newmsg.port=(int)ntohs(newUser_si_other.
        
        cout<<"The message = "<<msgg.mess<<endl;
        
        if (sendto(newUser_s, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &newUser_si_other, newUser_slen)==-1) {
            error("sendto() 11");
            }
        }
        eofCheck=0;
        exit(0);
        } 

    }
	
	cout<<"Client Sender Handler is dying"<<endl;
}

void* client_send(void* sendd){
	struct Messageobj newmess;
	struct sockaddr_in si_recv;
	struct sendPar* sendp = (struct sendPar*) sendd;
	
	newmess=sendp->m;
	int t=0;
	
	struct timeval tv;
    tv.tv_sec       = 1;
    tv.tv_usec      = 1000;

	while(t<3){
	if (sendto(client_ack, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &sendp->info, newUser_slen)==-1) {
                error("sendto() 12 ");
            }
	if(recvfrom(client_ack, &newmess, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_recv, &send_slen) == -1){
                error("recvfromto() 12 ");
            }
		cout<<newmess.newmsg.messageType<<endl;
	if((int)ntohs(si_recv.sin_port)==(int)ntohs(sendp->info.sin_port)&& newmess.newmsg.messageType==RECVD){
			break;
			}
		t++;
	}
}

void *client_ack_handler(void *)
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    struct sockaddr_in heartBeatChk;
	socklen_t hrtChkLen=sizeof(heartBeatChk);
	
    int sFailed=0;
	
	struct timeval tv1;
    tv1.tv_sec       = 1;
    tv1.tv_usec      = 1000;

   if (setsockopt(client_s, SOL_SOCKET, SO_RCVTIMEO, &tv1, sizeof(tv1)) < 0) {
        			perror("Error");
    }
	
    cout<<"outside while"<<endl;
    while(End_CAckH==0) {

        string m="";
        char send[1024];
        strcpy(send, "ack");
        Message msgg;
        msgg.messageType=ACK;

        struct Messageobj newmess;
        newmess.newmsg=msgg;
		newmess.CUser=currentUser;
        //cout<<"seq port "<<(int)ntohs(si_ackclient.sin_port)<<endl;
		//cout<<"RANdom"<<endl;
		
		if (recvfrom(client_s, &newmess, sizeof(struct Messageobj), 0, (struct sockaddr *) &heartBeatChk, &hrtChkLen) == -1) {
            cout<<"seq failed"<<endl;
            sFailed=1;
            break;
        }
		//cout<<"RANdom RECV"<<endl;
		
		
		
		//if(newmess.newmsg.mess!="FAKE"){
        if (sendto(client_s, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &heartBeatChk,hrtChkLen)==-1) {
            error("sendto() 6");
        }
	//}
		//cout<<"YO "<<newmess.newmsg.mess<<endl;
    }
    
    if(sFailed==1) SequencerVanished();
	
	cout<<"I went to sequencer vanished and i came back"<<endl;
}

void *leaderElection_handler(void *)
{
	
	
	/* Added Extra
	struct timeval tv;
    tv.tv_sec       = 5;
   	tv.tv_usec      = 1000;
	
	int numberOfMessageGot=0;
	*/
	
    //pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    
    int beginSwitch=0;
    
    struct Messageobj newSeqData;
    
    while(End_LE==0) {

        struct Messageobj newmess;

		
		
        if (recvfrom(socket_leaderElection, &newmess, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_leaderElection, &si_leaderElection_slen) == -1) {
               
			/* Added Extra 
				if(leaderElectionHappening==1){
				cout<<"THe potential leader has vanished i guess"<<endl;
			    potentialLeaderVanished=1;
			    numberOfMessageGot=0;
			}
			
			*/
        }
		
		cout<<"Leader election rcv data = "<<newmess.newmsg.mess<<endl;
		cout<<"Leader election rcv data = "<<newmess.newmsg.messageType<<endl;
		if(strcmp(newmess.newmsg.mess, "Killing LE")==0)
		{
			if (sendto(socket_leaderElection, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &si_leaderElection, si_leaderElection_slen)==-1) {
                error("sendto() self loop killing");
            }
		}
        
		
        if(newmess.newmsg.messageType==LE_PortSend)
        {
			//numberOfMessageGot++;
			
            int clientPortToCompare=atoi(newmess.newmsg.mess);
            
			Messageobj replyMsg;
			replyMsg.newmsg.messageType=LE_Reply;
			
            if(clientPortToCompare>currentUser.portNumber)
            {
                
                
                strcpy(replyMsg.newmsg.mess, "LESSER");
                //replyMsg.newmsg.port=currentUser.portNumber;
                
                
            }
            else
            {
               // Messageobj replyMsg;
                //replyMsg.newmsg.messageType=LE_Reply;
                strcpy(replyMsg.newmsg.mess, "HIGHER");
                //replyMsg.newmsg.port=currentUser.portNumber;
                
               
            }
			if (sendto(socket_leaderElection, &replyMsg, sizeof(struct Messageobj), 0 , (struct sockaddr *) &si_leaderElection, si_leaderElection_slen)==-1) {
                     error("sendto() 8");
                }
			
			/* Added Extra 
			if(numberOfMessageGot==(usrInGrpClientRecordCtr-1)){
			
			 cout<<"Setting timeout"<<endl;
    		if (setsockopt(socket_leaderElection, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
       				 perror("Error");
   			 }		
			
			cout<<"Setting timeout Done"<<endl;
			}
			*/
			
        }
        
        if(newmess.newmsg.messageType==LE_NewSeqMessage)
        {
            newSeqData=newmess;
            seqShifted=1;
            break;
        }
        
    }

    if(seqShifted==1)
    {
    
        currentUser.leaderPortNum=newSeqData.newmsg.port;
        strcpy(currentUser.seqIpAddr, newSeqData.newmsg.ip);
        
        seqShifted=1;
        
        pthread_t abc;
        
       // killThread_A=1;killThread_B=1;killThread_C=1;
        //pthread_create( &abc , NULL , KillAllThreads,(void*) 0);
        End_CR=1;
		End_CS=1;
		End_CAckH=1;
		
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
		
		
        //InitiateReconnect(newSeqData);
    }
  
}


   






//Sequencer Related threads

void *seq_receiver_handler(void *)
{
    
    //pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	struct Messageobj newIncomingMessage;
	struct sockaddr_in recv_si;
	int isRepeat=0;
	struct Messageobj newmesg;
	Message acksend;
	acksend.messageType = RECVD;
	/* struct sockaddr_in seq_sendsock;
    seq_sendsock.sin_family = AF_INET; */
	int pointer=0;
	socklen_t sizerecv = sizeof(recv_si);
	
    while(1) {
        isRepeat =0;
        int recv_len;
        if ((recv_len = recvfrom(sock, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &recv_si, &slen)) == -1) {
            error("recvfrom() 1");
        }
		/* char *ip_addr = inet_ntoa(si_other.sin_addr);
        if (inet_aton(ip_addr, &seq_sendsock.sin_addr) == 0)  {
            fprintf(stderr, "inet_aton() failed\n");
            exit(1);
		}*/
        sprintf( acksend.mess, "%ld", newIncomingMessage.newmsg.seqNum);
		
		/* int port = (int)ntohs(si_other.sin_port);
		seq_sendsock.sin_port = htons(port+1); */
		newmesg.newmsg=acksend;
		
		if(strcmp(newIncomingMessage.newmsg.mess,"JOIN")!=0){
		if (sendto(sock, &newmesg, sizeof(struct Messageobj), 0 , (struct sockaddr *) &recv_si, sizerecv)==-1) {
                error("sendto() 13");
            }
		}	
			//cout<<"Hi"<<endl;
			int i;
			for(i=0;i<100;i++){
			if(seq_arr[i]==newIncomingMessage.newmsg.seqNum){
				isRepeat =1;
		}
		else if(pointer<100){
			seq_arr[pointer] = newIncomingMessage.newmsg.seqNum;
			pointer++;
		}
		else{
			seq_arr[0] = newIncomingMessage.newmsg.seqNum;
			pointer=1;
		}
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
			newmess.CUser=currentUser;
            if (sendto(sock, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &recv_si, newUser_slen)==-1) {
                error("sendto() 1 ");
            }
			
            addToMultiCastDS(newIncomingMessage, recv_si);
			
            mPrint.lock();
			printMessages(newIncomingMessage,false);
		    mPrint.unlock();

        } else if(newMessageArrived == "group_leave") {
            int clientid2;
            int clientid = (int)ntohs(si_other.sin_port);
            for(int i=0; i<usrInGrpSeqRecordCtr; i++) {
                if(usrInGrpSeqRecord[i].portNumber==(int)ntohs(si_other.sin_port)) {
                    clientid2 = usrInGrpSeqRecord[i].hbportNumber;
                    break;
                }
            }

            removefrommulticast(clientid,user_name);
         
        } else {

             if(newIncomingMessage.newmsg.messageType==DATA) {
                
                seq=seq+1;
                newIncomingMessage.newmsg.seqNum=seq;
                newIncomingMessage.newmsg.port=(int)ntohs(si_other.sin_port);
            }
			mPrint.lock();
            printMessages(newIncomingMessage,false);
			mPrint.unlock();
            multicast(newIncomingMessage);
        }
    }
}

void *seq_ack_handler(void *)
{
    //pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    
    while(1) {

        string m="";
        char send[1024];
        strcpy(send, "ack");
        Message msgg;
        msgg.messageType=ACK;

        struct Messageobj newmess;
        newmess.newmsg=msgg;
        string clientname;
		//newmess.CUser=currentUser;
                
       
            for(int i=0; i<usrInGrpSeqRecordCtr; i++) {
				
                 if (sendto(sockack, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &usrInGrpSeqRecord[i].hbSockAddr, si_hb_slen)==-1) {
                    error("sendto() 19");
                }

                struct sockaddr_in si_ack_other;
                if (recvfrom(sockack, &newmess, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_ackclient, &ack_slen) == -1) {
                    int cnt=0;
					
					cout<<"Didnt receive in time"<<endl;
                    for(int j=0; j<usrInGrpSeqRecordCtr; j++) {
                        if(usrInGrpSeqRecord[j].hbportNumber==(int)ntohs(usrInGrpSeqRecord[i].hbSockAddr.sin_port)) {
                            cnt=j;
                            clientname = usrInGrpSeqRecord[j].name;
                        }
                    }
                    int clientid1 = (int)ntohs(usrInGrpSeqRecord[cnt].multicastSockAddr.sin_port);
                    int clientid2 = (int)ntohs(usrInGrpSeqRecord[cnt].hbSockAddr.sin_port);
                    
                    removefrommulticast(clientid1, clientname);
                   
                  
                }
            }
        }
}

void *seq_mess_sender_handler(void *)
{
	
	
    //pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    
	
	while(1) {

        char send[1024];
        
        if(inputFlag==1)
        {
       
        strcpy(send, mInput.c_str());
        struct Messageobj newmess;
        if(strcmp(send, "")!=0)
        {
			
            Message msgg;
            msgg.messageType=DATA;
       
       
            strcpy(msgg.mess, send);
            strcpy(msgg.name, currentUser.name);

        
            
            seq=seq+1;
            
            
            msgg.seqNum=seq;
            newmess.newmsg=msgg;
            
           
            mPrint.lock();
            printMessages(newmess,false);
            mPrint.unlock();
			
            newmess.newmsg=msgg;
			newmess.CUser=currentUser;
			//if( sendto(sock, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &si_other, newUser_slen)==-1) {
              //  error("sendto() 1 ");
            //} 
			multicast(newmess);
			
			inputFlag=0;
        }
      //  for(int i=0; i<usrInGrpSeqRecordCtr; i++) {
           
            //if(strcmp(send, "")!=0){
              //  pthread_t sendseq; (sendto(sock, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &si_other, newUser_slen)==-1) {
                //error("sendto() 1 ");
            //}
              //  struct sendPar* sendp = (struct sendPar*) malloc(sizeof(struct sendPar));
             //   sendp->m = newmess;
             //   sendp->info = usrInGrpSeqRecord[i].multicastSockAddr;
             //   pthread_create( &sendseq , NULL , seq_send, sendp);
            //}
       // }
      }
		
       if(eofCheck==1)
      {
        eofCheck=0;
        exit(0);
      }
    }

}

void *seq_send(void* sendd){
    struct Messageobj newmess;
    struct sockaddr_in si_recv;
    struct sendPar* sendp = (struct sendPar*) sendd;
    newmess=sendp->m;
    int t=0;
    struct timeval tv;
    tv.tv_sec       = 1;
    tv.tv_usec      = 1000;

    while(t<3){
    if (sendto(sendsock, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &sendp->info, newUser_slen)==-1) {
                error("sendto() 12 ");
            }
    if(recvfrom(sendsock, &newmess, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_recv, &send_slen) == -1){
                error("recvfromto() 12 ");
            }
        if((int)ntohs(si_recv.sin_port)==(int)ntohs(sendp->info.sin_port)&& newmess.newmsg.messageType==RECVD){
            break;
        }
        t++;
    }
}


void *addToMultiCastDS(Messageobj newMessage, sockaddr_in si_other)
{
    bool isExisting=false;
    for(int i=0; i<usrInGrpSeqRecordCtr; i++)
        if((int)ntohs(si_other.sin_port)==(int)ntohs(usrInGrpSeqRecord[i].multicastSockAddr.sin_port)) isExisting=true;

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
        notecnt++;
        newNotifMessage.newmsg.noteNum=notecnt;
        
        multicast(newNotifMessage);
		mPrint.lock();
        printMessages(newNotifMessage,false);
          mPrint.unlock();  
        newMessage.CUser.hbSockAddr=create_sock_struct(newMessage);
        newMessage.CUser.multicastSockAddr=si_other;
    }

    usrInGrpSeqRecord[usrInGrpSeqRecordCtr]=newMessage.CUser;
    usrInGrpSeqRecordCtr++;
	
	//PrintSeqRecord();
	
	
    //Sending number of users
    char message[1024];
    strcpy(message,ToString(usrInGrpSeqRecordCtr).c_str());

    if (sendto(sock, &message, sizeof(message), 0 , (struct sockaddr *) &si_other, newUser_slen)==-1) {
        error("sendto() 5");
    }

    //Sending User Data
    struct Messageobj existUserList;
    for(int i=0; i<usrInGrpSeqRecordCtr; i++) {
        
        existUserList.newmsg.messageType=NOTIFICATION;
        existUserList.CUser=usrInGrpSeqRecord[i];
        
        if (sendto(sock, &existUserList, sizeof(struct Messageobj), 0 , (struct sockaddr *) &si_other, newUser_slen)==-1) {
            error("sendto() 4");
        }
    }
}

void multicastSpawn(Messageobj newMessage)
{
		int i;
	    for(i=0; i<usrInGrpSeqRecordCtr; i++) {
        pthread_t sendseq;
		struct sendPar* sendp = (struct sendPar*) malloc(sizeof(struct sendPar));
		sendp->m = newMessage;
		sendp->info = usrInGrpSeqRecord[i].multicastSockAddr;
		pthread_create( &sendseq , NULL , seq_send, sendp);
    }
}

void *multicast(Messageobj newMessage)
{
    Messageobj sendMsg;
	int k;
	bool chkFlag=false;
	priority_msg.push_back(newMessage);
	
	for(k=0;k<priority_msg.size();k++)
	{
		if(priority_msg[k].newmsg.messageType==NOTIFICATION)
		{
			sendMsg=priority_msg[k];
			multicastSpawn(sendMsg);
			priority_msg.erase(priority_msg.begin()+k);
			chkFlag=true;
		}
			
	}
	
	if(!chkFlag)
	{
		sendMsg=priority_msg[0];
		multicastSpawn(sendMsg);
		priority_msg.erase(priority_msg.begin());	
	}
	
}

void *removefrommulticast(int client_remove,string name)
{
	ChatUser temp;
	
    int t=0;
    for(int i=0; i<usrInGrpSeqRecordCtr; i++) {
        if((int)ntohs(usrInGrpSeqRecord[i].multicastSockAddr.sin_port)==client_remove) {
          temp=usrInGrpSeqRecord[i];
			usrInGrpSeqRecord[i]=usrInGrpSeqRecord[i+1];
            t=1;
			
        } else if(t==1) {
            usrInGrpSeqRecord[i]=usrInGrpSeqRecord[i+1];
        }
    }

    usrInGrpSeqRecordCtr--;
    /*t=0;
    for(int i=0; i<usrInGrpSeqRecordCtr; i++) {
        if(usrInGrpSeqRecord[i].portNumber==client_remove) {
            usrInGrpSeqRecord[i]=usrInGrpSeqRecord[i+1];
            t=1;
        } else if(t==1) {
            usrInGrpSeqRecord[i]=usrInGrpSeqRecord[i+1];
        }
    }
    usrInGrpSeqRecordCtr--;*/

    Message newNotice;
    newNotice.messageType=NOTIFICATION;

    string notif_msg="";
    notif_msg="NOTICE "+name+" left the chat or crashed ";
    strcpy(newNotice.mess ,notif_msg.c_str());
    
    struct Messageobj newNotifMessage;
    newNotifMessage.newmsg=newNotice;
    notecnt++;
    newNotifMessage.newmsg.noteNum=notecnt;
    newNotifMessage.CUser=temp;
	mPrint.lock();
    printMessages(newNotifMessage,false);
	mPrint.unlock();
    multicast(newNotifMessage);
	
	   
	
}


/*
void *android_interface_receiver_handler(void *)
{
	
	while(1){
	
	//pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    struct Messageobj newIncomingMessage;
	
	if (recvfrom(socket_Android, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_Android, &si_Android_slen) == -1) {
            error("recvfrom()");
        }
	
	if (sendto(newUser_s, &newIncomingMessage, sizeof(struct Messageobj), 0 , (struct sockaddr *) &newUser_si_other, newUser_slen)==-1) {
            error("sendto() 11");
	    }
	}
}
*/
