/*
 * Calibration.h
 *
 *  Created on: Nov 1, 2015
 *      Author: rafael
 */

#ifndef CALIBRATION_H_
#define CALIBRATION_H_

#define CAL_MODE_OFF          (0u)
#define CAL_MODE_ON           (1u)

// Stop acceleration measurement after 60 minutes to save battery
#define ACCEL_MEASUREMENT_TIMEOUT               (60 * 60u)

#include "bm.h"

// *************************************************************************************************
// Global Variable section
struct cal
{
    u8 max;
    u8 min;
    u8 points[10];                  // Data Points
    u8 mode;
    u16 timeout;                // Timeout

};
extern struct cal sCal;

// *************************************************************************************************
// Extern section
extern void reset_calib(void);
extern void sx_calibration(u8 line);
extern void mx_calibration(void);
extern void display_calibration(u8 line, u8 update);
extern u8 is_calibration(void);
extern void do_calibration(void);





#endif /* CALIBRATION_H_ */
