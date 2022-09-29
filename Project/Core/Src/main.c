
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

	// Configure EEPROM
	EEPROM_t eeprom;
	eeprom.address = GET_ADDR(0,0,0);
	eeprom.i2c = &i2c_handle;
	if(EEPROM_Init(&eeprom) != EEPROM_OK){
		return 0;
	}

	//Initialize temperature sensor
	TMP_Init();

	while (1)
	{
		if(Timer_Flag_Set()){
			uint8_t temp_data = TMP_Read_Volts();
			if(EEPROM_Log_Data(&eeprom, &temp_data, 1) != EEPROM_OK){
				return 0;
			}
		}

	}

}

