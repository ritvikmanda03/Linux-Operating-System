/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {

    outb(ICW1, 0x20); // Initialize on PIC1 on Port 0x20 <- ICW1
     
    outb(ICW2_MASTER, 0x21); // Port 21 <- ICW2
     
    outb(ICW3_MASTER, 0x21); // Port 21 <- ICW3
     
    outb(ICW4, 0x21); // Port 21 <- ICW4
     
    master_mask = 0xFF;
    outb(master_mask, 0x21); // Disable all interrupts for PIC1
     

    outb(ICW1, 0xA0); // Initialize on PIC2 on Port 0xA0 <- ICW1
     
    outb(ICW2_SLAVE, 0xA1); // Port A1 <- ICW2
     
    outb(ICW3_SLAVE, 0xA1); // Port A1 <- ICW3
     
    outb(ICW4, 0xA1); // Port A1 <- ICW4
     

    slave_mask = 0xFF;
    outb(slave_mask, 0xA1); // Disable all interrupts for PIC2;


}

//THIS IS DOING PROTEXTED MODE WE NEED TO DO REAL MODE

//clear interrupt masks
void enable_irq(uint32_t irq_num) {
    uint16_t port;
    uint16_t value;
 
    if(irq_num < 8) {
        port = 0x21; // Port is set to Master data
    }else{
		irq_num -= 8; // Subtract 8 to get IRQ for slave pic
        port  = 0xA1; // Port is set to Slave data
	}
    value = inb(port) & ~(1 << irq_num); // AND the value to Clear the mask
    outb(value,port);

}

// mask interrupts
void disable_irq(uint32_t irq_num) {
    uint16_t port;
    uint16_t value;

    if(irq_num < 8) {
		port = 0x21; // Port is set to Master data
	}else{
		port = 0xA1; // Port is set to Slave data
		irq_num -= 8; // Subtract 8 to get IRQ for slave pic
	}
    value = inb(port) | (1 << irq_num); // OR the value to set the mask
    outb(value,port);
}

void send_eoi(uint32_t irq_num) {
    if(irq_num >= 8){
		outb(EOI | 2, MASTER_8259_PORT); //OR End of interrupt with Slave IRQ on Master PIC then send that to the corresponding port
        outb(EOI | (irq_num-8), SLAVE_8259_PORT); //OR End of interrupt with IRQ on slave pic then send it to the corresponding port
    }else{
        outb(EOI | irq_num, MASTER_8259_PORT); //No slave port is active so only send EOI to master pic
    }
}
