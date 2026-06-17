#include "stm32g071xx.h"
#include "stm32g071xx_gpio_driver.h"
#include "stm32g071xx_i2c_driver.h"
#include <string.h>

#define MY_ADDR        0x68U

#define CMD_SEND_LEN   0x51U
#define CMD_SEND_MSG   0x52U

I2C_Handle_t I2C1Handle;

uint8_t tx_buf[] = "Hello from G071";

volatile uint8_t commandCode = 0xFF;

volatile uint8_t dbgLastAppEv = 0;
volatile uint8_t dbgRxByte = 0;
volatile uint8_t dbgTxByte = 0;
volatile uint32_t dbgDataRcvCount = 0;
volatile uint32_t dbgDataReqCount = 0;
volatile uint32_t dbgAfCount = 0;

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
	I2C1Handle.I2CConfig.I2C_DeviceAddress = MY_ADDR;
	I2C1Handle.I2CConfig.I2C_ACKControl = I2C_ACK_ENABLE;
	I2C1Handle.I2CConfig.I2C_FMDutyCycle = I2C_FM_DUTY_2;

	I2C_Init(&I2C1Handle);
}


int main(void)
{
    I2C1_GPIOInits();
    I2C1_Inits();

    /*
     * Enable I2C1 interrupt in the CPU interrupt controller.
     */
    I2C_IRQConfig(IRQ_NO_I2C1, 0, ENABLE);

    /*
     * Enable I2C slave interrupt sources inside the I2C peripheral.
     */
    I2C_SlaveEnableDisableCallbackEvents(I2C1, ENABLE);

    while (1);
}


void I2C1_IRQHandler(void){
	I2C_IRQHandling(&I2C1Handle);
}

void I2C_ApplicationEventCallback(I2C_Handle_t *pI2CHandle, uint8_t AppEv){
    static uint8_t cnt = 0;

    dbgLastAppEv = AppEv;

    if (AppEv == I2C_EV_DATA_RCV){
        dbgDataRcvCount++;

        dbgRxByte = I2C_SlaveReceiveData(pI2CHandle->pI2Cx);
        commandCode = dbgRxByte;
    }
    else if (AppEv == I2C_EV_DATA_REQ){
        dbgDataReqCount++;

        if (commandCode == CMD_SEND_LEN){
            dbgTxByte = strlen((char *)tx_buf);
            I2C_SlaveSendData(pI2CHandle->pI2Cx, dbgTxByte);
        }
        else if (commandCode == CMD_SEND_MSG){
            dbgTxByte = tx_buf[cnt];
            I2C_SlaveSendData(pI2CHandle->pI2Cx, dbgTxByte);
            cnt++;
        }
    }
    else if (AppEv == I2C_ERROR_AF){
        dbgAfCount++;

        commandCode = 0xFF;
        cnt = 0;
    }
}
