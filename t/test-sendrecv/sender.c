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
#include <fon/fon_test.h>
#include "common.h"

int main(int argc, char **argv)
{
    FonClient       *client     = NULL;
    packet_buff_t   buff        = {{0}};
    char            msg[]       = "Hello World!\0";
    int             port        = -1;
    int             snder_id    = -1;
    int             rcver_id    = -1;

    if(argc != 4)
    {
        printf("Usage: %s PORT SENDER_ID RECIVER_ID\n", argv[0]);
        return -1;
    }

    port        = atoi(argv[1]);
    snder_id    = inet_addr(argv[2]);
    rcver_id    = inet_addr(argv[3]);

    client = fon_client_new(FON_TEST_TYPE, port);
    if(client == NULL)
    {
        perror("fon_client_new()");
        exit(EXIT_FAILURE);
    }

    pkt_hdr_set(&buff.hdr,
                snder_id,
                rcver_id,
                FON_TEST_TYPE);

    pkt_payload_set(&buff.hdr,
                    sizeof(packet_buff_t),
                    msg,
                    strlen(msg)+1);

    fon_test_table_print(client);

    while(1)
    {
        fon_sendto(client, &buff.hdr);
        sleep(1);
    }

    return 0;
}

