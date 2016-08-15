#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <glib.h>

typedef struct _aco_parameters
{
	int fon_client_port;
	int ipc_listen_port;
	
	// the number of packets per cycle
	int packets_per_cycle;

	// evapor interval(cycle) in milisecond
	guint cycle_interval;

	// 페로몬 테이블 참조
	aco_ph_t max;
	aco_ph_t min;
	int endurance_max;

	// 증발계수 등
	double evapor_rate;
	double remain_rate;
	double concen_rate;
} aco_parameters;

static inline
void init_parameters(aco_parameters *para,
		int fon_client_port,
		int ipc_listen_port,
		int packets_per_cycle,
		int cycle_interval,
		aco_ph_t max,
		aco_ph_t min,
		int endurance_max,
		double evapor_rate)
{
	para->fon_client_port		= fon_client_port;
	para->ipc_listen_port		= ipc_listen_port;
	para->packets_per_cycle		= packets_per_cycle;
	para->cycle_interval		= cycle_interval;
	para->max			= max;
	para->min			= min;
	para->endurance_max		= endurance_max;
	para->evapor_rate		= evapor_rate;
	para->remain_rate		= 1 - evapor_rate;
	para->concen_rate		= (max*evapor_rate/(double)packets_per_cycle);
}

#endif /* PARAMETERS_H */
