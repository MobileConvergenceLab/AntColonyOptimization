#include "etherlink.hpp"
#include <iostream>

#define ARGV_INDEX_INTERNAL	(1)
#define ARGV_INDEX_HOST		(2)
#define ARGV_INDEX_REQUIRED	(3)

int main(int argc, char **argv)
{
	if(argc < ARGV_INDEX_REQUIRED)
	{
		std::cout << "Usage: " << argv[0] << " " << "INTENRNAL HOST" << std::endl;
		exit(EXIT_FAILURE);
	}

	GMainContext *m_context;
	GMainLoop *m_loop;
	EtherLinkPtr m_ether;
	if_name_t internal = argv[ARGV_INDEX_INTERNAL];
	fon_id_t host = atoi(argv[ARGV_INDEX_HOST]);
	IdTablePtr idtable(new IdTable(host));



	m_context = g_main_context_new();
	m_loop = g_main_loop_new(m_context, FALSE);
	m_ether = std::make_shared<EtherLink>(m_context,
			m_loop,
			internal,
			idtable);

	g_main_loop_run(m_loop);

	return 0;
}
