CC=g++
RM=rm -vf
CPPFLAGS=-std=c++11 -Wall -pedantic -Wextra -pthread
SRCFILES=chat_client.cpp
OBJFILES= $(patsubst %.cpp, %.o, $(SRCFILES))
PROGFILES= $(patsubst %.cpp, %, $(SRCFILES))
.PHONY: all clean
all: $(PROGFILES)
clean:
	$(RM) $(OBJFILES) $(PROGFILES)

