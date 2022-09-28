/*
 * tmp_sensor.h
 *
 *  Created on: Sep. 27, 2022
 *      Author: lemck
 */

#ifndef INC_TMP_SENSOR_H_
#define INC_TMP_SENSOR_H_

#include <stdlib.h>
#include "stm32f429xx.h"

#define BUFFER_SIZE	20

typedef enum {
	TMP_OK,
	TMP_ERR
}TMP_Status_e;

TMP_Status_e TMP_Init(void);
uint8_t TMP_Read_Volts(void);


#endif /* INC_TMP_SENSOR_H_ */
