#include <stdint.h>

/* SPI command codes received from the G071 master */
#define COMMAND_LED_CTRL       0x50
#define COMMAND_SENSOR_READ    0x51
#define COMMAND_LED_READ       0x52
#define COMMAND_PRINT          0x53
#define COMMAND_ID_READ        0x54

#define RCC_BASE      0x40023800U
#define GPIOA_BASE    0x40020000U
#define GPIOB_BASE    0x40020400U
#define SPI2_BASE     0x40003800U

#define RCC_AHB1ENR   (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define RCC_APB1ENR   (*(volatile uint32_t *)(RCC_BASE + 0x40))

#define GPIOA_MODER   (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_BSRR    (*(volatile uint32_t *)(GPIOA_BASE + 0x18))

#define GPIOB_MODER   (*(volatile uint32_t *)(GPIOB_BASE + 0x00))
#define GPIOB_AFRH    (*(volatile uint32_t *)(GPIOB_BASE + 0x24))

#define SPI2_CR1      (*(volatile uint32_t *)(SPI2_BASE + 0x00))
#define SPI2_CR2      (*(volatile uint32_t *)(SPI2_BASE + 0x04))
#define SPI2_SR       (*(volatile uint32_t *)(SPI2_BASE + 0x08))
#define SPI2_DR       (*(volatile uint32_t *)(SPI2_BASE + 0x0C))

#define SPI_SR_RXNE   (1 << 0)
#define SPI_SR_TXE    (1 << 1)

void SPI2_Slave_GPIOInits(void){
	RCC_AHB1ENR |= (1 << 0); // Enable GPIOA clock
	RCC_AHB1ENR |= (1 << 1); // Enable GPIOB clock

	// PB12, PB13, PB14, PB15 to Alternate Function (10)
	GPIOB_MODER &= ~((3 << (12 * 2)) | (3 << (13 * 2)) | (3 << (14 * 2)) | (3 << (15 * 2)));
	GPIOB_MODER |=  ((2 << (12 * 2)) | (2 << (13 * 2)) | (2 << (14 * 2)) | (2 << (15 * 2)));

	// Set AF5 (SPI2) for PB12, PB13, PB14, PB15
	GPIOB_AFRH &= ~((0xF << (4 * 4)) | (0xF << (5 * 4)) | (0xF << (6 * 4)) | (0xF << (7 * 4)));
	GPIOB_AFRH |=  ((5 << (4 * 4)) | (5 << (5 * 4)) | (5 << (6 * 4)) | (5 << (7 * 4)));
}

void SPI2_Inits(void){
	RCC_APB1ENR |= (1 << 14); // Enable SPI2 clock
	SPI2_CR1 = 0;             // Slave mode, CPOL=0, CPHA=0, 8-bit, SSM=0 (Hardware NSS)
	SPI2_CR2 = 0;             // Ensure SSOE = 0 for Hardware NSS Input
}

void Led_Init(void){
	GPIOA_MODER &= ~(3 << (5 * 2));
	GPIOA_MODER |=  (1 << (5 * 2)); // PA5 to Output
}

