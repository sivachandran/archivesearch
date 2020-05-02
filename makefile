INCL=-I$(QTDIR)/include -I/usr/include/kde
CFLAGS=-O2 -fno-strength-reduce -DQT_THREAD_SUPPORT
LFLAGS=-L$(QTDIR)/lib -L/usr/lib/kde3 -L/usr/X11R6/lib
LIBS=-lkdecore -lkdeui -lkio -lqt -lX11 -lXext -ldl
CC=g++

ArchiveSearch: main.o ArchiveSearch.o moc_ArchiveSearch.o
	$(CC) $(LFLAGS) -o ArchiveSearch main.o ArchiveSearch.o moc_ArchiveSearch.o $(LIBS)

main.o: main.cpp
ArchiveSearch.o: ArchiveSearch.cpp ArchiveSearch.h
moc_ArchiveSearch.cpp: ArchiveSearch.h
	$(QTDIR)/bin/moc ArchiveSearch.h -o moc_ArchiveSearch.cpp

install:
	cp ArchiveSearch /usr/bin
	
clean:
	rm -f main
	rm -f ArchiveSearch
	rm -f main.o
	rm -f ArchiveSearch.o
	rm -f moc_*

.SUFFIXES:.cpp

.cpp.o:
	$(CC) -c $(CFLAGS) $(INCL) -o $@ $<
  
