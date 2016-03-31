#ifndef PARPAMETER_H
#define PARPAMETER_H

/*==============================================================================
 * 원래 main-def.h에 있던 파라미터들
 *==============================================================================*/
// 최소값을 0으로하면 매우 안좋은 결과를 보임. 0보다는 큰 값.
#define     PHEROMONE_MIN           (0.1)
#define     PHEROMONE_MAX           (99.9)


#define     WAIT_REMAIN_PKT         (3000)
#define     FORWARD_PERIOD_MS       (10)
#define     FORWARD_TARGET          (15)
#define     MONITOR_PERIOD_MS       (1000)

#define     HOW_MANY_TX             (500)

/*==============================================================================
 * 원래 ant-def.h에 있던 파라미터들
 *==============================================================================*/
#define     ANT_DEFAULT_TTL             (ANT_OBJ_MAXIMUM_ARR_SIZE)
#define     ANT_EVAPORATION_RATE        (0.10)
#define     ANT_REMAINS_RATE            (1-ANT_EVAPORATION_RATE)
#define     ANT_COCENTRATION_CONST      (PHEROMONE_MAX*ANT_EVAPORATION_RATE)

#define     ANT_MODEL_SELECTOR          (ant_normalizing_model)
#define     ANT_DESTINATION_UPDATE      (0)
#define     ANT_SOURCE_UPDATE           (1)
#define     ANT_BACKTRACK_UPDATE        (0)

#endif /* PARPAMETER_H */
