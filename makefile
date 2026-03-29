all:
	gcc server.c websocket.c database.c logger.c -o server -lpthread -lssl -lcrypto