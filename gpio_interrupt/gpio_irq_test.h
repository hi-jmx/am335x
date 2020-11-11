#ifndef __GPIO_IRQ_TEST_H__
#define __GPIO_IRQ_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif




extern void DMTimer7ModuleClkConfig(void);
extern void IntRegister(unsigned int intrNum, void (*fnHandler)(void));
extern void IntPrioritySet(unsigned int intrNum, unsigned int priority, 
                            unsigned int hostIntRoute);
extern void IntSystemEnable(unsigned int intrNum);
extern void DMTimerCounterSet(unsigned int baseAdd, unsigned int counter);
extern void DMTimerModeConfigure(unsigned int baseAdd, unsigned int timerMode);











#ifdef __cplusplus
}
#endif

#endif