CCFLAGS=-Wall -g -std=gnu99
LDFLAGS=-Wall -g -std=gnu99
OBJ=poll.o
EXE=poll

.PHONY: all clean install

all : $(EXE)

$(EXE) : $(OBJ)
	@echo building $@
	$(CC) -o $(EXE) $(LDFLAGS) $(OBJ)
	@echo done

%.o : %.c
	@echo building $@ ...
	$(CC) $(CCFLAGS) -c $<
	@echo done

clean:
	@echo cleaning repository...
	@rm -f ./poll
	@rm -f *.o
	@rm -f *~
	@echo cleaned.

install :
	-@rm /usr/local/bin/poll
	cp ./poll /usr/local/bin/poll
