#include "stm32g071xx.h"
#include "stm32g071xx_gpio_driver.h"
#include "stm32g071xx_usart_driver.h"
#include <string.h>

USART_Handle_t USARTHandle;
uint8_t msg[] = "TX testing!\r\n";
uint8_t rxBuf[sizeof(msg)];
volatile uint8_t rxCmplt = 0;

void USART_GPIOInits(void){
	GPIO_Handle_t USARTPins;

	USARTPins.pGPIOx = GPIOA;

	USARTPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	USARTPins.GPIO_PinConfig.GPIO_PinAltFunMode = 1U;
	USARTPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	USARTPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	USARTPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_HIGH;

	// PA2 = TX
	USARTPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_2;
	GPIO_Init(&USARTPins);

	// PA3 = RX
	USARTPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_3;
	GPIO_Init(&USARTPins);
}

void USART_Inits(void){
	USARTHandle.pUSARTx = USART2;

	USARTHandle.USARTConfig.USART_Baud = USART_STD_BAUD_115200;
	USARTHandle.USARTConfig.USART_WordLength = USART_WORDLEN_8BITS;
	USARTHandle.USARTConfig.USART_NoOfStopBits = USART_STOPBITS_1;
	USARTHandle.USARTConfig.USART_ParityControl = USART_PARITY_DISABLE;
	USARTHandle.USARTConfig.USART_Mode = USART_MODE_TXRX;
	USARTHandle.USARTConfig.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE;

	USART_Init(&USARTHandle);
}

void Button_Init(void){
	GPIO_Handle_t UserButton;

	UserButton.pGPIOx = GPIOC;

	UserButton.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
	UserButton.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
	UserButton.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    UserButton.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_LOW;
    UserButton.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    UserButton.GPIO_PinConfig.GPIO_PinAltFunMode = 0U;

    GPIO_Init(&UserButton);
}

void delay(void){
    for (volatile uint32_t i = 0; i < 500000U; i++);
}


int main(void){
	USART_GPIOInits();
	USART_Inits();
	Button_Init();
	USART_IRQConfig(IRQ_NO_USART2, 3U, ENABLE);

	while (1){
		while (GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_NO_13) == GPIO_PIN_SET);
		delay();
		rxCmplt = 0;
		USART_ReceiveDataIT(&USARTHandle, rxBuf, strlen((char*)msg));
		USART_SendData(&USARTHandle, msg, strlen((char*)msg));
		while (rxCmplt == 0);
		rxBuf[sizeof(msg) - 1U] = '\0';
		while (GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_NO_13) == GPIO_PIN_RESET);
	}
}

void USART2_IRQHandler(void){
    USART_IRQHandling(&USARTHandle);
}

void USART_ApplicationEventCallback(USART_Handle_t *pHandle, uint8_t AppEv){
    if (AppEv == USART_EVENT_RX_CMPLT){
        rxCmplt = 1;
    }
}
