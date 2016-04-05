#ifndef ANT_PARAMETERS_H
#define ANT_PARAMETERS_H

/*==============================================================================
 * Parameters
 *==============================================================================*/
// 최소값을 0으로하면 매우 안좋은 결과를 보임. 0보다는 큰 값.
#define     PHEROMONE_MIN               (0.1)
#define     PHEROMONE_MAX               (99.9)


// time for the send packet
#define     WAIT_REMAIN_PKT             (1000)

#define     FORWARD_TARGET              (4)
#define     MONITOR_PERIOD_MS           (1000)

#define     CYCLE_PERIOD_MS             (50)
#define     NUMBER_OF_CYCLES            (100)
#define     PACKETS_PER_CYCLE           (10)

// The maximum value is Depend on MTU.
#define     ANT_MAXIMUM_TTL             (128)

// Evaporation rate of pheromone
#define     ANT_EVAPORATION_RATE        (0.10)
#define     ANT_REMAINS_RATE            (1-ANT_EVAPORATION_RATE)
#define     ANT_COCENTRATION_CONST      (PHEROMONE_MAX*ANT_EVAPORATION_RATE/(double)PACKETS_PER_CYCLE)

#define     ANT_MODEL_SELECTOR          (ant_colony_system_model)

// Flgas
#define     ANT_DESTINATION_UPDATE      (0)
#define     ANT_SOURCE_UPDATE           (1)
#define     ANT_ACS_UPDATE              (0)

#if !ANT_DESTINATION_UPDATE && !ANT_SOURCE_UPDATE && !ANT_ACS_UPDATE
#error At least one flag must be set.
#endif

#define     ANT_BACKTRACK_UPDATE        (0)

#endif
