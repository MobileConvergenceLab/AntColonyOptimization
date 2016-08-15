#ifndef MSG_HNDLER_H
#define MSG_HNDLER_H

#include <fon/fon_ipc_msgs.h>

class Client;

class ClientMsgHndler
{
public:
	static void req_handlers(Client *obj, const msg_req_hdr_t *req);
private:
	static void req_handler_reg(Client *obj, const msg_req_hdr_t *req);
	static void req_handler_dereg(Client *obj, const msg_req_hdr_t *req);
	static void req_handler_sendto(Client *obj, const msg_req_hdr_t *req);
	static void req_handler_table_add(Client *obj, const msg_req_hdr_t *req);
	static void req_handler_table_del(Client *obj, const msg_req_hdr_t *req);
	static void req_handler_table_get(Client *obj, const msg_req_hdr_t *req);
	static void req_handler_host_get(Client *obj, const msg_req_hdr_t *req);
};


#endif // MSG_HNDLER_H
