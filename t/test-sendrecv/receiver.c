/**
 * client-daemon 간 IPC 테스트를 위해 작성한 코드.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <fon/fon.h>
#include <fon/fon_ipc.h>
#include "common.h"

int main(int argc, char **argv)
{
    FonClient       *client     = NULL;
    packet_buff_t   buff        = {{0}};
    int             port        = -1;

    if(argc != 2)
    {
        printf("Usage: %s PORT\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]);

    client = fon_client_new(FON_TEST_TYPE, port);
    if(client == NULL)
    {
        perror("fon_client_new()");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        fon_recvfrom(client,
                     &buff.hdr,
                     sizeof(packet_buff_t));

        printf("%s\n", buff.hdr.data);
    }

    return 0;
}

