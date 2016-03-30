
enum class MsgType { JOIN, QUIT, SEQ_ALIVE , USER_ALIVE, ELECT, LEADER, ACK, STATUS };

struct sendInfo
{
	int portNumber;
	string ipAddr;
	
};


class Message
{
private:
	   MsgType msgType;
	   sendInfo srcInfo;
	   sendInfo dstInfo;
	   string msg;
	   string sendTimeStamp;
	   string recvTimeStamp;
public:

};