/**
Description: CIS 505 Project 3
Authors: Karthik Anantha Ram, Sanjeet Phatak and Sarath Vadakkepat
**/

#include "ChatServer.h"

//Compulsory needed variables and cross checked 

ChatUser currentUser;

pthread_t thread_1,thread_2, thread_3, thread_4 , getLine;
int killThread_A=0,killThread_B=0,killThread_C=0,killThread_D=0;

//Class for users in a group chat
struct ChatUser usrInGrpClientRecord[CHATUSERS_COUNT];
int usrInGrpClientRecordCtr=0;

struct ChatUser usrInGrpSeqRecord[CHATUSERS_COUNT];
int usrInGrpSeqRecordCtr=0;

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
//End

string welcome_mess;

void *KillAllThreads(void *)
{
	if(killThread_A!=0)	{
    while(pthread_kill(thread_1, 0) == 0) {
            pthread_cancel(thread_1);
         }
	}
	if(killThread_B!=0) {
		
		while(pthread_kill(thread_2, 0) == 0) {
            pthread_cancel(thread_2);
        }
	}
	if(killThread_C!=0)	{
        while(pthread_kill(thread_3, 0) == 0) {
            pthread_cancel(thread_3);
            
        }
	}  
	
	if(killThread_D!=0)	{
        while(pthread_kill(thread_4, 0) == 0) {
            pthread_cancel(thread_4);
            
        }
	}  
	killThread_A=0;killThread_B=0;killThread_D=0;killThread_C=0;
    pthread_exit(NULL);
}

void initialize() {
	
	usrInGrpSeqRecordCtr=0;
    usrInGrpClientRecordCtr=0;
    seq=0;
    seqChk=1;
}

//Method to enable a user join a existing chat
void existGrpChat(ChatUser newUser)
{
    initialize();
    currentUser=newUser;
   
    if ( (newUser_s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        error("socket");
    }

    memset((char *) &newUser_si_other, 0, sizeof(newUser_si_other));
    newUser_si_other.sin_family = AF_INET;
    newUser_si_other.sin_port = htons(newUser.leaderPortNum);
	
	if ( (client_ack=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        error("socket");
    }

   
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
        error("bind 1");
    }

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
        error("bind 2");
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
        error("sendto() 2");
    }
    
    struct Messageobj newIncomingMessage;
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
        error("bind 3");
    }
    
    //Struct or LE complete.
        
    if(strcmp(newIncomingMessage.newmsg.mess,"INDIRECT")==0) {
        string leadIP(newIncomingMessage.newmsg.ip);
        
        strcpy(newUser.seqIpAddr, leadIP.c_str());
        newUser.leaderPortNum= newIncomingMessage.newmsg.port;
        close(newUser_s);
        close(client_s);
        close(socket_leaderElection);
        existGrpChat(newUser);
        return;
    } else {
        cout<<newUser.name<<" joining a new chat on "<<newUser.seqIpAddr<<":"<<newUser.leaderPortNum<<", listening on "<<newUser.ipAddr<<":"<<newUser.portNumber<<endl;
        seqChk=newIncomingMessage.newmsg.seqNum+1;
        printMessages(newIncomingMessage,false);
       
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

		usrInGrpClientRecord[i]=newIncomingMessage.CUser;
        usrInGrpClientRecordCtr++;
		
		string name(newIncomingMessage.CUser.name);
        string ip(newIncomingMessage.CUser.ipAddr);
        string notif=name+ " "+ip+":"+ToString(newIncomingMessage.CUser.portNumber);
        strcpy(newIncomingMessage.newmsg.mess ,notif.c_str());
		       
        printMessages(newIncomingMessage,false);
        
    }

    pthread_create( &thread_1 , NULL , receiver_handler,(void*) 0);
    pthread_create( &thread_2, NULL , sender_handler,(void*) 0);
    pthread_create( &thread_3, NULL , client_ack_handler,(void*) 0);
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

void *getInput(void *)
{
  while(1){
      if(inputFlag==0 && eofCheck==0){
      getline(cin,mInput);  
      if(cin.eof()==1){
          eofCheck=1;
          break;
      }
      else{
        inputFlag=1;
      }
    }
  }
}

