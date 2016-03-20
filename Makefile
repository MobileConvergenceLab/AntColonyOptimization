EXCLUDE = --exclude=.test-ipc --exclude=.test-sendrecv --exclude=.scipts --exclude=Makefile

all:
	cd ./fon && make
	cd ./src && make
	cd ./aco && make

tag:
	ctags -R $(EXCLUDE)

clean:
	cd ./fon && make clean
	cd ./src && make clean
	cd ./aco && make clean

