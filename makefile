COMPILER = gcc -g
FILESYSTEM_FILES_CLIENT = net_raid_client.c parse.c cache.c `pkg-config fuse --cflags --libs`
FILESYSTEM_FILES_SERVER = net_raid_server.c -lssl -lcrypto  `pkg-config fuse --cflags --libs`

build:
	$(COMPILER) $(FILESYSTEM_FILES_CLIENT) -o client # pkg-config fuse --cflags --libs
	$(COMPILER) $(FILESYSTEM_FILES_SERVER) -o server # pkg-config fuse --cflags --libs
	@echo 'run: ./out_client -f [config file]'
	@echo 'run: ./out_server [server] [ip] [serverdir]'

clean:
	rm client
	rm server
