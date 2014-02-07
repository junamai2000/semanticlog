CC=g++
LIB=-lapr-1
INC=-I/usr/include/apr-1.0/
OBJS=SemanticLogFileRoute.o SemanticLogger.o main.o
OPT=-fPIC

# 生成規則部
a.out: $(OBJS)
	$(CC) $(LIB) -o $@ $(OBJS)

.cc.o:
	$(CC) $(OPT) $(INC) -c $<

clean:
	rm *.o
	rm a.out

