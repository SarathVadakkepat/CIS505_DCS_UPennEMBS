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
#include<math.h>
#include <cstdlib>
#include <time.h>
#include <ctime>
#include <fstream>
#include <iostream>







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
#define ANDROID 9
#define TRAFFIC_CNTRL 10

void error(const char *msg)
{
    perror(msg);
    //exit(-1);
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
	//unsigned long int noteNum;
	int tryNum;
	unsigned long int pktNum;
};

class ChatUser
{
public:
    
    char name[32];
    int UIRI;
    int portNumber;
    int hbportNumber;
	int sendportNumber;
	
	int leaderPortNum;
	int leaderSendPortNumber;
    int leaderHBPortNumber;
	
	char ipAddr[32];
    
    bool isSequencer=false;
   
    char seqIpAddr[32];
   
    int leaderElectionPort;
	
	
	
	sockaddr_in multicastSockAddr;
	sockaddr_in hbSockAddr;
	
	int clientType=0;
	
	int currentMsgCount=0;
	int prevMsgCount=0;
	bool trafficFlag=false;
};


struct Messageobj {
    Message newmsg;
    ChatUser CUser;
};

struct sendPar{
	sockaddr_in info;
	Messageobj m;
};

struct multiCastPkt{
	Messageobj m;
};

vector<string> split(string str, char delimiter) {
  vector<string> internal;
  stringstream ss(str); // Turn the string into a stream.
  string tok;
  
  while(getline(ss, tok, delimiter)) {
    internal.push_back(tok);
  }
  
  return internal;
}

//Encrypted
long int p,q,i,n,t,j,m[512],d[512], temp[512], e[512],en[512],flag;
int prime(long int);
void ce();
long int cd(long int);


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
struct sockaddr_in si_clientsend;

socklen_t send_slen=sizeof(si_clientsend);
//socklen_t send_slen=sizeof(si_send);

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
void *multicast(void *);
void multicastSpawn(Messageobj newMessage);
void *removefrommulticast(int client_remove, string name);
void *seq_indirect_joining_handler(void *);
void *client_ack_handler(void *);
void *seq_ack_handler(void *);
void *leaderElection_handler(void *);
void *SendNewSeqMessageToClient(ChatUser initSeq);
void *InitiateReconnect(Messageobj newSeq);
void seq_send(Messageobj newmess,sockaddr_in sendTo);
void client_send(Messageobj newmsg,sockaddr_in);
string ToString(int val);
void *android_interface_receiver_handler(void *);
string decrypt(string mainMesg);
string encrypt(string msg);
//void *android_interface_receiver_handler(void *);
void *procSeqRcvPkt(void * sendp);
void *TrafficCheck(void *);
void *monitorCliMessages(void *);



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

