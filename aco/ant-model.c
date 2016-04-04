#include "ant-model.h"
#include "ant-def.h"

void ant_normalizing_model(pheromone_t *ph, int global_min, int local_min, int nhops, bool increase)
{
    int normalized = local_min - global_min + 1;
    int divider = normalized*normalized;

    if(increase)
    {
        // 정규화된 거리를 이용하여 페로몬을 갱신한다.
        // ANT_COCENTRATION_CONST를 적절히 선택했기 때문에
        // ACO_TABLE_PHEROMONE_MAX 값을 넘어 설 수 없다.
        *ph += ANT_COCENTRATION_CONST / (double)(divider);
    }
    else
    {
        *ph *= ANT_REMAINS_RATE;
    }
}

void ant_local_model(pheromone_t *ph, int global_min, int local_min, int nhops, bool increase)
{
    if(increase)
    {
        *ph += ANT_COCENTRATION_CONST / (double)(local_min);
    }
    else
    {
        *ph *= ANT_REMAINS_RATE;
    }
}

void ant_density_model(pheromone_t *ph, int global_min, int local_min, int nhops, bool increase)
{
    if(increase)
    {
        *ph += ANT_COCENTRATION_CONST;
    }
    else
    {
        *ph *= ANT_REMAINS_RATE;
    }
}

void ant_system_model(pheromone_t *ph, int global_min, int local_min, int nhops, bool increase)
{
    if(increase)
    {
        *ph += ANT_COCENTRATION_CONST / (double)(nhops);
    }
    else
    {
        *ph *= ANT_REMAINS_RATE;
    }
}

void ant_colony_system_model(pheromone_t *ph, int global_min, int local_min, int nhops, bool increase)
{
    if(increase &&global_min == nhops)
    {
        *ph += ANT_COCENTRATION_CONST / (double)(nhops);
    }
    else
    {
        *ph = (*ph)*ANT_REMAINS_RATE;
    }
}
