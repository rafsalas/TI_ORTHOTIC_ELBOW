/*
 * Calibration.c
 *
 *  Created on: Nov 1, 2015
 *      Author: rafael
 */
#include "Project.h"
// driver
#include "display.h"
#include "bmp_as.h"
#include "cma_as.h"
#include "as.h"

// logic
#include "Calibration.h"
#include "ports.h"
#include "simpliciti.h"
#include "user.h"
struct cal sCal;


void reset_calib(void){

    // Default mode is off
    sCal.mode = CAL_MODE_OFF;

}
void sx_calibration(u8 line){
	//display_chars(LCD_SEG_L1_3_0, " CAL", SEG_ON_BLINK_OFF  );

	//display_symbol(LCD_ICON_BEEPER1, SEG_OFF_BLINK_OFF);

}

int tog_signal(int status){
	if(!status){
		display_symbol(LCD_ICON_BEEPER1, SEG_ON_BLINK_ON);
		display_symbol(LCD_ICON_BEEPER2, SEG_ON_BLINK_ON);
		display_symbol(LCD_ICON_BEEPER3, SEG_ON_BLINK_ON);
		//send signal for max
	}
	else{
		display_symbol(LCD_ICON_BEEPER1, SEG_ON_BLINK_OFF);
		display_symbol(LCD_ICON_BEEPER2, SEG_ON_BLINK_OFF);
		display_symbol(LCD_ICON_BEEPER3, SEG_ON_BLINK_OFF);
		//stop signal
	}
	return !status;
}

void mx_calibration(void){
	int flag1 = 0;
	int flag2 = 0;
	//display_symbol(LCD_ICON_BEEPER1, SEG_ON);
	//display_symbol(LCD_ICON_BEEPER2, SEG_ON);
	//display_symbol(LCD_ICON_BEEPER3, SEG_ON);
	display_symbol(LCD_ICON_BEEPER1, SEG_ON_BLINK_ON);
	display_symbol(LCD_ICON_BEEPER2, SEG_ON_BLINK_ON);
	display_symbol(LCD_ICON_BEEPER3, SEG_ON_BLINK_ON);

	while(1){
		/*if(button.flag.up){
			if(!flag1){
				;//send sig
			}
			flag1 = tog_signal(flag1);
		}
		else if(button.flag.down){
			if(!flag2){
				;//send sig
			}
			flag2 = tog_signal(flag2);
		}*/
		if(button.flag.star){
			break;
		}
	}
	//display_chars(LCD_SEG_L1_3_0, " CAL", SEG_ON_BLINK_OFF );
	display_symbol(LCD_ICON_BEEPER1, SEG_OFF_BLINK_OFF);
	display_symbol(LCD_ICON_BEEPER2, SEG_OFF_BLINK_OFF);
	display_symbol(LCD_ICON_BEEPER3, SEG_OFF_BLINK_OFF);
}

void display_calibration(u8 line, u8 update){
    if (update == DISPLAY_LINE_UPDATE_FULL)
    {
        display_chars(LCD_SEG_L1_3_0, " CAL", SEG_ON);
        //display_symbol(LCD_ICON_HEART, SEG_ON_BLINK_ON);

        // Set mode
        sCal.mode = CAL_MODE_ON;
    }
    else if (update == DISPLAY_LINE_UPDATE_PARTIAL)
    {
    	display_chars(LCD_SEG_L1_3_0, " CAL", SEG_ON);
    }
    else if (update == DISPLAY_LINE_CLEAR)
    {
        display_symbol(LCD_ICON_BEEPER1, SEG_OFF);

        reset_calib();
    }

}

u8 is_calibration(void){

	return ((sCal.mode == CAL_MODE_ON) && (sCal.timeout > 0));
}
void do_calibration(void){
	display_chars(LCD_SEG_L1_3_0, " CAL", SEG_ON_BLINK_ON );

	display_symbol(LCD_ICON_BEEPER1, SEG_ON_BLINK_ON);
}

