#include "ant-model.h"
#include "ant-def.h"

void ant_normalizing_model(aco_ph_t* ph, aco_dist_t global_min, aco_dist_t local_min, aco_dist_t dist)
{
    int normalized = local_min - global_min + 1;
    int divider = normalized*normalized;

    *ph += CONCENTRATION_RATE / (double)(divider);
}

void ant_local_model(aco_ph_t* ph, aco_dist_t global_min, aco_dist_t local_min, aco_dist_t dist)
{
    *ph += CONCENTRATION_RATE / (double)(local_min);
}

void ant_density_model(aco_ph_t* ph, aco_dist_t global_min, aco_dist_t local_min, aco_dist_t dist)
{
    *ph += CONCENTRATION_RATE;
}

void ant_system_model(aco_ph_t* ph, aco_dist_t global_min, aco_dist_t local_min, aco_dist_t dist)
{
    *ph += CONCENTRATION_RATE / (double)(dist);
}

void ant_colony_system_model(aco_ph_t* ph, aco_dist_t global_min, aco_dist_t local_min, aco_dist_t dist)
{
    if(global_min == dist)
    {
        *ph += CONCENTRATION_RATE / (double)(dist);
    }
}