//Method to initiate sequencer
void newGrpChat(ChatUser initSeq)
{
    int tempUse_usrInGrpClientRecordCtr=usrInGrpClientRecordCtr;
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

        string temp_ipAddr="192.168.0.101";
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
        
        string temp_ipAddr="192.168.0.101";
        strcpy(newUser.ipAddr,temp_ipAddr.c_str());
        strcpy(newUser.seqIpAddr,temp_ipAddr.c_str());
        
        
        existGrpChat(newUser);
        
        if(seqShifted==1) {
            
            close(client_s);
            close(socket_leaderElection);
            close(newUser_s);
            
            pthread_cancel(thread_4);
            while(pthread_kill(thread_4, 0) == 0) {
            pthread_cancel(thread_4);
            }
                     
            existGrpChat(currentUser);            
        }
        
        if(seqFailed==1) {
            
            close(client_s);
            close(socket_leaderElection);
            close(newUser_s);

            pthread_cancel(thread_3);
            
            while(pthread_kill(thread_3, 0) == 0) {
            pthread_cancel(thread_3);
        }
            
            memset((char *) &newUser_si_other, 0, sizeof(newUser_si_other));
            memset((char *) &si_user, 0, sizeof(si_user));
            memset((char *) &si_ackclient, 0, sizeof(si_ackclient));
            memset((char *) &si_hb, 0, sizeof(si_hb));
            
            newGrpChat(currentUser);
            
        }
        
        pthread_join(getLine,NULL);
        return 0;
    }

    else {
        cout<<"Error:Invalid arguments"<<endl;
        return -1;
    }
}

void *printMessages(Messageobj newMessage, bool recvFlag)
{
	mPrint.lock();
	 
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
                    
                    printMessages(holdBack[i],true);
                    
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
                
                usrInGrpClientRecord[usrInGrpClientRecordCtr]=newMessage.CUser;
                usrInGrpClientRecordCtr++;
                
                break;
            }
        }
    }
	 
mPrint.unlock();
}


void *receiver_handler(void *)
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    int pointer=0;
	int i;
    struct Messageobj newIncomingMessage;
	struct sockaddr_in recv_si;
	int isRepeat=0;
	struct Messageobj newmesg;
	Message acksend;
	acksend.messageType = RECVD;
	struct sockaddr_in seq_sendsock;
    seq_sendsock.sin_family = AF_INET;
	//int port=newUser.hbportNumber+1000;
    seq_sendsock.sin_port = htons(7000);
	socklen_t sizerecv = sizeof(recv_si);
    
	if (inet_aton(currentUser.seqIpAddr, &seq_sendsock.sin_addr) == 0)  {
            fprintf(stderr, "inet_aton() failed\n");
            exit(1);
            }
	
	while(1) {
        isRepeat=0;
        if (recvfrom(newUser_s, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &recv_si, &sizerecv) == -1) {
            error("recvfrom()");
        }
		 sprintf( acksend.mess, "%ld", newIncomingMessage.newmsg.seqNum);
   
		//acksend.mess = (string)newIncomingMessage.newmsg.seqNum;
		newmesg.newmsg=acksend;
		
		if (sendto(newUser_s, &newmesg, sizeof(struct Messageobj), 0 , (struct sockaddr *) &recv_si, sizerecv)==-1) {
                error("sendto() 13");
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
        string newMessage;
        Message msgg;
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

            if (sendto(newUser_s, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &newUser_si_other, newUser_slen)==-1) {
                error("sendto() 13");
            }
        }

        else {
            
            printMessages(newIncomingMessage,false);
            
        }
    }
}

