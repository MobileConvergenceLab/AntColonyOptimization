#include <stdio.h>
#include "acod.h"
#include "aco-policies.h"

typedef struct _MainArg
{
	int	port;
	int	test_flag;
	int	test_target;
} MainArg;

bool argument_parse(MainArg *arg, int argc, char **argv)
{
	memset(arg, 0, sizeof(MainArg));

	if(argc < 2)
	{
		printf("Not Enough arguments\n");
		return false;
	}

	arg->port = atoi(argv[1]);

	if(argc == 4)
	{
		arg->test_flag = 1;
		arg->test_target = atoi(argv[3]);
	}
	else
	{
		arg->test_flag = 0;
	}

	return true;
};

int main(int argc, char **argv)
{
	MainArg arg;
	aco_parameters para;

	if(!argument_parse(&arg, argc, argv))
	{
		exit(EXIT_FAILURE);
	}

	init_parameters(&para,
			// fon port
			arg.port,
			// aco port
			0,
			PACKETS_PER_CYCLE,
			CYCLE_INTERVAL,
			PHEROMONE_MAX,
			PHEROMONE_MIN,
			ENDURANCE_MAX,
			ANT_EVAPORATION_RATE);

	AcoDaemon* daemon = aco_daemon_create(&para);

	printf("arg.test_flag: %d\n", arg.test_flag);

	if(arg.test_flag == 1)
	{
		aco_id_t target		= arg.test_target;
		int ncycle		= 10;
		int npacket_per_cycle	= 10;
		// the timeout interval in milliseconds
		guint interval		= 2000;

		if(!aco_table_is_neigh(daemon->table, target))
		{
			aco_table_add_target(daemon->table, target);
		}

		aco_daemon_request_attach(daemon,
					target,
					ncycle,
					npacket_per_cycle,
					interval);
	}

	// run
	aco_daemon_run(daemon);

	return 0;
}
