#include "ChatServer.h"

void *android_interface_receiver_handler(void *);
void *android_interface_sender_handler(void *);

int init=0;

struct sockaddr_in interface_clientAddress;
int interface_newSocket;
struct sockaddr_in interface_newUser_si_other;
int interface_newUser_s;
socklen_t interface_newUser_slen=sizeof(interface_newUser_si_other);
	
socklen_t interface_addrlen;
	int length,bytes,x;
	char finalData[1024];
	int bytes_recieved;
	char receive_data[1024];   

string nam="";
pthread_t androidRecv, androidSend;

int main(int argc, char* argv[]){
  	srand(time(NULL));
	 
	int interface_mySocketfd;
	struct sockaddr_in interface_myAddress;
	
	interface_mySocketfd = socket(AF_INET,SOCK_STREAM,0);
	
	if (interface_mySocketfd < 0){
		perror("ERROR IN THE SOCKET CONNECTION");
		
	}
	
	bzero((char*) &interface_myAddress,sizeof(interface_myAddress));
	
	int port=1024+rand()%1000;
	cout<<"Port = "<<port<<endl;
	interface_myAddress.sin_family = AF_INET;
	interface_myAddress.sin_port = htons(port);
	interface_myAddress.sin_addr.s_addr = INADDR_ANY;

	if (bind(interface_mySocketfd, (struct sockaddr *)&interface_myAddress, sizeof(interface_myAddress))<0){
		perror("ERROR IN BIND");
		exit(1);
		}

	if (listen(interface_mySocketfd, 5) == -1) {                      //listening for connections
        perror("Listen");  
        exit(1);
    }
	
	interface_addrlen = sizeof(interface_clientAddress);
	
	int read_size;
	while(1) {
		interface_newSocket = accept(interface_mySocketfd,(struct sockaddr *)&interface_clientAddress,&interface_addrlen);
		while(1) {
			
			bytes_recieved = recv(interface_newSocket,receive_data,1024,0);                     
          	receive_data[bytes_recieved] = '\0';
                                   
            if(init==0){
			vector<string> sep = split(receive_data, '+');
		   
			string name(sep[0]);
			nam=name;
			string ip(sep[1]);
			string message = "gnome-terminal -e 'sh -c \"./dchat "+name+" "+ip+"\"'";
			system(message.c_str());
			
			sleep(2);
				
			if ( (interface_newUser_s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
       				 error("socket");
   			}

   			memset((char *) &interface_newUser_si_other, 0, sizeof(interface_newUser_si_other));
    		interface_newUser_si_other.sin_family = AF_INET;
    		interface_newUser_si_other.sin_port = htons(80000);	
				
			sep = split( ip, ':');
			
			if (inet_aton(sep[0].c_str(), &interface_newUser_si_other.sin_addr) == 0)  {
       			 fprintf(stderr, "inet_aton() failed\n");
        		 exit(1);
  			}
			struct Messageobj newIncomingMessage;
			strcpy(newIncomingMessage.newmsg.mess,"ANDROID");
			
			if (sendto(interface_newUser_s, &newIncomingMessage, sizeof(struct Messageobj), 0 , (struct sockaddr *) &interface_newUser_si_other, interface_newUser_slen)==-1) {
       			 error("sendto() 2");
    		}
				
			init=1;
				
			pthread_create( &androidRecv , NULL , android_interface_receiver_handler,(void*) 0);
			pthread_create( &androidSend , NULL , android_interface_sender_handler,(void*) 0);
				
			}
			
			pthread_join(androidRecv,NULL);
   			pthread_join(androidSend,NULL);
		}
	}
	close(interface_newSocket);
	close(interface_mySocketfd);
	return 0; 
}


void *android_interface_receiver_handler(void *)
{
	while(1){
	bytes_recieved = recv(interface_newSocket,receive_data,1024,0);                     
    receive_data[bytes_recieved] = '\0';
           
    struct Messageobj newIncomingMessage;
	
	strcpy(newIncomingMessage.newmsg.name, nam.c_str());
	strcpy(newIncomingMessage.newmsg.mess,   receive_data);
	newIncomingMessage.newmsg.messageType=2;	
	
	if (sendto(interface_newUser_s, &newIncomingMessage, sizeof(struct Messageobj), 0 , (struct sockaddr *) &interface_newUser_si_other, interface_newUser_slen)==-1) {
       			 error("sendto() 2");
    }
}
}

void *android_interface_sender_handler(void *)
{
	while(1){
	
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    struct Messageobj newIncomingMessage;
	
	 if (recvfrom(interface_newUser_s, &newIncomingMessage, sizeof(struct Messageobj), 0, (struct sockaddr *) &interface_newUser_si_other, &interface_newUser_slen) == -1) {
            error("recvfrom()");
        }
	
	char recv[1024];
	if(newIncomingMessage.newmsg.messageType==1)
			strcpy(recv, newIncomingMessage.newmsg.mess);
		
	if(newIncomingMessage.newmsg.messageType==2){
			strcpy(recv, newIncomingMessage.newmsg.name);
			strcat(recv, ":: ");
			strcat(recv, newIncomingMessage.newmsg.mess);
	}
	strcat(recv,"\n");
	send(interface_newSocket, &recv, strlen(recv) , 0);
	bzero(recv,1024);
	
	}
	
}