#include <iostream>
#include <list>
#include <memory>
#include "ovs-if.hpp"

// required arguments
#define ARGV_INDEX_INTERNAL	(1)
#define ARGV_INDEX_BRNAME	(2)
#define ARGV_INDEX_MININET	(3)
#define ARGV_INDEX_REQUIRED	(4)

// options
#define ARGV_INDEX_IFLIST	(4)

int main(int argc, char **argv)
{
	if_name_t internal;
	br_name_t br_name;
	bool mininet;
	if_list_t if_list;
	std::shared_ptr<OvsIf> ovsif;

	if(argc < ARGV_INDEX_REQUIRED)
	{
		std::cout << "Usage: " << "INTENRNAL BRNAME MININET(0 or 1) [IFLIST...]" << std::endl;
		exit(EXIT_FAILURE);
	}


	internal = argv[ARGV_INDEX_INTERNAL];
	br_name = argv[ARGV_INDEX_BRNAME];
	mininet = atoi(argv[ARGV_INDEX_MININET]);

	for(int i=ARGV_INDEX_IFLIST; i < argc; i++)
	{
		if_list.emplace_back(argv[i]);
	}

	ovsif = std::make_shared<OvsIf>(nullptr, internal, std::move(br_name), std::move(if_list), mininet);

	char c;
	std::cin >> c;

	return 0;
}
