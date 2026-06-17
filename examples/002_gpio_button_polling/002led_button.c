#include "stm32g071xx_gpio_driver.h"

/*
 * Example: GPIO Button Polling
 *
 * Board:  NUCLEO-G071RB
 * LED:    PA5 / LD2
 * Button: PC13 / B1
 *
 * This example reads the onboard user button using polling.
 * Each button press toggles the onboard LED.
 */

#define LED_GPIO_PORT          GPIOA
#define LED_PIN                GPIO_PIN_NO_5

#define BUTTON_GPIO_PORT       GPIOC
#define BUTTON_PIN             GPIO_PIN_NO_13
#define BUTTON_PRESSED         0U

static void delay(void){
    for (volatile uint32_t i = 0U; i < 250000U; i++);
}

int main(void){
    GPIO_Handle_t GpioLed;
    GPIO_Handle_t GpioButton;

    /* Configure PA5 as output for the onboard LED */
    GpioLed.pGPIOx = LED_GPIO_PORT;
    GpioLed.GPIO_PinConfig.GPIO_PinNumber = LED_PIN;
    GpioLed.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    GpioLed.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_HIGH;
    GpioLed.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    GpioLed.GPIO_PinConfig.GPIO_PinAltFunMode = 0U;

    GPIO_Init(&GpioLed);

    /* Configure PC13 as input for the onboard user button */
    GpioButton.pGPIOx = BUTTON_GPIO_PORT;
    GpioButton.GPIO_PinConfig.GPIO_PinNumber = BUTTON_PIN;
    GpioButton.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
    GpioButton.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_HIGH;
    GpioButton.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    GpioButton.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    GpioButton.GPIO_PinConfig.GPIO_PinAltFunMode = 0U;

    GPIO_Init(&GpioButton);

    while (1){
        if (GPIO_ReadFromInputPin(BUTTON_GPIO_PORT, BUTTON_PIN) == BUTTON_PRESSED){
            delay();

            GPIO_ToggleOutputPin(LED_GPIO_PORT, LED_PIN);

            /*
             * Wait until the button is released.
             * This prevents multiple toggles from one long press.
             */
            while (GPIO_ReadFromInputPin(BUTTON_GPIO_PORT, BUTTON_PIN) == BUTTON_PRESSED);
        }
    }

    return 0;
}