string decrypt(string mainMesg)
{
	/*
	vector<string> MainSplit = split(mainMesg, '$');
	
	long int en[512], temp[512];
	vector<string> en1 = split(MainSplit[0], ',');
	vector<string> temp1 = split(MainSplit[1], ',');
	
	int n=atoi(MainSplit[2].c_str());
	int key=atoi(MainSplit[3].c_str());
	
	for(int ctr=0;ctr<512;ctr++) { en[ctr]=0; temp[ctr]=0;}
	
	for(int ctr=0;ctr<en1.size();ctr++) en[ctr]=atoi(en1[ctr].c_str());
	for(int ctr=0;ctr<temp1.size();ctr++) temp[ctr]=atoi(temp1[ctr].c_str());
	
	long int pt, ct, k, m[512];
    long int i = 0, j;
	
    while (en[i] != -1)
    {
        ct = temp[i];
        k = 1;
        for (j = 0; j < key; j++)
        {
            k = k * ct;
            k = k % n;
        }
        pt = k + 96;
        m[i] = pt;
        i++;
    }
    m[i] = -1;
	
	string decryptedMessageChar;
	for (i = 0; m[i] != -1; i++)
	{
		decryptedMessageChar=decryptedMessageChar+(char)m[i];
	}
	return decryptedMessageChar;;
	*/
	
	
	vector<string> MainSplit = split(mainMesg, '~');
	
	long int en[512], temp[512];
	//vector<string> en1 = split(MainSplit[0], ',');
	//vector<string> temp1 = split(MainSplit[1], ',');
	
	
	int n=atoi(MainSplit[2].c_str());
	int key=atoi(MainSplit[3].c_str());
	
	for(int ctr=0;ctr<512;ctr++) { en[ctr]=0; temp[ctr]=0;}
	
	char msg[2048];
	strcpy(msg,MainSplit[0].c_str());
	int stringLength=strlen(msg);
	
	
	for (int i = 0; i<stringLength; i++){
     	en[i] = msg[i];
		}
	
	bzero(msg, 2048);
	strcpy(msg,MainSplit[1].c_str());
	stringLength=strlen(msg);
	for (int i = 0; i<stringLength; i++){
     	temp[i] = msg[i];
		}
	
	
	//for(int ctr=0;ctr<en1.size();ctr++) en[ctr]=(int);
	//for(int ctr=0;ctr<temp1.size();ctr++) temp[ctr]=atoi(temp1[ctr].c_str());
	
		
		
	
	/*
	vector<string> MainSplit = split(mainMesg, '$');
	
	long int en[512], temp[512];
	vector<string> en1 = split(MainSplit[0], ',');
	vector<string> temp1 = split(MainSplit[1], ',');
	
	int n=atoi(MainSplit[2].c_str());
	int key=atoi(MainSplit[3].c_str());
	
	for(int ctr=0;ctr<512;ctr++) { en[ctr]=0; temp[ctr]=0;}
	
	for(int ctr=0;ctr<en1.size();ctr++) en[ctr]=atoi(en1[ctr].c_str());
	for(int ctr=0;ctr<temp1.size();ctr++) temp[ctr]=atoi(temp1[ctr].c_str());
	
	*/
	
	
	long int pt, ct, k, m[512];
    long int i = 0, j;
	
    while (en[i] != -1)
    {
        ct = temp[i];
        k = 1;
        for (j = 0; j < key; j++)
        {
            k = k * ct;
            k = k % n;
        }
        pt = k + 96;
        m[i] = pt;
        i++;
    }
    m[i] = -1;
	
	
    	
	
	string decryptedMessageChar;
	for (i = 0; m[i] != -1; i++)
	{
		decryptedMessageChar=decryptedMessageChar+(char)m[i];
	
	}
	
	return decryptedMessageChar;
	
	
	
}





class RandomPrimeGenerator
{
      public:
      RandomPrimeGenerator(){ srand(time(0));}
      ~RandomPrimeGenerator(){}

      void init_fast(unsigned int _max_number);
    

      unsigned int get();
      
      private:
      unsigned int max_number;
      unsigned int max_prime_count;
      vector<unsigned int> prime_list;
      void find_primes();
};


void RandomPrimeGenerator::find_primes()
{
     prime_list.clear();
     prime_list.push_back(2);
     
     int i,j;
     unsigned int size=prime_list.size();
     bool is_prime;
     unsigned int cur_prime;
     for (i=3; max_number==0||i<=max_number; i+=2)
     {
         is_prime=true;
         for (j=0; j<size; j++)
         {
             cur_prime=prime_list[j];
             if (i<cur_prime*cur_prime) break;
             
             if (i%cur_prime==0) {is_prime=false; break;}
         }
         if (is_prime)
         {
            prime_list.push_back(i);
            size++;
            if (max_prime_count!=0&&size==max_prime_count) break;
         }
     }
}

void RandomPrimeGenerator::init_fast(unsigned int _max_number)
{
     max_number=_max_number;
     max_prime_count=0;
    
     find_primes();
}

unsigned int RandomPrimeGenerator::get()
{
       unsigned int size=prime_list.size();
        unsigned int index=int(0.5+(size-1)*(rand()/double(RAND_MAX)));
        return prime_list[index];
       
}


