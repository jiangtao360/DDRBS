CC = g++
LIBS = -lzmq 
D_FLAGS = -g -Wall 
R_FLAGS = -O2 -Wall 

TARGET = ddrbs_cli
SRC = main.cpp file_filter.cpp file_partition.cpp sha1.cpp network.cpp chunk_dedup.cpp file_backup.cpp

all : $(TARGET)

$(TARGET) : main.o file_filter.o file_partition.o sha1.o network.o chunk_dedup.o file_backup.o  
	$(CC) $(LIBS) $(D_FLAGS) -o $(TARGET) main.o file_filter.o file_partition.o sha1.o network.o chunk_dedup.o file_backup.o

main.o : file_filter.h file_partition.h main.cpp
	$(CC) $(D_FLAGS) -c main.cpp

file_filter.o : file_filter.h file_filter.cpp
	$(CC) $(D_FLAGS) -c file_filter.cpp

sha1.o : sha1.h sha1.cpp
	$(CC) $(D_FLAGS) -c sha1.cpp

file_partition.o : file_partition.h file_partition.cpp utility.h sha1.h
	$(CC) $(D_FLAGS) -c file_partition.cpp

network.o : network.h network.cpp utility.h
	$(CC) $(D_FLAGS) -c network.cpp

chunk_dedup.o : chunk_dedup.h chunk_dedup.cpp utility.h network.h
	$(CC) $(D_FLAGS) -c chunk_dedup.cpp

file_backup.o : file_backup.h file_backup.cpp utility.h network.h
	$(CC) $(D_FLAGS) -c file_backup.cpp


debug:
	$(CC) $(D_FLAGS) $(LIBS) -o $(TARGET) $(SRC)

release:
	$(CC) $(R_FLAGS) $(LIBS) -o $(TARGET) $(SRC)
	
.PHONY : clean
clean:
	rm -f *.o core* $(TARGET) DDRBS_Client.* record
	