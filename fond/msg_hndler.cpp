#include <string.h>
#include <fon/fon_dbg.h>
#include <fon/fon_ipc.h>
#include <fon/hexdump.h>
#include <fon/fon_fib_if.h>
#include "dbg.hpp"
#include "ipcmanager.hpp"
#include "msg_hndler.hpp"


void ClientMsgHndler::req_handlers(Client *client, const msg_req_hdr_t *req)
{
	switch(req->msg_type)
	{
	case MSG_TYPE_REG:
		req_handler_reg(client, req);
		break;

	case MSG_TYPE_DEREG:
		req_handler_dereg(client, req);
		break;

	case MSG_TYPE_SENDTO:
		req_handler_sendto(client, req);
		break;

	case MSG_TYPE_TABLE_ADD:
		req_handler_table_add(client, req);
		break;

	case MSG_TYPE_TABLE_DEL:
		req_handler_table_del(client, req);
		break;

	case MSG_TYPE_TABLE_GET:
		req_handler_table_get(client, req);
		break;

	case MSG_TYPE_HOST_GET:
		req_handler_host_get(client, req);
		break;

	default:
		DBG_PRINT_MSG("ERROR");
		exit(EXIT_FAILURE);
	}
}

void ClientMsgHndler::req_handler_reg(Client *client, const msg_req_hdr_t *req)
{
	msg_req_reg_t	   *reg = (msg_req_reg_t*)req;

	// update client's func_type
	client->m_type  = req->func_type;

	// update client's async_addr
	struct sockaddr_in  *async_addr = &client->m_asyncaddr;
	async_addr->sin_family = AF_INET;
	async_addr->sin_port=htons(reg->async_port);
	if(inet_pton(AF_INET,"127.0.0.1",&(async_addr->sin_addr)) != 1)
	{
		perror("inet_pton()");
		exit(EXIT_FAILURE);
	}

	// add(register) to clientmanager
	client->reg();

	msg_rsp_buff_t buff;
	msg_rsp_fill	(&buff.hdr,
			 sizeof(msg_rsp_buff_t),
			 MSG_RESULT_SUCCESS,
			 "Success",
			 NULL,
			 0);
	msg_rsp_send(client->m_syncsock, &buff.hdr);

	return;
}

void ClientMsgHndler::req_handler_dereg(Client *client, const msg_req_hdr_t *req)
{
	delete client;
} // req_handler_dereg()

void ClientMsgHndler::req_handler_sendto(Client *client, const msg_req_hdr_t *req)
{
	static char err_str_fail[]		= "Fail: There is no did in Forward Table";
	static char err_str_succ[]		= "Succes";

	msg_req_sendto_t	*sendto		= (msg_req_sendto_t*)req;
	packet_buff_t		pkt		= {0,};
	int			neigh_id	= -1;
	fib_tuple_t		tuple		= {0,};
	Neighbor		*neighbor	= NULL;
	msg_result_t		result		= MSG_RESULT_FAILURE;
	char			*err_str	= NULL;
	FibPtr			fib		= client->m_fib;
	IdTablePtr		idtable		= fib->get_idtable();
	msg_rsp_buff_t		buff		= {0,};

	pkt_cpy(&pkt.hdr, sizeof(packet_buff_t), &sendto->pkt);

	/* 어디로 포워딩 해야하는지 알아낸다. */
	if(!fib->get(pkt.hdr.did, &tuple))
	{
		// client에 대한 에러 메세지를 설정
		result  = MSG_RESULT_FAILURE;
		err_str = err_str_fail;
		goto RETURN;
	}
	neigh_id = tuple.neighbor;
	neighbor = idtable->get_neighbor(neigh_id);
	assert(neighbor != nullptr);

	/* 실제로 패킷을 보내는 가상함수를 호출한다. */
	neighbor->op_sendto(&pkt.hdr);

	// client에 대한 에러 메세지를 설정
	result  = MSG_RESULT_SUCCESS;
	err_str = err_str_succ;

RETURN:
	msg_rsp_fill	(&buff.hdr,
			sizeof(msg_rsp_buff_t),
			result,
			err_str,
			NULL,
			0);
	msg_rsp_send(client->m_syncsock, &buff.hdr);

	return;
} // req_handler_sendto()

void ClientMsgHndler::req_handler_table_add(Client *client, const msg_req_hdr_t *req)
{
	//dbg("Called");
	msg_result_t		result		= MSG_RESULT_FAILURE;
	char			*err_str	= NULL;
	msg_req_table_add_t	*req_table_add	= (msg_req_table_add_t*)req;
	FibPtr			fib		= client->m_fib;

	fib->update(req_table_add->tuple);
	result = MSG_RESULT_SUCCESS;
	err_str = strdup("Success");

	msg_rsp_buff_t buff;
	msg_rsp_fill	(&buff.hdr,
			sizeof(msg_rsp_buff_t),
			result,
			err_str,
			NULL,
			0);
	msg_rsp_send(client->m_syncsock, &buff.hdr);

	free(err_str);

	//dbg("Done");
	return;
} // req_handler_table_add()

void ClientMsgHndler::req_handler_table_del(Client *client, const msg_req_hdr_t *req)
{
	//dbg("Called");
	msg_req_table_del_t 	*req_table_del	= (msg_req_table_del_t*)req;
	FibPtr			fib		= client->m_fib;

	/* always success */
	fib->del(req_table_del->id);

	msg_rsp_buff_t buff;
	msg_rsp_fill(&buff.hdr,
				 sizeof(msg_rsp_buff_t),
				 MSG_RESULT_SUCCESS,
				 "Success",
				 NULL,
				 0);
	msg_rsp_send(client->m_syncsock, &buff.hdr);

	//dbg("Done");
	return;
} // req_handler_table_del()

void ClientMsgHndler::req_handler_table_get(Client *client, const msg_req_hdr_t *req)
{
	//dbg("Called");
	FibPtr			fib	= client->m_fib;
	// vector of fib tuple
	auto			tv	= fib->get_all();
	msg_rsp_buff_t		buff	= {0,};

	if(tv.size() != 0)
	{
		msg_rsp_fill	(&buff.hdr,
				sizeof(msg_rsp_buff_t),
				MSG_RESULT_SUCCESS,
				"Success",
				&tv[0],
				sizeof(fib_tuple_t)*tv.size());
	}
	else
	{
		msg_rsp_fill	(&buff.hdr,
				sizeof(msg_rsp_buff_t),
				MSG_RESULT_SUCCESS,
				"Success",
				NULL,
				0);
	}

	msg_rsp_send(client->m_syncsock, &buff.hdr);

	//dbg("Done");
	return;
} // req_handler_table_get()


void ClientMsgHndler::req_handler_host_get(Client *client, const msg_req_hdr_t *req)
{
	size_t paylen  = sizeof(msg_rsp_host_get_t) - sizeof(msg_rsp_hdr_t);
	fon_id_t host = client->m_fib->get_idtable()->get_host();

	msg_rsp_buff_t buff;
	msg_rsp_fill	(&buff.hdr,
			sizeof(msg_rsp_buff_t),
			MSG_RESULT_SUCCESS,
			"Success",
			&host,
			paylen);
	msg_rsp_send(client->m_syncsock, &buff.hdr);

	return;
} // req_handler_host_get()