void *seq_mess_sender_handler(void *)
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    while(1) {

        char send[1024];
        
        if(inputFlag==1)
        {
            inputFlag=0;
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
            
           // cout<<"The message = "<<msgg.mess<<endl;
            
            
            printMessages(newmess,false);
            
            newmess.newmsg=msgg;
        }
        for(int i=0; i<usrInGrpSeqRecordCtr; i++) {
            
            if(strcmp(send, "")!=0){
				pthread_t sendseq;
				struct sendPar* sendp = (struct sendPar*) malloc(sizeof(struct sendPar));
				sendp->m = newmess;
				sendp->info = usrInGrpSeqRecord[i].multicastSockAddr;
				pthread_create( &sendseq , NULL , seq_send, sendp);
            }
        }
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

void *sender_handler(void *)
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    
    while(1) {

        char send[1024];
        Message msgg;
      
        if(inputFlag==1){
        
        strcpy(send, mInput.c_str());
        msgg.messageType=DATA;

        if(strcmp(send, "")!=0){
        strcpy(msgg.mess, send);
        strcpy(msgg.name, currentUser.name);

        struct Messageobj newmess;
        newmess.newmsg=msgg;
        //newmess.newmsg.port=(int)ntohs(newUser_si_other.
        
            cout<<"The message = "<<msgg.mess<<endl;
        
        if (sendto(newUser_s, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &newUser_si_other, newUser_slen)==-1) {
            error("sendto() 11");
            }
        }
        inputFlag=0;
    }
        
        if(eofCheck==1) {
           // cout<<"leaving "<<endl;
            strcpy(send, "group_leave");
            msgg.messageType=INTERNAL;
            //check=1;
            

        if(strcmp(send, "")!=0){
        strcpy(msgg.mess, send);
        strcpy(msgg.name, currentUser.name);

        struct Messageobj newmess;
        newmess.newmsg=msgg;
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
}

void *seq_ack_handler(void *)
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    
    while(1) {

        string m="";
        char send[1024];
        strcpy(send, "ack");
        Message msgg;
        msgg.messageType=ACK;

        struct Messageobj newmess;
        newmess.newmsg=msgg;
        string clientname;
       
            for(int i=0; i<usrInGrpSeqRecordCtr; i++) {

                 if (sendto(sockack, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &usrInGrpSeqRecord[i].hbSockAddr, si_hb_slen)==-1) {
                    error("sendto() 19");
                }

                struct sockaddr_in si_ack_other;
                if (recvfrom(sockack, &newmess, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_ackclient, &ack_slen) == -1) {
                    int cnt=0;

                    for(int j=0; j<usrInGrpSeqRecordCtr; j++) {
                        if(usrInGrpSeqRecord[j].hbportNumber==(int)ntohs(usrInGrpSeqRecord[i].hbSockAddr.sin_port)) {
                            cnt=j;
                            clientname = usrInGrpSeqRecord[j].name;
                        }
                    }
                    int clientid1 = (int)ntohs(usrInGrpSeqRecord[cnt].multicastSockAddr.sin_port);
                    int clientid2 = (int)ntohs(usrInGrpSeqRecord[cnt].hbSockAddr.sin_port);
                    
                    removefrommulticast(clientid1, clientname);
                    removeackclient(clientid2);
                  
    			}
            }
	    }
}

void *SequencerVanished()
{
    int becomeSeqFlag=0;
    
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
            
            if(NewMessage.newmsg.messageType==LE_Reply)
            {
                if(strcmp(NewMessage.newmsg.mess, "LESSER")==0) leaderElectionCount++;
                
            if(leaderElectionCount==(usrInGrpClientRecordCtr-1)) {
                
                becomeSeqFlag=1;
                break;
                                
            }
            }
            
        }
    }
    
    if(becomeSeqFlag==1) {
    
        currentUser.isSequencer=true;
        currentUser.leaderPortNum=currentUser.portNumber;
        seqFailed=1;
        
        pthread_t abc;
		killThread_A=1;killThread_B=1;killThread_D=1;
        pthread_create( &abc , NULL , KillAllThreads,(void*) 0);
                
    }
    
}

void *leaderElection_handler(void *)
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    
    int beginSwitch=0;
    
    struct Messageobj newSeqData;
    
    while(1) {

        struct Messageobj newmess;
        
        if (recvfrom(socket_leaderElection, &newmess, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_leaderElection, &si_leaderElection_slen) == -1) {
                error("recvfrom()");
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
                     error("sendto() 8");
                }
            }
            else
            {
                Messageobj replyMsg;
                replyMsg.newmsg.messageType=LE_Reply;
                strcpy(replyMsg.newmsg.mess, "HIGHER");
                replyMsg.newmsg.port=currentUser.portNumber;
                
                if (sendto(socket_leaderElection, &replyMsg, sizeof(struct Messageobj), 0 , (struct sockaddr *) &si_leaderElection, si_leaderElection_slen)==-1) {
                     error("sendto() 7");
                }
            }
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
		
		killThread_A=1;killThread_B=1;killThread_C=1;
        pthread_create( &abc , NULL , KillAllThreads,(void*) 0);
        
        //InitiateReconnect(newSeqData);
    }
  
}

void *client_ack_handler(void *)
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    
    int sFailed=0;
    
    while(1) {

        string m="";
        char send[1024];
        strcpy(send, "ack");
        Message msgg;
        msgg.messageType=ACK;

        struct Messageobj newmess;
        newmess.newmsg=msgg;
        
        if (sendto(client_s, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &si_ackclient, sizeof(si_ackclient))==-1) {
            error("sendto() 6");
        }
        if (recvfrom(client_s, &newmess, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_ackclient, &ack_slen) == -1) {
            cout<<"seq failed"<<endl;
            sFailed=1;
            break;
        }
    }
    
    if(sFailed==1) SequencerVanished();
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
        printMessages(newNotifMessage,false);
       		
    	newMessage.CUser.hbSockAddr=create_sock_struct(newMessage);
		newMessage.CUser.multicastSockAddr=si_other;
    }

    usrInGrpSeqRecord[usrInGrpSeqRecordCtr]=newMessage.CUser;
    usrInGrpSeqRecordCtr++;
    
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

