#ifndef ANT_PARAMETERS_H
#define ANT_PARAMETERS_H

/*==============================================================================
 * Parameters
 *==============================================================================*/
// Evaporation rate of pheromone
#define     CYCLE_INTERVAL              (2000)
#define     PACKETS_PER_CYCLE           (10)
#define     ENDURANCE_MAX               (PACKETS_PER_CYCLE*2)
#define     PHEROMONE_MAX               (99.9)
#define     PHEROMONE_MIN               (0.1)
#define     ANT_EVAPORATION_RATE        (0.50)
#define     ANT_REMAIN_RATE             (1-ANT_EVAPORATION_RATE)
#define     CONCENTRATION_RATE          (PHEROMONE_MAX*ANT_EVAPORATION_RATE/(double)PACKETS_PER_CYCLE)

/*==============================================================================
 * Conditional compile flags
 *==============================================================================*/
// 개미 모델 선택
// - 개미 모델에 따라서 성능차이가 현저하게 들어남
// - ant-model.[hc] 파일 참조
#define     ANT_MODEL_SELECTOR          (ant_normalizing_model)

// Endurance 알고리즘 적용 여부
#define     ENDURANCE_ENABLE            (1)

// 페로몬 정책 선택
// - 개미(패킷)을 수신했을때 테이블을 업데이트할지, 아니면
//   발신할 때 페로몬 테이블을 업데이트를 할지 정책을 결정
#define     ANT_DESTINATION_UPDATE      (0)
#define     ANT_SOURCE_UPDATE           (1)
#define     ANT_ACS_UPDATE              (0)

#if (ANT_DESTINATION_UPDATE + ANT_SOURCE_UPDATE + ANT_ACS_UPDATE) != 1
# error ONLY ONE FLAG MUST BE SET.
#endif

// 백트랙 업데이트 알고리즘 적용 여부
#define     ANT_BACKTRACK_UPDATE        (1)

#endif // ANT_PARAMETERS_H
