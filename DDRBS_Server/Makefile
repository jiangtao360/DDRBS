CC = g++
LIBS = -lzmq -lleveldb
D_FLAGS = -g -Wall 
R_FLAGS = -O2 -Wall 

TARGET = ddrbs_ser
SRC = network.cpp chunk_db.cpp main.cpp

all : $(TARGET)

$(TARGET) : network.o chunk_db.o main.o
	$(CC) $(LIBS) $(D_FLAGS) -o $(TARGET) network.o chunk_db.o main.o

network.o : network.h network.cpp utility.h
	$(CC) $(D_FLAGS) -c network.cpp

chunk_db.o : chunk_db.h chunk_db.cpp 
	$(CC) $(D_FLAGS) -c chunk_db.cpp 

main.o : main.cpp utility.h network.h chunk_db.h
	$(CC) $(D_FLAGS) -c main.cpp

debug:
	$(CC) $(D_FLAGS) $(LIBS) -o $(TARGET) $(SRC)

release:
	$(CC) $(R_FLAGS) $(LIBS) -o $(TARGET) $(SRC)
	
.PHONY : clean
clean:
	rm -f *.o core* $(TARGET) DDRBS_Server.* 

