#include <sys/socket.h> // socket()
#include <arpa/inet.h> // htonl()
#include <stdlib.h> // atoi()
#include <stdio.h> // perror()
#include <unistd.h> // close()

#include "aco_ipc.h"

int main(int argc, char **argv)
{
	struct sockaddr_in	addr = {};
	socklen_t		len = sizeof(struct sockaddr_in);
	int			sock = -1;
	int			ipc_port = -1;
	int			ret = -1;
	struct find_reqeust 	fr;
	char 			buff[1024];
	size_t 			buflen = sizeof(buff);
	int			sendbyte = 0;

	if(argc ==1)
	{
		printf("Wrong argument\n");
		ret = -1;
		goto RETURN;
	}

	ipc_port = atoi(argv[1]);
	fprintf(stderr, "port: %d\n", ipc_port);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port=htons(ipc_port);

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1) {
		perror("socket()");
		ret = -1;
		goto RETURN;
	}

	if(connect(sock, (struct sockaddr*)&addr, len) == -1) {
		perror("connext()");
		ret = -1;
		goto RETURN;
	}
	printf("Connected\n");

	fr.hdr.type = message_type_find;
	fr.hdr.paylen = sizeof(aco_id_packed_t) + sizeof(aco_cycle_packed_t);
	fr.target = ACO_ID_PACK(atoi(argv[2]));
	fr.ncycle = ACO_CYCLE_PACK(10);

	request_serial(buff, &buflen, &fr.hdr);

	sendbyte = send(sock, buff, buflen, 0);

	if(sendbyte != 0)
	{
		ret = 0;
	}
	else
	{
		ret = -1;
	}

RETURN:
	close(sock);
	return ret;
}
