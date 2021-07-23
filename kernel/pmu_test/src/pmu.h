
#ifndef _PMU_H_
#define _PMU_H_

void pmu_start(void);

void pmu_reset_counter(void);
void pmu_reset_evnt_counter(void);
void pmu_enable_counter(void);
void pmu_enable_event0(void);
void pmu_set_event0(int event);

unsigned int pmu_read_counter(void);
unsigned int pmu_read_event0(void);

#endif /* _PMU_H_ */
