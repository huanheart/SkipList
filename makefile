
CXXFLAGS = -w

objects = main.o

edit :$(objects)
	g++ -g -o skip $(objects)
main.o : skiplist.h

.PHONY : clean
clean: 
	rm -f $(objects)
