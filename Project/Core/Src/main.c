
#include "clocks.h"
#include "i2c.h"
#include "eeprom.h"
#include "tmp_sensor.h"



int main(void)
{

	// Configure system clock
	SysClockConfig();

	// Configure I2C
	I2C_Handler_t i2c_handle;
	i2c_handle.I2C = I2C1;
	i2c_handle.speed = 100;
	if(I2C_Init(&i2c_handle) != I2C_OK){
		return 0;
	}

	EEPROM_t eeprom;

	eeprom.address = GET_ADDR(0,0,0);
	eeprom.i2c = &i2c_handle;
	if(EEPROM_Init(&eeprom) != EEPROM_OK){
		return 0;
	}

	TMP_Status_e status = TMP_Init();



	uint16_t temp;

	while (1)
	{
		Delay(1000);
		temp = TMP_Read_Volts();

	}

}

