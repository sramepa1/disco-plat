all: ./client/Client ./server/Server

./client/Client:
	cd client && make

./server/Server:
	cd server && make

clean:
	cd client && make clean
	cd server && make clean

run: ./client/Client ./server/Server
	./start.sh
