#include "stm32g071xx.h"
#include "stm32g071xx_gpio_driver.h"
#include "stm32g071xx_i2c_driver.h"
#include <string.h>

#define SLAVE_ADDR   0x68U

void I2C1_GPIOInits(void);
void I2C1_Inits(void);
void Button_Init(void);
void delay(void);

I2C_Handle_t I2C1Handle;
uint8_t some_data[] = "We are testing I2C master TX!!!";

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


int main(void)
{
	I2C1_GPIOInits();
	I2C1_Inits();
	Button_Init();

    while (1)    {
    	while (GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_NO_13) == GPIO_PIN_SET);
    	delay();
    	I2C_MasterSendData(&I2C1Handle, some_data, strlen((char*)some_data), SLAVE_ADDR);
    	while (GPIO_ReadFromInputPin(GPIOC, GPIO_PIN_NO_13) == GPIO_PIN_RESET);
    }
}
