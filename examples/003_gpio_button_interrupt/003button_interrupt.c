#include "stm32g071xx_gpio_driver.h"

/*
 * Example: GPIO Button Interrupt
 *
 * Board:  NUCLEO-G071RB
 * LED:    PA8 / D7 external LED
 * Button: PA10 / D2 external button
 *
 * The button is configured with an internal pull-up.
 * A button press pulls PA10 low and generates a falling-edge interrupt.
 */

#define LED_GPIO_PORT          GPIOA
#define LED_PIN                GPIO_PIN_NO_8

#define BUTTON_GPIO_PORT       GPIOA
#define BUTTON_PIN             GPIO_PIN_NO_10
#define BUTTON_PRESSED         GPIO_PIN_RESET

static volatile uint8_t buttonPressed = 0U;

static void delay(void){
    for (volatile uint32_t i = 0U; i < 250000U; i++);
}

void EXTI4_15_IRQHandler(void){
    GPIO_IRQHandling(BUTTON_PIN);

    /* The actual button handling is done in the main loop */
    buttonPressed = 1U;
}

int main(void){
    GPIO_Handle_t GpioLed = {0};
    GPIO_Handle_t GpioButton = {0};

    /*Configure PA8 as output for the external LED */
    GpioLed.pGPIOx = LED_GPIO_PORT;
    GpioLed.GPIO_PinConfig.GPIO_PinNumber = LED_PIN;
    GpioLed.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    GpioLed.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_HIGH;
    GpioLed.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    GpioLed.GPIO_PinConfig.GPIO_PinAltFunMode = 0U;

    /*
     * Configure PA10 as external interrupt input.
     * The internal pull-up keeps the pin high when the button is not pressed.
     */
    GpioButton.pGPIOx = BUTTON_GPIO_PORT;
    GpioButton.GPIO_PinConfig.GPIO_PinNumber = BUTTON_PIN;
    GpioButton.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IT_FT;
    GpioButton.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_HIGH;
    GpioButton.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
    GpioButton.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PU;
    GpioButton.GPIO_PinConfig.GPIO_PinAltFunMode = 0U;

    GPIO_Init(&GpioLed);
    GPIO_Init(&GpioButton);

    GPIO_IRQPriorityConfig(IRQ_NO_EXTI4_15, 2U);
    GPIO_IRQConfig(IRQ_NO_EXTI4_15, 0U, ENABLE);

    while (1){
        if (buttonPressed == 1U){
            buttonPressed = 0U;

            /* Simple debounce delay */
            delay();

            if (GPIO_ReadFromInputPin(BUTTON_GPIO_PORT, BUTTON_PIN) == BUTTON_PRESSED){
                GPIO_ToggleOutputPin(LED_GPIO_PORT, LED_PIN);

                /* Wait for button release to avoid repeated toggles */
                while (GPIO_ReadFromInputPin(BUTTON_GPIO_PORT, BUTTON_PIN) == BUTTON_PRESSED);
            }
        }
    }

    return 0;
}
