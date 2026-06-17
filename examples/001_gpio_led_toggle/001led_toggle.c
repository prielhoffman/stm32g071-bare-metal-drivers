#include "stm32g071xx_gpio_driver.h"

/*
 * Example: GPIO LED Toggle
 *
 * Board: NUCLEO-G071RB
 * LED:   PA5 / LD2
 *
 * This example toggles the onboard LED using the custom GPIO driver.
 */

#define LED_GPIO_PORT      GPIOA
#define LED_PIN            GPIO_PIN_NO_5

static void delay(void){
    for (volatile uint32_t i = 0U; i < 500000U; i++);
}

int main(void){
    GPIO_Handle_t GpioLed;

    GpioLed.pGPIOx = LED_GPIO_PORT;
    GpioLed.GPIO_PinConfig.GPIO_PinNumber = LED_PIN;
    GpioLed.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    GpioLed.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_HIGH;
    GpioLed.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    GpioLed.GPIO_PinConfig.GPIO_PinAltFunMode = 0U;

    GPIO_Init(&GpioLed);

    while (1){
        GPIO_ToggleOutputPin(LED_GPIO_PORT, LED_PIN);
        delay();
    }

    return 0;
}
