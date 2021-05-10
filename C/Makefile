Target		= display
CXX         	= gcc
ODIR		= obj
SRC		=$(wildcard *.c)
DIR		=$(notdir $(SRC))
OBJ 		= $(patsubst %.c, %.o, $(DIR))
$(Target): $(OBJ)
	$(CXX) $^ -o $@ -g -rdynamic 
.PHONY:clean
clean:
	rm -rf *.o
	rm -rf $(Target)
