CPPOPT=-g -Og -D_DEBUG
CPPFLAGS=$(CPPOPT) -Wall -ansi -pedantic -std=c++17 -Iinclude -Wno-reorder 

# documents and scripts
DOCS=Tasks.txt README.md
SCRPS=

# headers and code sources
HDRS=	defs.h \
		Iterator.h Scan.h Filter.h Tree.h Sort.h Witness.h 
SRCS=	defs.cpp Assert.cpp Test.cpp \
		Iterator.cpp Scan.cpp Filter.cpp Tree.cpp Sort.cpp Witness.cpp 

# compilation targets
OBJS=	defs.o Assert.o Test.o \
		Iterator.o Scan.o Filter.o Tree.o Sort.o Witness.o 

# RCS assists
REV=-q -f
MSG=no message-Wall -ansi -pedantic -std=c++11 -Iinclude -Wno-reorder


Test.exe : Makefile $(OBJS)
	g++ $(CPPFLAGS) -o Test.exe $(OBJS)

trace : Test.exe Makefile
	@date > trace
	@size -t Test.exe $(OBJS) | sort -r >> trace
	./Test.exe >> trace

$(OBJS) : Makefile defs.h
Test.o : Iterator.h Scan.h Filter.h Tree.h Sort.h Witness.h 
Iterator.o Scan.o Filter.o Sort.o : Iterator.h
Scan.o : Scan.h
Filter.o : Filter.h
Sort.o : Sort.h
Tree.o : Tree.h
Witness.o : Witness.h

list : Makefile
	echo Makefile $(HDRS) $(SRCS) $(DOCS) $(SCRS) > list
count : list
	@wc `cat list`

ci :
	ci $(REV) -m"$(MSG)" $(HDRS) $(SRCS) $(DOCS) $(SCRS)
	ci -l $(REV) -m"$(MSG)" Makefile
co :
	co $(REV) -l $(HDRS) $(SRCS) $(DOCS) $(SCRS)

clean :
	@rm -f $(OBJS) Cache.txt RAM.txt RAM2.txt Disk.txt Disk2.txt input.txt Output.txt Test.exe Test.exe.stackdump trace
