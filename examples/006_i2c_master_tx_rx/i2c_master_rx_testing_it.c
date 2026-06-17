#include "stm32g071xx.h"
#include "stm32g071xx_gpio_driver.h"
#include "stm32g071xx_i2c_driver.h"
#include <string.h>

#define SLAVE_ADDR       0x68U
#define CMD_SEND_LEN	 0x51U
#define CMD_SEND_MSG	 0x52U

void I2C1_GPIOInits(void);
void I2C1_Inits(void);
void Button_Init(void);
void delay(void);

I2C_Handle_t I2C1Handle;
uint8_t len;
uint8_t rcv_buf[32];

volatile uint8_t txComplt = RESET;
volatile uint8_t rxComplt = RESET;
volatile uint8_t i2cError = RESET;

void I2C1_GPIOInits(void){
	GPIO_Handle_t I2CPins;

	I2CPins.pGPIOx = GPIOB;

	I2CPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	I2CPins.GPIO_PinConfig.GPIO_PinAltFunMode = 6U;
	I2CPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
	I2CPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PU;
	I2CPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_HIGH;

	// PB6 = I2C1_SCL
	I2CPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6;
	GPIO_Init(&I2CPins);

	// PB7 = I2C1_SDA
	I2CPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_7;
	GPIO_Init(&I2CPins);
}

void I2C1_Inits(void){
	I2C1Handle.pI2Cx = I2C1;

	I2C1Handle.I2CConfig.I2C_SCLSpeed = I2C_SCL_SPEED_SM;
	I2C1Handle.I2CConfig.I2C_DeviceAddress = 0x61U;
	I2C1Handle.I2CConfig.I2C_ACKControl = I2C_ACK_ENABLE;
	I2C1Handle.I2CConfig.I2C_FMDutyCycle = I2C_FM_DUTY_2;

	I2C_Init(&I2C1Handle);
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
	I2C1_GPIOInits();
	I2C1_Inits();
	I2C_IRQConfig(IRQ_NO_I2C1, 0, ENABLE);
	Button_Init();

	uint8_t commandCode;

	while (1){
		while(GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_NO_13) == GPIO_PIN_SET);
		delay();

		commandCode = CMD_SEND_LEN;
		txComplt = RESET;
		i2cError = RESET;

		while(I2C_MasterSendDataIT(&I2C1Handle, &commandCode, 1, SLAVE_ADDR, I2C_DISABLE_SR) != I2C_READY);
		while ((txComplt != SET) && (i2cError != SET));

		if (i2cError == SET){
			while (GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_NO_13) == GPIO_PIN_RESET);
			continue;
		}

		rxComplt = RESET;
		i2cError = RESET;

		while(I2C_MasterReceiveDataIT(&I2C1Handle, &len, 1, SLAVE_ADDR, I2C_DISABLE_SR) != I2C_READY);
		while ((rxComplt != SET) && (i2cError != SET));

		if (i2cError == SET){
			while (GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_NO_13) == GPIO_PIN_RESET);
			continue;
		}

		if (len == 0){
		    while (GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_NO_13) == GPIO_PIN_RESET);
		    continue;
		}

		if (len > sizeof(rcv_buf) - 1){
			len = sizeof(rcv_buf) - 1;
		}

		commandCode = CMD_SEND_MSG;
		txComplt = RESET;
		i2cError = RESET;

		while(I2C_MasterSendDataIT(&I2C1Handle, &commandCode, 1, SLAVE_ADDR, I2C_DISABLE_SR) != I2C_READY);
		while ((txComplt != SET) && (i2cError != SET));

		if (i2cError == SET){
			while (GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_NO_13) == GPIO_PIN_RESET);
			continue;
		}

		rxComplt = RESET;
		i2cError = RESET;
		memset(rcv_buf, 0, sizeof(rcv_buf));

		while(I2C_MasterReceiveDataIT(&I2C1Handle, rcv_buf, len, SLAVE_ADDR, I2C_DISABLE_SR) != I2C_READY);
		while ((rxComplt != SET) && (i2cError != SET));

		if (i2cError == SET){
			while (GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_NO_13) == GPIO_PIN_RESET);
			continue;
		}

		rcv_buf[len] = '\0';

		while (GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_NO_13) == GPIO_PIN_RESET);
	}
}

void I2C1_IRQHandler(void){
	I2C_IRQHandling(&I2C1Handle);
}

void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle, uint8_t AppEv)
{
    (void)pI2CHandle;

    if (AppEv == I2C_EV_TX_CMPLT){
        txComplt = SET;
    }
    else if (AppEv == I2C_EV_RX_CMPLT){
        rxComplt = SET;
    }
    else if (AppEv == I2C_ERROR_AF){
        i2cError = SET;
    }
    else if (AppEv == I2C_ERROR_BERR){
        i2cError = SET;
    }
    else if (AppEv == I2C_ERROR_ARLO){
        i2cError = SET;
    }
    else if (AppEv == I2C_ERROR_OVR){
        i2cError = SET;
    }
}
