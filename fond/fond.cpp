#include "dbg.hpp"
#include "fond.hpp"

FonDaemon::FonDaemon
	(fon_id_t host,
	if_name_t internal,
	br_name_t bridge,
	if_list_t if_list,
	bool mininet,
	int port)

	:m_context(g_main_context_new()),
	m_loop(g_main_loop_new(m_context, FALSE)),
	m_internal(internal)
{
	DBG_PRINT_VALUE(host);
	DBG_PRINT_VALUE(internal);
	DBG_PRINT_VALUE(bridge);
	//DBG_PRINT_VALUE(if_list);
	DBG_PRINT_VALUE(mininet);
	DBG_PRINT_VALUE(port);

	m_idtable = std::make_shared<IdTable>(host);

	m_ovsif = std::make_shared<OvsIf>(m_idtable,
					m_internal,
					bridge,
					if_list,
					mininet);

	m_fib = std::make_shared<Fib>(m_idtable,
					m_ovsif);

	m_cm = std::make_shared<IpcManager>(m_context,
					m_loop,
					m_fib,
					port);

	m_ether = std::make_shared<EtherLink>(m_context,
				m_loop,
				m_internal,
				m_idtable,
				m_cm);
}


FonDaemon::~FonDaemon()
{
	g_main_loop_unref(m_loop);
	g_main_context_unref(m_context);
}

void FonDaemon::run()
{
	g_main_loop_run(m_loop);
}
