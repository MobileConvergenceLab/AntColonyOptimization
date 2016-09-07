#include <stdio.h>
#include "acod.h"
#include "aco-policies.h"

typedef struct _MainArg
{
	int	fon_port;
	int	aco_port;
} MainArg;

bool argument_parse(MainArg *arg, int argc, char **argv)
{
	memset(arg, 0, sizeof(MainArg));

	if(argc < 3)
	{
		printf("Not Enough arguments\n");
		printf("Uasage: %s FON_PORT ACO_PORT\n", argv[0]);
		return false;
	}

	arg->fon_port = atoi(argv[1]);
	arg->aco_port = atoi(argv[2]);

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
			arg.fon_port,
			arg.aco_port,
			PACKETS_PER_CYCLE,
			CYCLE_INTERVAL,
			PHEROMONE_MAX,
			PHEROMONE_MIN,
			ENDURANCE_MAX,
			ANT_EVAPORATION_RATE);

	AcoDaemon* daemon = aco_daemon_create(&para);

	// run
	aco_daemon_run(daemon);

	return 0;
}
