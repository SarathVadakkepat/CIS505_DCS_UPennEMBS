
/**
Description: CIS 505 Project 3
Authors: Karthik Anantha Ram, Sanjeet Phatak and Sarath Vadakkepat
**/

#include "ChatServer.h"
#include <time.h>

#define SEQ_REQ 11
#define SEQ_REQ_REPLY 12
#define SEQ_APPROVED 13


//Compulsory needed variables and cross checked 
pthread_t getLine, thread_1,thread_2, multiCast;
string mInput="";
	
ChatUser currentUser;
mutex userInGrpSeq, mPrint,priorityQueue;
vector<ChatUser> usrInGrpClientRecord;

int inputFlag=0;
int eofCheck=0;
int noOfMsgsReceived=0;
unsigned long int seq=0;
unsigned long int seqChk=seq+1;
bool joinExistingChat=false;

mutex msgQueue;
string welcome_mess="";
vector<Messageobj> priority_msg;
vector<Messageobj> holdBack;
vector<unsigned long int> recvdSeqNums;
vector<unsigned long int> seqApprovedNums;
int A=0;
int P=0;

int newJoinedPortNum=0;
//bool 

void *printMessages(Messageobj newMessage, bool recvFlag)
{
	if(newMessage.newmsg.messageType==DATA) {
	cout<<newMessage.newmsg.seqNum<<" - "<<newMessage.newmsg.name<<":: "<<newMessage.newmsg.mess<<endl;
	}
	
    if(newMessage.newmsg.messageType==NOTIFICATION) {
		
        cout<<newMessage.newmsg.mess<<endl;
        
        istringstream iss(newMessage.newmsg.mess);
        string token;

        int cnt=0;
        while (getline(iss, token, ' ')) {
            if(strcmp(token.c_str(), "joined")==0) {
				 if(strcmp(newMessage.CUser.name, currentUser.name)!=0){
                 usrInGrpClientRecord.push_back(newMessage.CUser);
				 break;
				 }
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


void initialize() {
   
	seq=0;
    seqChk=1;
	inputFlag=0;
	eofCheck=0;
	usrInGrpClientRecord.clear();
}

int max()
{
 	if(P>A)
	 return P+1;
	if(A>=P)
		return A+1;
}

void startChat(ChatUser newUser)
{
    currentUser=newUser;
    if ( (newUser_s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        error("socket");
    }
	
	 if ( (newUser_i=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
          error("socket");
    }
	
    
    memset((char *) &si_user, 0, sizeof(si_user));
    si_user.sin_family = AF_INET;
    si_user.sin_port = htons(currentUser.portNumber);
    si_user.sin_addr.s_addr = htonl(INADDR_ANY);
	
	memset((char *) &si_hb, 0, sizeof(si_hb));
    si_hb.sin_family = AF_INET;
    si_hb.sin_port = htons(currentUser.hbportNumber);
    si_hb.sin_addr.s_addr = htonl(INADDR_ANY);

	if( bind(newUser_s , (struct sockaddr*)&si_user, sizeof(si_user) ) == -1) {
        error("bind 1");
    }
	
	if( bind(newUser_i , (struct sockaddr*)&si_hb, sizeof(si_user) ) == -1) {
        error("bind 1");
    }
	
	currentUser.multicastSockAddr=si_user;
	currentUser.hbSockAddr=si_hb;
      
    if(joinExistingChat){
        
    memset((char *) &newUser_si_other, 0, sizeof(newUser_si_other));
    newUser_si_other.sin_family = AF_INET;
    newUser_si_other.sin_port = htons(currentUser.leaderPortNum);
    
    if (inet_aton(currentUser.seqIpAddr, &newUser_si_other.sin_addr) == 0)  {
        fprintf(stderr, "inet_aton() failed 1 \n");
        exit(1);
    }

			Message newMessage;
    		strcpy(newMessage.mess ,"JOIN");
    		strcpy(newMessage.name, currentUser.name);
    		strcpy(newMessage.ip,currentUser.ipAddr);
    		newMessage.port=currentUser.portNumber;
    		newMessage.messageType=INTERNAL;
			
    		struct Messageobj newMessageObj;
    		newMessageObj.newmsg=newMessage;
    		newMessageObj.CUser=currentUser;
	
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
       			exit(-1);
   			 }
			
   			 tv.tv_sec       = 0;
    		 tv.tv_usec      = 0;

    		 if (setsockopt(newUser_s, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
       	 		perror("Error");
  			  }
				
        		cout<<currentUser.name<<" joining a new chat on "<<currentUser.seqIpAddr<<":"<<currentUser.leaderPortNum<<", listening on "<<currentUser.ipAddr<<":"<<currentUser.portNumber<<endl;
        		seqChk=newIncomingMessage.newmsg.seqNum+1;
       		    mPrint.lock();
				printMessages(newIncomingMessage,false);
        		mPrint.unlock();
    		
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
				mPrint.lock();      
        		printMessages(newIncomingMessage,false);
        		mPrint.unlock();
    		}
    }
    
    else
    {
     initialize();
   
    
	usrInGrpClientRecord.push_back(currentUser);
       
	
    int tempUse_usrInGrpClientRecordCtr=usrInGrpClientRecord.size();
  
   
    cout<<currentUser.name<<" started a new chat, listening on "<<currentUser.ipAddr<<":"<<currentUser.portNumber<<endl;
    cout<<"Succeeded, current users:"<<endl;
    cout<<currentUser.name<<" "<<currentUser.ipAddr<<"."<<currentUser.portNumber<<endl;

    string port=ToString(currentUser.portNumber);
    string t_name(currentUser.name);
    string t_ip(currentUser.ipAddr);
    
    cout<<"Waiting for others to join..."<<endl;
	  
    }
        
	pthread_create( &thread_1 , NULL , receiver_handler,(void*) 0);
    pthread_create( &thread_2, NULL , sender_handler,(void*) 0);
    pthread_create( &multiCast, NULL ,multicast,(void*)0);
    
     pthread_join(thread_1,NULL);
     pthread_join(thread_2,NULL);
}

void *addToMultiCastDS(Messageobj newMessage, sockaddr_in si_other)
{
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
            
        newMessage.CUser.multicastSockAddr=si_other;
        printMessages(newNotifMessage, false);
		
		msgQueue.lock();
        priority_msg.push_back(newNotifMessage);
        msgQueue.unlock();
    
        char message[1024];
    	strcpy(message,ToString(usrInGrpClientRecord.size()).c_str());

    		if (sendto(newUser_s, &message, sizeof(message), 0 , (struct sockaddr *) &si_other, newUser_slen)==-1) {
        error("sendto() 5");
   		 }

 
  
   		struct Messageobj existUserList;
    	for(int i=0; i<usrInGrpClientRecord.size(); i++) {
      		existUserList.newmsg.messageType=NOTIFICATION;
        	existUserList.CUser=usrInGrpClientRecord[i];
        if (sendto(newUser_s, &existUserList, sizeof(struct Messageobj), 0 , (struct sockaddr *) &si_other, newUser_slen)==-1) {
            error("sendto() 4");
           }   
    }
    
}

void procSeqRcvPkt(Messageobj newIncomingMessage,sockaddr_in recv_si)
{

	struct Messageobj newMsgForMulti;
    struct Messageobj proposeSeq;
    
    Message proposeSeqNum;
    proposeSeqNum.messageType=SEQ_REQ;
	Message newNotice;
    		
	int i;
	int isRepeat=0;
	struct Messageobj newmesg;
	Message acksend;
	acksend.messageType = RECVD;
	int pointer=0;
	socklen_t sizerecv = sizeof(recv_si);

		
        string newMessageArrived(newIncomingMessage.newmsg.mess);
	    string user_name=newIncomingMessage.newmsg.name;
        string newMessage;
        Message msgg;

		if(newMessageArrived == "JOIN") {  
    
			
            newMessage="Succeeded, current users:";
            strcpy(msgg.mess ,newMessage.c_str());
            msgg.seqNum=seq;
            struct Messageobj newmess;
            newmess.newmsg=msgg;
            newmess.newmsg.messageType=NOTIFICATION;
			newmess.CUser=currentUser;
            if (sendto(newUser_s, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &recv_si, newUser_slen)==-1) {
                error("sendto() 1a");
            }
			
			newJoinedPortNum=newIncomingMessage.CUser.portNumber;
            addToMultiCastDS(newIncomingMessage, recv_si);
			
        } 
	
	    if(newIncomingMessage.newmsg.messageType==DATA) {     
          
			printMessages(newIncomingMessage, false);
			   
            }     
        
         
		if(newIncomingMessage.newmsg.messageType==SEQ_REQ)
        {
            P=max();
           	strcpy(newIncomingMessage.newmsg.mess, ToString(P).c_str());
            newIncomingMessage.newmsg.messageType=SEQ_REQ_REPLY;
           
            if (sendto(newUser_s, &newIncomingMessage, sizeof(struct Messageobj), 0 , (struct sockaddr *) &newIncomingMessage.CUser.hbSockAddr, sizerecv)==-1) {
                error("sendto() 2 ");
            }
			
        }
    
       
	
	 if(newIncomingMessage.newmsg.messageType==NOTIFICATION)
        {
	
		   string name(currentUser.name);
		   string ip(currentUser.ipAddr);
		 
        string notif_msg="";
        notif_msg="NOTICE "+name+" joined on "+ip+":"+ToString(currentUser.portNumber);//+port;
		  
		 if(strcmp(newIncomingMessage.newmsg.mess, notif_msg.c_str())!=0)
				printMessages(newIncomingMessage, false);
	 }
}


void *receiver_handler(void *)
{
    
	struct Messageobj newIncomingMessage;
	struct sockaddr_in recv_si;
	struct Messageobj newmesg;
	Message acksend;
	acksend.messageType = RECVD;
	int recv_len;
	socklen_t sizerecv = sizeof(recv_si);
	
	 while(1) {
		 
		 
        if ((recv_len = recvfrom(newUser_s, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &recv_si, &slen)) == -1) {
            error("recvfrom() 1");
        }
		 
		procSeqRcvPkt(newIncomingMessage,recv_si);
		 
	 }
}

void *sender_handler(void *)
{
	int End_CS=0;
	
	if(strcmp(currentUser.name, "s3")==0||strcmp(currentUser.name, "s6")==0)
	   {
		cout<<"Flooding in 17"<<endl;
		   sleep(10);
		   cout<<"Flooding in 3"<<endl;
		   sleep(3);
		   Message msgg1;
		   msgg1.messageType=DATA;
		   
		   
           strcpy(msgg1.name, currentUser.name);
		   struct Messageobj newmess1;
           
		   newmess1.CUser=currentUser;
       
       // cout<<"The message = "<<msgg.mess<<endl;
			
		for(int i=0;i<7000;i++)
		{
			strcpy(msgg1.mess, "Hi this is flooding  = ");
            strcat(msgg1.mess, currentUser.name);
            strcat(msgg1.mess, "  ");
			strcat(msgg1.mess, ToString(i).c_str());
		   newmess1.newmsg=msgg1;
           
           msgQueue.lock();
           priority_msg.push_back(newmess1);
           //cout<<"Sending "<<i<<endl;

          msgQueue.unlock();
            
             nanosleep((const struct timespec[]){{0,100000000L}},NULL);

         /** pthread_t clientsend1;
		  struct sendPar* sendp1 = (struct sendPar*) malloc(sizeof(struct sendPar));
		  sendp1->m = newmess1;
		  sendp1->info = newUser_si_other;
		  pthread_create( &clientsend1 , NULL , client_send, sendp1);**/
		  // pthread_join(clientsend1,NULL);
		}
		
      //  sleep(5);

       
        End_CS==1;
}
	
	
	while(End_CS==0) {
		
		
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
		newmess.newmsg.messageType=DATA;
        newmess.newmsg=msgg;
		newmess.CUser=currentUser;
			
        noOfMsgsReceived=0;
        
		msgQueue.lock();
        priority_msg.push_back(newmess);
        msgQueue.unlock();
          
        }
        inputFlag=0;

    }
        
        if(eofCheck==1) {     
		
		Message newMessage;
		string name(currentUser.name);
		string notice="NOTICE "+name+" left the chat or crashed";
    	strcpy(newMessage.mess,notice.c_str());
    	strcpy(newMessage.name, currentUser.name);
    	strcpy(newMessage.ip,currentUser.ipAddr);
    	newMessage.port=currentUser.portNumber;
    	newMessage.messageType=NOTIFICATION;
			
    	struct Messageobj newMessageObj;
    	newMessageObj.newmsg=newMessage;
    	newMessageObj.CUser=currentUser;
        
		for(i=0; i<usrInGrpClientRecord.size(); i++) {
        if(usrInGrpClientRecord[i].portNumber!=currentUser.portNumber)
		{
            if (sendto(newUser_s, &newMessageObj, sizeof(struct Messageobj), 0 , (struct sockaddr *) &usrInGrpClientRecord[i].multicastSockAddr, (socklen_t)sizeof(usrInGrpClientRecord[i].multicastSockAddr))==-1) {
                error("sendto() Ctrl+D exit ");
            }
        }
    }
		eofCheck=0;
        exit(0);
        } 

    }
}

void seq_send(Messageobj newmess,sockaddr_in sendTo){
 
    struct sockaddr_in si_recv;
  	socklen_t size=sizeof(sendTo);

    if (sendto(newUser_s, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &sendTo, size)==-1) {
                //error("sendto() 12 ");
            }  
}


void multicastSpawn(Messageobj newMessage)
{	 
		int i;
	    for(i=0; i<usrInGrpClientRecord.size(); i++) {
            
        if(usrInGrpClientRecord[i].portNumber!=currentUser.portNumber){
		 	seq_send(newMessage,usrInGrpClientRecord[i].multicastSockAddr);
		}
    }
}

int greatest()
{
	int maxVal=0;
	for(int ctr=0;ctr<recvdSeqNums.size();ctr++) {
            if(recvdSeqNums[ctr]>maxVal) {
                maxVal=recvdSeqNums[ctr];
            }
        }
	
	recvdSeqNums.clear();
	return maxVal;
}

int maxSeqA()
{
	if(seq>=A)
		return seq;
	if(A>seq)
		return A;
}

void *multicast(void *)
{
    struct Messageobj proposeSeq;
	struct Messageobj clientCrashObj;
	struct sockaddr_in si_recv;
	struct Messageobj newIncomingMessage;
	struct sockaddr_in seqReply;
	Message crashMessage;
	socklen_t slen=sizeof(si_recv);
    int recv_len;

	while(1)
	{
	    int maxVal=0;
        msgQueue.lock();
		
    if(!priority_msg.empty())  {
    
		if(priority_msg[0].newmsg.messageType==DATA){
			
			noOfMsgsReceived=0;
			
    	for(int ctr=0;ctr<usrInGrpClientRecord.size();ctr++) {   
               
        	if(currentUser.portNumber!=usrInGrpClientRecord[ctr].portNumber) {
             
        		proposeSeq.newmsg.messageType=SEQ_REQ;
        		proposeSeq.CUser=currentUser;
            	
				struct timeval tv;
    			tv.tv_sec       = 1;
    			tv.tv_usec      = 1000;
    			if (setsockopt(newUser_i, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
        		perror("Error");
    			}
				
       			 if (sendto(newUser_s, &proposeSeq, sizeof(struct Messageobj), 0 , (struct sockaddr *) &usrInGrpClientRecord[ctr].multicastSockAddr, (socklen_t)sizeof(usrInGrpClientRecord[ctr].multicastSockAddr))==-1) {
                	error("sendto() 3A ");
            	}
				
				
				 if ((recvfrom(newUser_i, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &seqReply,&slen )) == -1) {
					 	string name(usrInGrpClientRecord[ctr].name);
						string notice="NOTICE "+name+" left the chat or crashed";
    					strcpy(crashMessage.mess,notice.c_str());
    					strcpy(crashMessage.name, usrInGrpClientRecord[ctr].name);
    					strcpy(crashMessage.ip,usrInGrpClientRecord[ctr].ipAddr);
    					crashMessage.port=usrInGrpClientRecord[ctr].portNumber;
    					crashMessage.messageType=NOTIFICATION;
						
					 	clientCrashObj.newmsg=crashMessage;
    					clientCrashObj.CUser=usrInGrpClientRecord[ctr];
					 	
					 	//priority_msg.push_back(clientCrashObj);
					 	//multicastSpawn(clientCrashObj);
					 		userInGrpSeq.lock();
					 		usrInGrpClientRecord.erase(usrInGrpClientRecord.begin()+ctr);
					 		userInGrpSeq.unlock();
					 
					 		for(int i=0; i<usrInGrpClientRecord.size(); i++) {
        					if(usrInGrpClientRecord[i].portNumber!=currentUser.portNumber)
							{	
            					if (sendto(newUser_s, &clientCrashObj, sizeof(struct Messageobj), 0 , (struct sockaddr *) &usrInGrpClientRecord[i].multicastSockAddr, (socklen_t)sizeof(usrInGrpClientRecord[i].multicastSockAddr))==-1) {
                				error("sendto() Ctrl+D exit ");
           					 }
        					}
   						 }
					 	printMessages(clientCrashObj,false);
					 
					 	ctr=ctr-1;
					 }
		 			
				tv.tv_sec       = 0;
    			tv.tv_usec      = 0;
    			if (setsockopt(newUser_i, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
        		perror("Error");
    			}
				
				if(newIncomingMessage.newmsg.messageType==SEQ_REQ_REPLY)
        		{
            	recvdSeqNums.push_back(atoi(newIncomingMessage.newmsg.mess));
            	noOfMsgsReceived++;
					newIncomingMessage.newmsg.messageType=-1;
        		}
			}
    	}
        
			//msgQueue.unlock();	
	while(noOfMsgsReceived!=usrInGrpClientRecord.size()-1) {}
    
	if(noOfMsgsReceived==usrInGrpClientRecord.size()-1){  
        
      maxVal=greatest();
      seq=maxVal;
          
	  priority_msg[0].newmsg.seqNum=seq;
      priority_msg[0].newmsg.messageType=DATA;
		
      multicastSpawn(priority_msg[0]);
      printMessages(priority_msg[0], false);
	  A=maxSeqA();
      
      //msgQueue.lock();
      priority_msg.erase(priority_msg.begin());
      //
     }
}
		
	if(priority_msg[0].newmsg.messageType==NOTIFICATION) {
			
	  multicastSpawn(priority_msg[0]);
     // msgQueue.lock();
      priority_msg.erase(priority_msg.begin());
     // msgQueue.unlock();
	
	}
			
			
     }
		msgQueue.unlock();
		sleep(1);
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

int main(int argc, char* argv[])
{
	pthread_create( &getLine , NULL , getInput,(void*) 0);
	srand(time(NULL));
	
	 if(argc==2) {
		 
		 
		 ChatUser initSeq;
        
       
        strcpy(initSeq.ipAddr,getIP().c_str()); 
		
		int portStarting=5000;
		
        initSeq.portNumber=portStarting;
       	initSeq.hbportNumber=portStarting+1;
		
        string temp_name=argv[1];
        strcpy(initSeq.name, temp_name.c_str()); 
        
        strcpy(initSeq.seqIpAddr,initSeq.ipAddr); 
        initSeq.UIRI++;

        string temp_ipAddr="127.0.0.1";
        strcpy(initSeq.ipAddr,temp_ipAddr.c_str());
        strcpy(initSeq.seqIpAddr,temp_ipAddr.c_str());
		joinExistingChat=false;
		startChat(initSeq); 
		 
	 }
	
	else if(argc==3) {
		
		 ChatUser newUser;
      
        strcpy(newUser.ipAddr,getIP().c_str()); 
       
		int portStarting=1024+rand()%1000;
		
        newUser.portNumber=portStarting;
		newUser.hbportNumber=portStarting+1;
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
        
        string temp_ipAddr="127.0.0.1";
        strcpy(newUser.ipAddr,temp_ipAddr.c_str());
        strcpy(newUser.seqIpAddr,temp_ipAddr.c_str());
        
		joinExistingChat=true;
		startChat(newUser);   
	}
	
	 else {
        cout<<"Error:Invalid arguments"<<endl;
        return -1;
    }
	
}

void *removefrommulticast(int client_remove,string name)
{
	/*
	ChatUser temp;
	
    int t=0;
    for(int i=0; i<usrInGrpClientRecord.size(); i++) {
        if((int)ntohs(usrInGrpClientRecord[i].multicastSockAddr.sin_port)==client_remove) {
          temp=usrInGrpClientRecord[i];
		  
		  userInGrpSeq.lock();
		  usrInGrpClientRecord.erase(usrInGrpClientRecord.begin()+i);
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
	
	*/
	
}


