#ifndef __PROGRAM_RUN_FUNCTION_H
#define __PROGRAM_RUN_FUNCTION_H

#ifdef __PROGRAM_RUN_FUNCTION_C

void stop_spin(void);
void middle_spin(void);
void high_spin(void);
void middle_spin_run(void);
void high_spin_run(void);


#else
extern void stop_spin(void);
extern void middle_spin(void);
extern void high_spin(void);
extern void middle_spin_run(void);
extern void high_spin_run(void);



#endif
#endif






