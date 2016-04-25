all: dchat.cpp guiInterface.cpp
			g++ -w dchat.cpp -lpthread -o dchat -std=c++11
			g++ -w guiInterface.cpp -lpthread -o guiInterface -std=c++11
			g++ -w dchat_dec.cpp -lpthread -o dchat_dec -std=c++11
clean :
		rm dchat guiInterface dchat_dec