int main(void){
	uint8_t command_code = 0;
	uint8_t ack_byte = 0xF5;
	uint8_t rx_args[2];
	uint8_t analog_pin = 0;
	uint8_t sensor_mock_value = 170;
	uint8_t led_state = 0;
	uint8_t rx_buffer[32];
	uint8_t board_id[] = "F411-Slave";

	SPI2_Slave_GPIOInits();
	SPI2_Inits();
	Led_Init();

	// Enable SPI2
	SPI2_CR1 |= (1 << 6);

	while(1){

		while (!(SPI2_SR & SPI_SR_RXNE));
		command_code = (uint8_t)SPI2_DR;

		if (command_code == 0x50){
		    /*
		     * Load ACK before master sends dummy byte.
		     */
		    while (!(SPI2_SR & SPI_SR_TXE));
		    SPI2_DR = ack_byte;

		    /*
		     * Master sends dummy byte and reads ACK.
		     * Read DR to clear RXNE caused by that dummy byte.
		     */
		    while (!(SPI2_SR & SPI_SR_RXNE));
		    uint8_t dummy_flush = (uint8_t)SPI2_DR;

		    /*
		     * Receive argument 1: LED pin.
		     */
		    while (!(SPI2_SR & SPI_SR_RXNE));
		    rx_args[0] = (uint8_t)SPI2_DR;

		    /*
		     * Receive argument 2: LED value.
		     */
		    while (!(SPI2_SR & SPI_SR_RXNE));
		    rx_args[1] = (uint8_t)SPI2_DR;

		    if (rx_args[0] == 5 && rx_args[1] == 1){
		        GPIOA_BSRR = (1 << 5);
		        led_state = 1;
		    }
		    else if (rx_args[0] == 5 && rx_args[1] == 0){
		        GPIOA_BSRR = (1 << (5 + 16));
		        led_state = 0;
		    }
		}

		else if (command_code == 0x51){
		    while (!(SPI2_SR & SPI_SR_TXE));
		    SPI2_DR = ack_byte;

		    while (!(SPI2_SR & SPI_SR_RXNE));
		    uint8_t dummy_flush = (uint8_t)SPI2_DR;

		    while (!(SPI2_SR & SPI_SR_RXNE));
		    analog_pin = (uint8_t)SPI2_DR;

		    while (!(SPI2_SR & SPI_SR_TXE));
		    SPI2_DR = sensor_mock_value;

		    while (!(SPI2_SR & SPI_SR_RXNE));
		    uint8_t dummy_read_last = (uint8_t)SPI2_DR;
		}
		else if(command_code == COMMAND_LED_READ){
		    /*
		     * Load ACK before master sends dummy byte.
		     */
		    while (!(SPI2_SR & SPI_SR_TXE));
		    SPI2_DR = ack_byte;

		    /*
		     * Master sends dummy byte and reads ACK.
		     * Read DR to clear RXNE caused by that dummy byte.
		     */
		    while (!(SPI2_SR & SPI_SR_RXNE));
		    uint8_t dummy_flush = (uint8_t)SPI2_DR;

		    /*
		     * Load LED status before master sends the next dummy byte.
		     * When the master sends that dummy byte, it will receive led_state.
		     */
		    while (!(SPI2_SR & SPI_SR_TXE));
		    SPI2_DR = led_state;

		    /*
		     * Master sends dummy byte to read led_state.
		     * Read DR to clear RXNE caused by that dummy byte.
		     */
		    while (!(SPI2_SR & SPI_SR_RXNE));
		    uint8_t dummy_read_last = (uint8_t)SPI2_DR;
		}
		else if (command_code == COMMAND_PRINT){
		    while (!(SPI2_SR & SPI_SR_TXE));
		    SPI2_DR = ack_byte;

		    while (!(SPI2_SR & SPI_SR_RXNE));
		    uint8_t dummy_flush = (uint8_t)SPI2_DR;

		    /*
		     * Receive string from master.
		     * The master sends sizeof(message), including the '\0'.
		     * Here we read up to 32 bytes or until '\0' is received.
		     */
		    for (uint32_t i = 0; i < sizeof(rx_buffer); i++){
		        while (!(SPI2_SR & SPI_SR_RXNE));
		        rx_buffer[i] = (uint8_t)SPI2_DR;

		        if (rx_buffer[i] == '\0'){
		            break;
		        }
		    }
		}
		else if (command_code == COMMAND_ID_READ){
		    while (!(SPI2_SR & SPI_SR_TXE));
		    SPI2_DR = ack_byte;

		    while (!(SPI2_SR & SPI_SR_RXNE));
		    uint8_t dummy_flush = (uint8_t)SPI2_DR;

		    for (uint32_t i = 0; i < 10; i++){
		    	while (!(SPI2_SR & SPI_SR_TXE));
		    	SPI2_DR = board_id[i];

		        /*
		         * Master sends dummy byte to generate clock.
		         * Read it to clear RXNE.
		         */
			    while (!(SPI2_SR & SPI_SR_RXNE));
			    uint8_t dummy_read = (uint8_t)SPI2_DR;
		    }
		}
	}
	return 0;
}