int prime(long int pr)
{
    int i;
    j = sqrt(pr);
    for (i = 2; i <= j; i++)
    {
        if (pr % i == 0)
            return 0;
    }
    return 1;
}


void ce()
{
    int k;
    k = 0;
    for (i = 2; i < t; i++)
    {
        if (t % i == 0)
            continue;
        flag = prime(i);
        if (flag == 1 && i != p && i != q)
        {
            e[k] = i;
            flag = cd(e[k]);
            if (flag > 0)
            {
                d[k] = flag;
                k++;
            }
            if (k == 99)
                break;
        }
    }
}
long int cd(long int x)
{
    long int k = 1;
    while (1)
    {
        k = k + t;
        if (k % x == 0)
            return (k / x);
    }
}

string encrypt(string Imsg)
{
	 p=0,q=0,i=0,n=0,t=0,j=0,m[512],d[512], temp[512], e[512],en[512],flag=0;
	
	for(int ctr=0;ctr<512;ctr++)
	{
		m[ctr]=0;
		d[ctr]=0;
			temp[ctr]=0;
			e[ctr]=0;
			en[ctr]=0;
	}
	
	
	RandomPrimeGenerator rpg;
       
	rpg.init_fast(100);
	int prime1 = rpg.get();
	int prime2=rpg.get();
	//cout<<prime1<<endl;
	//cout<<prime2<<endl;
    
	p=7;
	q=17;
	//p=prime1;
	//q=prime2;
  
	
	
	char msg[256];
	strcpy(msg, Imsg.c_str());
	int stringLength=strlen(msg);

	for (i = 0; i<stringLength; i++){
     	m[i] = msg[i];
		}
	
    n = p * q;
    t = (p - 1) * (q - 1);
    ce();
	
	
    long int pt, ct, key = e[0], k, len;
    i = 0;
    len = strlen(msg);
    while (i != len)
    {
        pt = m[i];
        pt = pt - 96;
        k = 1;
        for (j = 0; j < key; j++)
        {
            k = k * pt;
            k = k % n;
        }
        temp[i] = k;
        ct = k + 96;
        en[i] = ct;
        i++;
    }
    en[i] = -1;
    
	char encryptedMessageChar[1024];
	bzero(encryptedMessageChar,1024);
	
	char encryptedMessageNum[1024];
	bzero(encryptedMessageNum,1024);
	
	char encryptedMessageTempChar[1024];
	char encryptedMessageTempNum[1024];
	bzero(encryptedMessageTempNum,1024);
	
	string encChar, temChar;
	
	for(i=0;i<512;i++){
	
		int msg=en[i];
			
		if(msg!=-1){	

		strcat(encryptedMessageNum, ToString(msg).c_str());
		strcat(encryptedMessageNum, ",");
			
		encChar=encChar+(char)msg;
		}
		
		if(msg==-1)
		{
			strcat(encryptedMessageNum, ToString(msg).c_str());
		    strcat(encryptedMessageNum, ",");
			encChar=encChar+(char)msg;
			break;
		}
}
	
	for(i=0;i<512;i++) {
	
		int tem=temp[i];
		if(tem!=0){
		
		strcat(encryptedMessageTempNum, ToString(tem).c_str());
		strcat(encryptedMessageTempNum, ",");
			temChar=temChar+(char)tem;
		}
		
	}
	
	string EncryptedMessage(encryptedMessageNum);
	string EncryptedMessageTemp(encryptedMessageTempNum);
	
	
	//EncryptedMessage=EncryptedMessage+"$"+EncryptedMessageTemp+"$"+ToString(n)+"$"+ToString(d[0])+"$";
	EncryptedMessage=encChar+"~"+temChar+"~"+ToString(n)+"~"+ToString(d[0])+"~";
	
	//cout<<EncryptedMessage<<endl;
	
	
	
	
	
	return EncryptedMessage;
	
	
}




