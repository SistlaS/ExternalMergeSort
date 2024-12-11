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

# Generate test cases target
generate_test_cases: Test.exe
	@bash ./generate_test_cases.sh

# TARGET=Test.exe

# # default target
# $(TARGET) : $(OBJS) Makefile
# 	g++ $(CPPFLAGS) -o Test.exe $(OBJS)

# #  TEST SUITE
# runNeg: $(TARGET) ./$(TARGET) -n -2

# runZero: $(TARGET)
# 	./$(TARGET) -n 0

# runOne: $(TARGET)
# 	./$(TARGET) -n 1

# run2: $(TARGET)
# 	./$(TARGET) -n 4

# run3: $(TARGET)
# 	./$(TARGET) -n 19

# run4: $(TARGET)
# 	./$(TARGET) -n 91

# run5: $(TARGET)
# 	./$(TARGET) -n 252

# run6: $(TARGET) ./$(TARGET) -n 921

# run7: $(TARGET)
# 	./$(TARGET) -n 1256

# run8: $(TARGET)
# 	./$(TARGET) -n 8889

# run9: $(TARGET)
# 	./$(TARGET) -n 40000

# run10: $(TARGET)
# 	./$(TARGET) -n 234987

# run11: $(TARGET)
# 	./$(TARGET) -n 500000

# run12: $(TARGET)
# 	./$(TARGET) -n 1000000

