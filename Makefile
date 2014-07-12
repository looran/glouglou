LIBS = libsendbuf libgsnetstream libggnet libglouglou
EXE = glougloud
DIRS = $(LIBS) $(EXE)

all:
	-for d in $(DIRS); do (cd $$d; $(MAKE)); done

clean:
	-for d in $(DIRS); do (cd $$d; $(MAKE) clean); done

install:
	-for d in $(DIRS); do (cd $$d; $(MAKE) install); done

test:
	make -j5 clean && make -j5 && sudo make -j5 install && sudo ./glougloud/glougloud -Dvv
	#make -j5 clean && make -j5 && sudo make -j5 install && sudo gdb ./glougloud/glougloud
