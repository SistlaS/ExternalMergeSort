# CPPOPT=-g -Og -D_DEBUG
# # -O2 -Os -Ofast
# # -fprofile-generate -fprofile-use
# CPPFLAGS=$(CPPOPT) -Wall -ansi -pedantic
# # -Wparentheses -Wno-unused-parameter -Wformat-security
# # -fno-rtti -std=c++11 -std=c++98

# # documents and scripts
# DOCS=Tasks.txt
# SCRS=

# # headers and code sources
# HDRS=	defs.h \
# 		Iterator.h Scan.h Filter.h Sort.h Witness.h
# SRCS=	defs.cpp Assert.cpp Test.cpp \
# 		Iterator.cpp Scan.cpp Filter.cpp Sort.cpp Witness.cpp

# # compilation targets
# OBJS=	defs.o Assert.o Test.o \
# 		Iterator.o Scan.o Filter.o Sort.o Witness.o

# # RCS assists
# REV=-q -f
# MSG=no message

# # default target
# #
# Test.exe : Makefile $(OBJS)
# 	g++ $(CPPFLAGS) -o Test.exe $(OBJS)

# trace : Test.exe Makefile
# 	@date > trace
# 	@size -t Test.exe $(OBJS) | sort -r >> trace
# 	./Test.exe >> trace

# $(OBJS) : Makefile defs.h
# Test.o : Iterator.h Scan.h Filter.h Sort.h Witness.h
# Iterator.o Scan.o Filter.o Sort.o : Iterator.h
# Scan.o : Scan.h
# Filter.o : Filter.h
# Sort.o : Sort.h
# Witness.o : Witness.h

# list : Makefile
# 	echo Makefile $(HDRS) $(SRCS) $(DOCS) $(SCRS) > list
# count : list
# 	@wc `cat list`

# ci :
# 	ci $(REV) -m"$(MSG)" $(HDRS) $(SRCS) $(DOCS) $(SCRS)
# 	ci -l $(REV) -m"$(MSG)" Makefile
# co :
# 	co $(REV) -l $(HDRS) $(SRCS) $(DOCS) $(SCRS)

# clean :
# 	@rm -f $(OBJS) Test.exe Test.exe.stackdump trace
# compiler options
CPPOPT=-g -Og -D_DEBUG

# compiler flags
CPPFLAGS=$(CPPOPT) -Wall -ansi -pedantic -std=c++11 -Iinclude -Wno-reorder

# headers and code sources
HDRS=$(wildcard *.h)
SRCS=$(wildcard *.cpp)
TARGET=ExternalMergeSort.exe

# compilation targets
OBJS=$(SRCS:%.cpp=%.o)

# pattern rules for object files
%.o: %.cpp $(HDRS)
	g++ $(CPPFLAGS) -c $< -o $@

# default target
$(TARGET) : $(OBJS) Makefile
	g++ $(CPPFLAGS) -o ExternalMergeSort.exe $(OBJS)

trace : ExternalMergeSort.exe Makefile
	@date > trace
	./ExternalMergeSort.exe >> trace
	@size -t ExternalMergeSort.exe $(OBJS) | sort -r >> trace

# run rule
run: $(TARGET)
	./$(TARGET) -c 2500000 -s 1000

list : Makefile
	echo Makefile $(HDRS) $(SRCS) $(DOCS) $(SCRS) > list
count : list
	@wc `cat list`

clean :
	@rm -f $(OBJS) ExternalMergeSort.exe ExternalMergeSort.exe.stackdump trace