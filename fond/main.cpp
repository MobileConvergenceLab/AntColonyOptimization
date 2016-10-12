#include <iostream>
#include "fond.hpp"

// required arguments
#define ARGV_INDEX_HOST		(1)
#define ARGV_INDEX_INTERNAL	(2)
#define ARGV_INDEX_BRNAME	(3)
#define ARGV_INDEX_BRCREATE	(4)
#define ARGV_INDEX_LISTENPORT	(5)
#define ARGV_INDEX_REQUIRED	(6)

// options
#define ARGV_INDEX_IFLIST	(ARGV_INDEX_REQUIRED)

int main(int argc, char **argv)
{
	fon_id_t	host;
	if_name_t	internal;
	br_name_t	bridge;
	bool		br_create;
	int		port;
	if_list_t	if_list;

	if(argc < ARGV_INDEX_REQUIRED)
	{
		std::cout << "Usage: " << "HOST INTERNAL BRNAME MININET PORT [IFLIST...]" << std::endl;

		std::cout
			<< "\n"
		        << "  " << "Option     Description\n"
			<< "  " << "HOST       FON host ID.\n"
			<< "  " << "INTERNAL   Internal interface which will be conneted to OVS-Bridge.\n"
			<< "  " << "BRNAME     The name of OVS-Bridge which will be made.\n"
			<< "  " << "BR_CREATE  Create Bridge using the given br-name. If Mininet, Do check this flag. \n"
			<< "  " << "PORT       IPC listen port number\n"
			<< "  " << "IFLIST     Physical interface names\n"
			<< std::endl;

		exit(EXIT_FAILURE);
	}

//	host		= atoi(argv[ARGV_INDEX_HOST]);
	host		= inet_addr(argv[ARGV_INDEX_HOST]);
	internal	= if_name_t(argv[ARGV_INDEX_INTERNAL]);
	bridge		= argv[ARGV_INDEX_BRNAME];
	br_create	= atoi(argv[ARGV_INDEX_BRCREATE]);
	port		= atoi(argv[ARGV_INDEX_LISTENPORT]);

	for(int i=ARGV_INDEX_IFLIST; i < argc; i++)
	{
		if_list.emplace_back(argv[i]);
	}

	FonDaemon *daemon;
	daemon = new FonDaemon(host,
				internal,
				bridge,
				if_list,
				br_create,
				port);

	daemon->run();

	std::cout << "Test Done" << std::endl;

	return 0;
}
