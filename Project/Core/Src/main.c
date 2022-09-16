
#include "clocks.h"
#include "i2c.h"

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
//	if(I2C_Test_Device(&i2c_handle, 0x50) != I2C_OK){
//		return 0;
//	}
	i2c_handle.buffer[0] = 0x00;
	i2c_handle.buffer[1] = 0x00;
	i2c_handle.buffer[2] = 0x11;
	I2C_Write(&i2c_handle, 0x50, 3);

	while (1)
	{

	}

}


//#ifdef  USE_FULL_ASSERT
///**
//  * @brief  Reports the name of the source file and the source line number
//  *         where the assert_param error has occurred.
//  * @param  file: pointer to the source file name
//  * @param  line: assert_param error line source number
//  * @retval None
//  */
//void assert_failed(uint8_t *file, uint32_t line)
//{
//  /* USER CODE BEGIN 6 */
//  /* User can add his own implementation to report the file name and line number,
//     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
//  /* USER CODE END 6 */
//}
//#endif /* USE_FULL_ASSERT */
