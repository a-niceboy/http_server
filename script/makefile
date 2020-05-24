THIS_PATH=$(shell pwd)

SHARED_LIB = $(THIS_PATH)/../shared_lib

LD_LIBRARY_PATH=$(SHARED_LIB)

OBJ = TimeManager.o KEncode.o HttpHeader.o Http.o SqlInterface.o  DataCenter.o Server.o main.o

EXE = nr_report_srv

INCLUDES = -I$(THIS_PATH)/../include \
	-I$(THIS_PATH)/../include/mysql

STATIC_LIB = $(THIS_PATH)/../static_lib/libmysqlclient.a \
	$(THIS_PATH)/../static_lib/libz.a \
	$(THIS_PATH)/../static_lib/libssl.a \
	$(THIS_PATH)/../static_lib/libcrypto.a \
	$(THIS_PATH)/../static_lib/libjsoncpp.a

all : $(OBJ)
	g++ -std=c++14 -lrt -luuid -ldl -lpthread -Wall -g -o $(EXE) $(OBJ) $(INCLUDES) $(STATIC_LIB)

TimeManager.o : TimeManager.h TimeManager.cpp
	g++ -std=c++14 -c -Wall -g -o TimeManager.o TimeManager.cpp $(INCLUDES)

KEncode.o : KEncode.h KEncode.cpp
	g++ -std=c++14 -c -Wall -g -o KEncode.o KEncode.cpp $(INCLUDES)

HttpHeader.o : HttpHeader.h HttpHeader.cpp
	g++ -std=c++14 -c -Wall -g -o HttpHeader.o HttpHeader.cpp $(INCLUDES)

Http.o : Http.h Http.cpp
	g++ -std=c++14 -c -Wall -g -o Http.o Http.cpp $(INCLUDES)

SqlInterface.o : SqlInterface.h SqlInterface.cpp
	g++ -std=c++14 -c -Wall -g -o SqlInterface.o SqlInterface.cpp $(INCLUDES)

DataCenter.o : DataCenter.h DataCenter.cpp
	g++ -std=c++14 -c -Wall -g -o DataCenter.o DataCenter.cpp $(INCLUDES)

Server.o : Server.h Server.cpp
	g++ -std=c++14 -c -Wall -g -o Server.o Server.cpp $(INCLUDES)

main.o : typedefine.h Server.h main.cpp
	g++ -std=c++14 -c -Wall -g -o main.o main.cpp $(INCLUDES)

	
clean :
	rm -f $(EXE) *.o *.gch core* *.out
