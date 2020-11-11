#ifndef _CONFIG_H_
#define _CONFIG_H_

#define FFT_N	64	/* Number of samples (64,128,256,512). Don't forget to clean! */

/* ADC Params */
#define OVERSAMPS 2 /* Artificial oversampling in software */

// Post-FFT division by two steps (i.e. divide by 2,4,8,16 etc)
#define DIVIDE_STEPS_MIN 0
#define DIVIDE_STEPS_MAX 20
// initial level
#ifdef LOG_VALUES
#define DIVIDE_STEPS_INIT (OVERSAMPS) 
#else
#define DIVIDE_STEPS_INIT (OVERSAMPS / 2)
#endif
#define DIVIDE_STEPS_CHANGE_DELAY 10 // delay before changing divide_steps

#endif
