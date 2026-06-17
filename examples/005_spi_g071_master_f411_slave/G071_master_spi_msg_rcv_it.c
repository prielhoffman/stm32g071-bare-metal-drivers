#include "stm32g071xx.h"
#include "stm32g071xx_gpio_driver.h"
#include "stm32g071xx_spi_driver.h"

// Function prototypes
void SPI2_Master_GPIOInits(void);
void SPI2_Master_Inits(void);
void delay(void);

volatile uint8_t dataAvailable = 0;
volatile uint8_t spiRxDone = 0;
SPI_Handle_t SPI2Handle;

typedef enum{
    MASTER_WAIT_DATA_READY,
    MASTER_START_SPI,
    MASTER_RECEIVE_BYTE,
    MASTER_WAIT_BYTE_DONE,
    MASTER_CHECK_BYTE,
    MASTER_DONE
} MasterState_t;

volatile MasterState_t masterState = MASTER_WAIT_DATA_READY;

void SPI2_Master_GPIOInits(void){
	GPIO_Handle_t SPIPins;

	SPIPins.pGPIOx = GPIOB;
	SPIPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	SPIPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	SPIPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_HIGH;

	// In STM32G071, SPI2 Alternate Function for PB13, PB14, PB15 is AF0
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

	SPI2Handle.pSPIx = SPI2; // Target the SPI2 peripheral
	SPI2Handle.SPIConfig.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER;
	SPI2Handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;

	// Low clock speed for safe and stable communication across jumper wires
	SPI2Handle.SPIConfig.SPI_SclkSpeed = SPI_SCLK_SPEED_DIV32;

	SPI2Handle.SPIConfig.SPI_CPOL = SPI_CPOL_LOW;
	SPI2Handle.SPIConfig.SPI_CPHA = SPI_CPHA_LOW;

	SPI2Handle.SPIConfig.SPI_SSM = SPI_SSM_EN; // Enable Software Slave Management
	SPI2Handle.SPIConfig.SPI_DataSize = SPI_DS_8BITS; // 8-bit data frame format

	SPI_Init(&SPI2Handle);
}

void DataReady_GPIOInterruptInit(void){
	GPIO_Handle_t DataReadyPin;

	DataReadyPin.pGPIOx = GPIOA;
	DataReadyPin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
	DataReadyPin.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IT_FT;
	DataReadyPin.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;
	DataReadyPin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PU;
	DataReadyPin.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;

	GPIO_Init(&DataReadyPin);

    /*
     * PA0 belongs to EXTI0_1 on STM32G071.
     * Enable this interrupt line in NVIC.
     */
	GPIO_IRQConfig(IRQ_NO_EXTI0_1, 1, ENABLE);
	GPIO_IRQPriorityConfig(IRQ_NO_EXTI0_1, 1);
}

void delay(void){
	for(volatile uint32_t i = 0; i < 500000; i++);
}


int main(void){
	SPI2_Master_GPIOInits();
	SPI2_Master_Inits();
	SPI_IRQPriorityConfig(IRQ_NO_SPI2, 2);
	SPI_IRQConfig(IRQ_NO_SPI2, 2, ENABLE);
	DataReady_GPIOInterruptInit();

	uint8_t dummyWrite = 0xFF;
	uint8_t receivedByte = 0;
	uint8_t rxBuffer[100];
	uint32_t rxIndex = 0;

	while (1){
		switch (masterState){
			case MASTER_WAIT_DATA_READY:
	            /*
	             * Wait until the slave notifies us
	             * that a message is ready.
	             */
				if (dataAvailable == 1){
					masterState = MASTER_START_SPI;
				}
				break;
			case MASTER_START_SPI:
	            /*
	             * - Pull NSS LOW
	             * - Enable SPI2
	             */
				GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_12, RESET);
				SPI_PeripheralControl(SPI2, ENABLE);

				masterState = MASTER_RECEIVE_BYTE;
				break;
			case MASTER_RECEIVE_BYTE:
				spiRxDone = 0;
				SPI_ReceiveDataIT(&SPI2Handle, &receivedByte, 1);
				SPI_SendDataIT(&SPI2Handle, &dummyWrite, 1);
				masterState = MASTER_WAIT_BYTE_DONE;
				break;
			case MASTER_WAIT_BYTE_DONE:
			    /*
			     * Wait until SPI RX interrupt says that one byte was received.
			     */
			    if (spiRxDone == 1){
			        spiRxDone = 0;
			        masterState = MASTER_CHECK_BYTE;
			    }
			    break;
			case MASTER_CHECK_BYTE:
			    /*
			     * Store the received byte in the receive buffer.
			     */
			    rxBuffer[rxIndex] = receivedByte;

			    /*
			     * If received byte is '\0', the message is complete.
			     */
			    if (receivedByte == '\0'){
			        masterState = MASTER_DONE;
			    }
			    else{
			        rxIndex++;

			        /*
			         * Prevent buffer overflow.
			         * Leave room for a terminating '\0'.
			         */
			        if (rxIndex >= (sizeof(rxBuffer) - 1U)){
			            rxBuffer[rxIndex] = '\0';
			            masterState = MASTER_DONE;
			        }
			        else{
			            masterState = MASTER_RECEIVE_BYTE;
			        }
			    }
			    break;
			case MASTER_DONE:
	            /*
	             * - Pull NSS HIGH
	             * - Disable SPI2
	             * - Reset flags/index
	             */
				while (SPI_GetFlagStatus(SPI2, SPI_BSY_FLAG));
				GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_12, SET);
				SPI_PeripheralControl(SPI2, DISABLE);
				dataAvailable = 0;
				rxIndex = 0;
	            masterState = MASTER_WAIT_DATA_READY;
				break;
	        default:
	            masterState = MASTER_WAIT_DATA_READY;
	            break;
		}
	}
}

void EXTI0_1_IRQHandler(void)
{
    /*
     * Clear EXTI pending bit for PA0.
     */
    GPIO_IRQHandling(GPIO_PIN_NO_0);

    /*
     * Mark that the slave has data ready.
     */
    dataAvailable = 1;
}

void SPI2_IRQHandler(void)
{
    SPI_IRQHandling(&SPI2Handle);
}

void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle, uint8_t AppEv)
{
    if (AppEv == SPI_EVENT_RX_CMPLT)
    {
        /*
         * One byte was received by SPI interrupt.
         */
        spiRxDone = 1;
    }
}
