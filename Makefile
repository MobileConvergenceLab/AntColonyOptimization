EXCLUDE = --exclude=.test-ipc --exclude=.test-sendrecv --exclude=.scipts --exclude=Makefile

all:
	cd ./fon && make
	cd ./fond && make
	cd ./t && make
	cd ./aco && make

tag:
	ctags -R $(EXCLUDE)

clean:
	cd ./aco && make clean
	cd ./t && make clean
	cd ./tools && make clean
	cd ./fond && make clean
	cd ./fon && make clean

test:
	cd ./fon && make re-build
	cd ./fond && make re-build
	cd ./t && make re-build

