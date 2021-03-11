Target		= display
CXX         		= gcc
LIBS		= -lwiringPi
ODIR		= obj
SRC			=$(wildcard *.c)
DIR			=$(notdir $(SRC))
OBJ 		= $(patsubst %.c, %.o, $(DIR))
$(Target): $(OBJ)
	$(CXX) $^ -o $@  $(LIBS)
.PHONY:clean
clean:
	rm -rf *.o
	rm -rf $(Target)
