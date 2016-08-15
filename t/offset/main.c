/**
 * client-daemon 간 IPC 테스트를 위해 작성한 코드.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

#include <fon/fon_ipc_msgs.h>
#include <fon/hexdump.h>

// https://www.guyrutenberg.com/2008/12/20/expanding-macros-into-string-constants-in-c/
#define STR(tok) #tok
#define SIZE_PRINT(TYPE)    printf(STR(TYPE) ": %ld\n", sizeof(TYPE));

int main(int argc, char **argv)
{

    SIZE_PRINT(msg_req_hdr_t);
    SIZE_PRINT(msg_req_reg_t);
    SIZE_PRINT(msg_req_dereg_t);
    SIZE_PRINT(msg_req_sendto_t);
    SIZE_PRINT(msg_req_table_add_t);
    SIZE_PRINT(msg_req_table_del_t);
    SIZE_PRINT(msg_req_table_get_t);
    SIZE_PRINT(msg_req_host_get_t);
    SIZE_PRINT(msg_req_buff_t);

    SIZE_PRINT(msg_rsp_hdr_t);
    SIZE_PRINT(msg_rsp_reg_t);
    SIZE_PRINT(msg_rsp_dereg_t);
    SIZE_PRINT(msg_rsp_sendto_t);
    SIZE_PRINT(msg_rsp_table_add_t);
    SIZE_PRINT(msg_rsp_table_del_t);
    SIZE_PRINT(msg_rsp_table_get_t);
    SIZE_PRINT(msg_rsp_host_get_t);
    SIZE_PRINT(msg_rsp_buff_t);

    return 0;
}