void *multicast(Messageobj newMessage)
{
    for(int i=0; i<usrInGrpSeqRecordCtr; i++) {

        pthread_t sendseq;
		struct sendPar* sendp = (struct sendPar*) malloc(sizeof(struct sendPar));
		sendp->m = newMessage;
		sendp->info = usrInGrpSeqRecord[i].multicastSockAddr;
		pthread_create( &sendseq , NULL , seq_send, sendp);
    }
}

void *removefrommulticast(int client_remove,string name)
{
    int t=0;
    for(int i=0; i<usrInGrpSeqRecordCtr; i++) {
        if((int)ntohs(usrInGrpSeqRecord[i].multicastSockAddr.sin_port)==client_remove) {
           usrInGrpSeqRecord[i]=usrInGrpSeqRecord[i+1];
            t=1;
        } else if(t==1) {
            usrInGrpSeqRecord[i]=usrInGrpSeqRecord[i+1];
        }
    }

    usrInGrpSeqRecordCtr--;
    t=0;
    for(int i=0; i<usrInGrpSeqRecordCtr; i++) {
        if(usrInGrpSeqRecord[i].portNumber==client_remove) {
            usrInGrpSeqRecord[i]=usrInGrpSeqRecord[i+1];
            t=1;
        } else if(t==1) {
            usrInGrpSeqRecord[i]=usrInGrpSeqRecord[i+1];
        }
    }
    usrInGrpSeqRecordCtr--;

    Message newNotice;
    newNotice.messageType=NOTIFICATION;

    string notif_msg="";
    notif_msg="NOTICE "+name+" left the chat or crashed ";
    strcpy(newNotice.mess ,notif_msg.c_str());
	
    struct Messageobj newNotifMessage;
    newNotifMessage.newmsg=newNotice;
	notecnt++;
	newNotifMessage.newmsg.noteNum=notecnt;
    
    printMessages(newNotifMessage,false);
    multicast(newNotifMessage);
}

void *removeackclient(int client_remove)
{
    int t=0;
    for(int i=0; i<usrInGrpSeqRecordCtr+1; i++) {
        if((int)ntohs(usrInGrpSeqRecord[i].hbSockAddr.sin_port)==client_remove) {
            usrInGrpSeqRecord[i]=usrInGrpSeqRecord[i+1];
            t=1;
        } else if(t==1) {
            usrInGrpSeqRecord[i]=usrInGrpSeqRecord[i+1];
        }
    }
}

void *seq_receiver_handler(void *)
{
    
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    
    struct Messageobj newIncomingMessage;
    while(1) {
        int recv_len;
        if ((recv_len = recvfrom(sock, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &si_other, &slen)) == -1) {
            error("recvfrom() 1");
        }
        
        //Sequencer Welcoming new client
        string newMessageArrived(newIncomingMessage.newmsg.mess);
        string user_name=newIncomingMessage.newmsg.name;
        string newMessage;
        Message msgg;
        if(newMessageArrived == "JOIN") {
            
            cout<<"Received a JOIN message "<<endl;
            newMessage="Succeeded, current users:\n";
            newMessage += welcome_mess;
            strcpy(msgg.mess ,newMessage.c_str());
            msgg.seqNum=seq;
            struct Messageobj newmess;
            newmess.newmsg=msgg;
            newmess.newmsg.messageType=NOTIFICATION;
            if (sendto(sock, &newmess, sizeof(struct Messageobj), 0 , (struct sockaddr *) &si_other, newUser_slen)==-1) {
                error("sendto() 1 ");
            }

            addToMultiCastDS(newIncomingMessage, si_other);
            
            printMessages(newIncomingMessage,false);
            

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
            removeackclient(clientid2);
        } else {

             if(newIncomingMessage.newmsg.messageType==DATA) {
                
                seq=seq+1;
                newIncomingMessage.newmsg.seqNum=seq;
                newIncomingMessage.newmsg.port=(int)ntohs(si_other.sin_port);
            }
            printMessages(newIncomingMessage,false);
            multicast(newIncomingMessage);
        }
    }
}
