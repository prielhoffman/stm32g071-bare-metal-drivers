#include "stm32g071xx.h"
#include "stm32g071xx_gpio_driver.h"
#include "stm32g071xx_spi_driver.h"

/* SPI command codes supported by the F411 slave */
#define COMMAND_LED_CTRL       0x50
#define COMMAND_SENSOR_READ    0x51
#define COMMAND_LED_READ       0x52
#define COMMAND_PRINT          0x53
#define COMMAND_ID_READ        0x54

void SPI2_Master_GPIOInits(void);
void SPI2_Master_Inits(void);
void Button_Init(void);
void LED_Init(void);
void delay(void);

void SPI2_Master_GPIOInits(void){
	GPIO_Handle_t SPIPins;

	SPIPins.pGPIOx = GPIOB;
	SPIPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	SPIPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	SPIPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_HIGH;

	// SPI2 alternate function for PB13, PB14 and PB15 on STM32G071
	SPIPins.GPIO_PinConfig.GPIO_PinAltFunMode = 0;

	// PB13 -> SPI2_SCK
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
	GPIO_Init(&SPIPins);

	// PB14 -> SPI2_MISO
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_14;
	GPIO_Init(&SPIPins);

	// PB15 -> SPI2_MOSI
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_15;
	GPIO_Init(&SPIPins);

	// PB12 is used as software-controlled NSS
	SPIPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
	GPIO_Init(&SPIPins);

	// NSS idle state is HIGH
	GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_12, GPIO_PIN_SET);
}

void SPI2_Master_Inits(void){
	SPI_Handle_t SPI2Handle;

	SPI2Handle.pSPIx = SPI2;
	SPI2Handle.SPIConfig.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER;
	SPI2Handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;

	// Slow clock for stable communication over jumper wires
	SPI2Handle.SPIConfig.SPI_SclkSpeed = SPI_SCLK_SPEED_DIV32;

	SPI2Handle.SPIConfig.SPI_CPOL = SPI_CPOL_LOW;
	SPI2Handle.SPIConfig.SPI_CPHA = SPI_CPHA_LOW;

	SPI2Handle.SPIConfig.SPI_SSM = SPI_SSM_EN; // Software slave management
	SPI2Handle.SPIConfig.SPI_DataSize = SPI_DS_8BITS; // 8-bit data frame format

	SPI_Init(&SPI2Handle);
}

void Button_Init(void){
	GPIO_Handle_t UserButton;

	UserButton.pGPIOx = GPIOC;
	UserButton.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13; // Built-in Blue Button on PC13
	UserButton.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
	UserButton.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_HIGH;
	UserButton.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_Init(&UserButton);
}

void LED_Init(void)
{
    GPIO_Handle_t Led;

    Led.pGPIOx = GPIOA;
    Led.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_5;
    Led.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    Led.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;
    Led.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    Led.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    Led.GPIO_PinConfig.GPIO_PinAltFunMode = 0;

    GPIO_Init(&Led);
}

void delay(void){
	for(volatile uint32_t i = 0; i < 500000; i++);
}


int main(void){
	uint8_t command_code = COMMAND_LED_CTRL;
	uint8_t dummy_read;
	uint8_t dummy_write = 0xFF;
	uint8_t ack_byte = 0;
	uint8_t state = 0;
	uint8_t analog_read_value = 0;
	uint8_t led_status = 0;
	uint8_t message[] = "Hello! How are you?";
	uint8_t id[11];

	// Step 1: Hardware Setup and Initialization
	SPI2_Master_GPIOInits();
	SPI2_Master_Inits();
	Button_Init();
	LED_Init();

	while(1){
		// Step 2: Polling - Wait here as long as the button (PC13) is NOT pressed (remains HIGH)
		while (GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_NO_13) == GPIO_PIN_SET);

		// Software debounce delay to filter out mechanical button bounce noise
		delay();

		if (state == 0 || state == 1)
		{
		    command_code = COMMAND_LED_CTRL;
		}
		else if (state == 2)
		{
		    command_code = COMMAND_SENSOR_READ;
		}
		else if (state == 3)
		{
		    command_code = COMMAND_LED_READ;
		}
		else if (state == 4)
		{
		    command_code = COMMAND_PRINT;
		}
		else if (state == 5)
		{
		    command_code = COMMAND_ID_READ;
		}

		// Step 3: Turn on the SPI Peripheral and pull NSS (PB12) LOW to activate the Slave
		SPI_PeripheralControl(SPI2, ENABLE);
		GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_12, GPIO_PIN_RESET);

		// Step 4: Transmit the 1-byte command code (0x50) to the slave
		SPI_SendData(SPI2, &command_code, 1);
		// DUMMY READ: Must read the received byte to clear the RXNE flag in hardware
		SPI_ReceiveData(SPI2, &dummy_read, 1);

		// Step 5: Send a dummy byte (0xFF) to generate clock pulses so the slave can reply
		SPI_SendData(SPI2, &dummy_write, 1);
		// Read the actual response shift-registered from the slave (We expect 0xF5)
		SPI_ReceiveData(SPI2, &ack_byte, 1);

		// Step 6: If the slave verified and replied with a valid ACK (0xF5), send the parameters
		if (ack_byte == 0xF5){
			if (command_code == COMMAND_LED_CTRL){
			uint8_t args[2];
			args[0] = 5;	// Argument 1: Target LED Pin number 5 on the slave board

			if (state == 0){
				args[1] = 1;
				}
			else{
				args[1] = 0;
			}

			SPI_SendData(SPI2, args, 2);
			SPI_ReceiveData(SPI2, &dummy_read, 1);
			SPI_ReceiveData(SPI2, &dummy_read, 1);
			}
			else if (command_code == COMMAND_SENSOR_READ){
				uint8_t analog_pin = 0;

				SPI_SendData(SPI2, &analog_pin, 1);
				SPI_ReceiveData(SPI2, &dummy_read, 1);

				for(volatile uint32_t i = 0; i < 5000; i++);

				SPI_SendData(SPI2, &dummy_write, 1);
				SPI_ReceiveData(SPI2, &analog_read_value, 1);

			}
			else if (command_code == COMMAND_LED_READ){
				SPI_SendData(SPI2, &dummy_write, 1);
				SPI_ReceiveData(SPI2, &led_status, 1);

				if (led_status == 1){
					GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_5, GPIO_PIN_SET);
				}
				else{
					GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_5, GPIO_PIN_RESET);
				}
			}
			else if (command_code == COMMAND_PRINT){
				for (uint32_t i = 0; i < sizeof(message); i++){
					SPI_SendData(SPI2, &message[i], 1);
					SPI_ReceiveData(SPI2, &dummy_read, 1);
				    }
			}
			else if (command_code == COMMAND_ID_READ){
				for (uint32_t i = 0; i < 10; i++){
					SPI_SendData(SPI2, &dummy_write, 1);
					SPI_ReceiveData(SPI2, &id[i], 1);
				}
				id[10] = '\0';
			}
		}

		for(volatile uint32_t i = 0; i < 1000; i++);

		// Step 7: Wait until the hardware finishes pushing the very last bit out of the wire
		while (SPI_GetFlagStatus(SPI2, SPI_BSY_FLAG));

		// Step 8: Set NSS (PB12) back to HIGH to deselect the Slave and end this communication cycle
		GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_12, GPIO_PIN_SET);

		SPI_PeripheralControl(SPI2, DISABLE);

		while (GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_NO_13) == GPIO_PIN_RESET);
		delay();

		state++;
		if (state > 5){
			state = 0;
		}
	}
}
