EXCLUDE = --exclude=.test-ipc --exclude=.test-sendrecv --exclude=.scipts --exclude=Makefile

all:
	cd ./fon && make
	cd ./fond && make
	cd ./aco && make

tag:
	ctags -R $(EXCLUDE)

clean:
	cd ./aco && make clean
	cd ./fond && make clean
	cd ./fon && make clean

