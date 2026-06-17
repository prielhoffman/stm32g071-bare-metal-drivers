#include "stm32g071xx.h"
#include <string.h>

/*
 * Configure GPIO pins to work as SPI2 pins.
 *
 * For this test we only need:
 * PB13 -> SPI2_SCK
 * PB15 -> SPI2_MOSI
 *
 * This example transmits a fixed string repeatedly.
 */

static void SPI2_GPIOInits(void);
static void SPI2_Inits(void);

/* Configure GPIO pins for SPI2 alternate function */
void SPI2_GPIOInits(void){
	GPIO_Handle_t SPIPins;

	SPIPins.pGPIOx = GPIOB;

	SPIPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	SPIPins.GPIO_PinConfig.GPIO_PinAltFunMode = 0U; /* AF0 for SPI2 on PB13/PB15 in STM32G071 */
	SPIPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	SPIPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_HIGH;

	/*
	 * PB13 -> SPI2_SCK
	 */
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
	GPIO_Init(&SPIPins);


    /*
     * PB15 -> SPI2_MOSI
     */
    SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_15;
    GPIO_Init(&SPIPins);
}

/* Configure SPI2 as master, full-duplex, 8-bit data frame */
void SPI2_Inits(void)
{
    SPI_Handle_t SPI2Handle;

    SPI2Handle.pSPIx = SPI2;

    SPI2Handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;
    SPI2Handle.SPIConfig.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER;
    SPI2Handle.SPIConfig.SPI_SclkSpeed = SPI_SCLK_SPEED_DIV16;
    SPI2Handle.SPIConfig.SPI_DataSize = SPI_DS_8BITS;
    SPI2Handle.SPIConfig.SPI_CPOL = SPI_CPOL_LOW;
    SPI2Handle.SPIConfig.SPI_CPHA = SPI_CPHA_LOW;
    SPI2Handle.SPIConfig.SPI_SSM = SPI_SSM_EN;

    SPI_Init(&SPI2Handle);
}

int main(void)
{
	char user_data[] = "Hello World";

	SPI2_GPIOInits();
	SPI2_Inits();
	SPI_PeripheralControl(SPI2, ENABLE);

	SPI_SendData(SPI2, (uint8_t*)user_data, strlen(user_data));

	while (1)
	{
		SPI_SendData(SPI2, (uint8_t*)user_data, strlen(user_data));

	    for (volatile uint32_t i = 0; i < 500000; i++);
	}
}
