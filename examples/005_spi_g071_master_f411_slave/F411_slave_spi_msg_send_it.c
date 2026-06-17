#include <stdint.h>

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

#define GPIOA_OTYPER   (*(volatile uint32_t *)(GPIOA_BASE + 0x04))
#define GPIOA_OSPEEDR  (*(volatile uint32_t *)(GPIOA_BASE + 0x08))
#define GPIOA_PUPDR    (*(volatile uint32_t *)(GPIOA_BASE + 0x0C))
#define GPIOA_ODR      (*(volatile uint32_t *)(GPIOA_BASE + 0x14))

typedef enum{
    SLAVE_IDLE,
    SLAVE_PREPARE_MESSAGE,
    SLAVE_SIGNAL_DATA_READY,
    SLAVE_SEND_BYTE,
    SLAVE_DONE
} SlaveState_t;

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

void DataReady_GPIOInit(void){
    /*
     * Enable GPIOA peripheral clock.
     * PA0 will be used as Data Ready output.
     */
    RCC_AHB1ENR |= (1U << 0);

    /*
     * Configure PA0 as output.
     * MODER0[1:0] = 01
     */
    GPIOA_MODER &= ~(3U << (0 * 2));
    GPIOA_MODER |=  (1U << (0 * 2));

    /* Output type: push-pull */
    GPIOA_OTYPER &= ~(1U << 0);

    /* Speed: low speed */
    GPIOA_OSPEEDR &= ~(3U << (0 * 2));

    /* No pull-up / pull-down on output */
    GPIOA_PUPDR &= ~(3U << (0 * 2));

    /* Idle state is HIGH */
    GPIOA_ODR |= (1U << 0);
}

void DataReady_NotifyMaster(void){
    /*
     * Create falling edge:
     * HIGH -> LOW.
     */
    GPIOA_ODR &= ~(1U << 0);

    for (volatile uint32_t i = 0; i < 10000; i++);

    /* Return line to idle HIGH */
    GPIOA_ODR |= (1U << 0);
}

int main(void){
	SlaveState_t slaveState = SLAVE_IDLE;
	uint8_t msg[] = "Hello from F411";
	uint32_t msg_idx = 0;
	uint8_t messageSent = 0;

	SPI2_Slave_GPIOInits();
	SPI2_Inits();
	DataReady_GPIOInit();

	SPI2_CR1 |= (1U << 6);

	while(1){
		switch(slaveState){
		case SLAVE_IDLE:
		    /*
		     * For now, send the message only once.
		     * Later this can be replaced by a real event.
		     */
		    if (messageSent == 0){
		        slaveState = SLAVE_PREPARE_MESSAGE;
		    }
		    break;

		case SLAVE_PREPARE_MESSAGE:

			slaveState = SLAVE_SIGNAL_DATA_READY;
			break;
		case SLAVE_SIGNAL_DATA_READY:

			slaveState = SLAVE_SEND_BYTE;
			break;
		case SLAVE_SEND_BYTE:
		{
		    /*
		     * Save the current byte before sending it.
		     * We need this later to check if it was '\0'.
		     */
		    uint8_t currentByte = msg[msg_idx];

		    /*
		     * Wait until the SPI data register is empty
		     * and ready for the next byte.
		     */
		    while (!(SPI2_SR & SPI_SR_TXE));

		    /*
		     * Load one byte into SPI data register.
		     * The byte will actually go out only when the Master provides clock.
		     */
		    SPI2_DR = currentByte;

		    /*
		     * Wait until a dummy byte arrives from the Master.
		     * Reading DR clears RXNE.
		     */
		    while (!(SPI2_SR & SPI_SR_RXNE));

		    /*
		     * Read and ignore the dummy byte received from the Master.
		     */
		    volatile uint8_t dummyRead = SPI2_DR;
		    (void)dummyRead;

		    /*
		     * If we sent '\0', the full message is done.
		     * Otherwise, move to the next byte and stay in SEND_BYTE.
		     */
		    if (currentByte == '\0'){
		        slaveState = SLAVE_DONE;
		    }
		    else{
		        msg_idx++;
		        slaveState = SLAVE_SEND_BYTE;
		    }

		    break;
		}

		case SLAVE_DONE:
		    /*
		     * Message finished.
		     * Reset index and prevent sending again automatically.
		     */
		    msg_idx = 0;
		    messageSent = 1;

		    slaveState = SLAVE_IDLE;
		    break;

        default:
            slaveState = SLAVE_IDLE;
            break;
		}
	}
	return 0;
}
