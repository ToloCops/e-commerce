#!/usr/bin/make

#IDIR=../hdr
IDIR  = ./include
RDIR  = ../con2redis/src
DDIR  = ../con2db
ODIR  = ./obj
BDIR  = ./bin
SDIR  = ./src

con2redis_OBJ = $(wildcard ../con2redis/obj/*.o)

CC    = g++

CFLAGS   =-std=c++17 -Os -ffast-math -fpermissive -L/usr/local/lib -L../libpqxx-7.9.1/build/src -Wall -I$(IDIR) -I$(RDIR) -I$(DDIR)
TARGET   = ./bin/run
CUSTOMER = ./src/customer/customer
FORNITORE = ./src/fornitore/fornitore
TRASPORTATORE = ./src/trasportatore/trasportatore

all: $(TARGET)


$(TARGET): $(CUSTOMER) $(FORNITORE) $(TRASPORTATORE) $(ODIR)/main.o
	$(CC) $(CFLAGS) ../con2redis/obj/readreply.o ../con2redis/obj/redisfun.o ../con2db/pgsql.o $(ODIR)/main.o -o $(TARGET) -lm -lhiredis -lpqxx -lpq

$(CUSTOMER): $(ODIR)/customer.o $(ODIR)/redis_helper.o
	$(CC) $(CFLAGS) ../con2redis/obj/readreply.o ../con2redis/obj/redisfun.o ../con2db/pgsql.o $(ODIR)/customer.o $(ODIR)/redis_helper.o -o $(CUSTOMER) -lm -lhiredis -lpqxx -lpq


$(FORNITORE): $(ODIR)/fornitore.o
	$(CC) $(CFLAGS) ../con2redis/obj/readreply.o ../con2redis/obj/redisfun.o ../con2db/pgsql.o $(ODIR)/fornitore.o $(ODIR)/redis_helper.o -o $(FORNITORE) -lm -lhiredis -lpqxx -lpq


$(TRASPORTATORE): $(ODIR)/trasportatore.o
	$(CC) $(CFLAGS) ../con2redis/obj/readreply.o ../con2redis/obj/redisfun.o ../con2db/pgsql.o $(ODIR)/trasportatore.o $(ODIR)/redis_helper.o -o $(TRASPORTATORE) -lm -lhiredis -lpqxx -lpq


$(ODIR)/main.o: $(SDIR)/main/main.cpp
	$(CC) -c $(CFLAGS) $< -o $@


$(ODIR)/redis_helper.o: $(SDIR)/redis_helper.cpp
	$(CC) -c $(CFLAGS) $< -o $@


$(ODIR)/fornitore.o: $(SDIR)/fornitore/fornitore.cpp
	$(CC) -c $(CFLAGS) $< -o $@


$(ODIR)/customer.o: $(SDIR)/customer/customer.cpp
	make -C $(RDIR)
	make -C $(DDIR)
	$(CC) -c $(CFLAGS) $< -o $@


$(ODIR)/trasportatore.o: $(SDIR)/trasportatore/trasportatore.cpp
	$(CC) -c $(CFLAGS) $< -o $@