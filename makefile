CFLAGS1 = -lgmp
# CFLAGS2 = -lpthread -lgmp -lmysqlclient
CC = g++
all:server client
server : Server.cpp datamanage.o UserClass.o  paillier.o 
	$(CC) -g Server.cpp datamanage.o UserClass.o  paillier.o -o server $(CFLAGS1) 

# my_semaphore.o : my_semaphore.c
# 	$(CC) -c my_semaphore.c 

# clig: clientpaillier_G.c paillier.o linklist.o my_semaphore.o 
# 	$(CC) -g clientpaillier_G.c paillier.o linklist.o my_semaphore.o -o clig $(CFLAGS2)

client : Client.cpp paillier.o datamanage.o UserClass.o
	$(CC) -g Client.cpp paillier.o datamanage.o UserClass.o -o client $(CFLAGS1) 

datamanage.o: datamanage.cpp 
	$(CC) -g -c datamanage.cpp $(CFLAGS1)

paillier.o:
	$(CC) -c paillier.c $(CFLAGS1)

UserClass.o:
	$(CC) -g -c UserClass.cpp


# query: query.c paillier.o
# 	$(CC) -g query.c paillier.o -o query -lgmp

# timecnt: timecnt.c paillier.o
# 	$(CC) -g timecnt.c paillier.o -o timecnt -lgmp

# timecnt_naive: timecnt_naive.c paillier.o
# 	$(CC) -g timecnt_naive.c paillier.o -o timecnt_naive -lgmp

clean:
	rm *.o server client