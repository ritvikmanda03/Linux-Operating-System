#include "rtc.h"

int rtcFlag = 0; //interrupt flag

/* 
 *   rtc_init
 *   DESCRIPTION: Initializes the rtc
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void rtc_init(){
    enable_irq(2);

    outb(0x8B, 0x70);		// select register B, and disable NMI
    char prev = inb(0x71);	// read the current value of register B
    outb(0x8B,0x70);		// set the index again (a read will reset the index to register D)
    outb(prev | 0x40, 0x71);	// write the previous value ORed with 0x40. This turns on bit 6 of register B
    
    //initialize to 1024hz
    outb(0x8A, 0x70);     
    prev = inb(0x71);
    outb(0x8A, 0x70);     
    outb((prev & 0xF0) | 0x06, 0x71); // max (1024)
    

    enable_irq(8);
}


/* 
 *   rtc_handler
 *   DESCRIPTION: Throws away contents in register and runs a test to display the rtc cycles visually
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void rtc_handler(){
    // if(!type_flag){
    //     test_interrupts();
    // }
    // printf("hello this is working");
    outb(0x0C, 0x70);	// select register C
    inb(0x71);		// just throw away contents
    rtcFlag = 1;
    send_eoi(8);
}

/* 
 *   rtc_open
 *   DESCRIPTION: Sets the value of the rtc frequency to the lowest value (2 hz)
 *   INPUTS: filename -- name of file
 *   OUTPUTS: none
 *   Return: 0
 */

int32_t rtc_open (const uint8_t* filename) {
    char prev;
    char hz_freq = 0x0F;
    //set frequency rate to 2 hz
    outb(0x8A, 0x70);     
    prev = inb(0x71);
    outb(0x8A, 0x70);     
    outb((prev & 0xF0) | hz_freq, 0x71);
    return 0;
}

int32_t rtc_close(int32_t fd) {
    return 0;
}

/* 
 *   rtc_read
 *   DESCRIPTION: Blocks until an interrupt is seen
 *   INPUTS: fd-- file descriptor index
 *          buf-- unused
 *          nbytes-- unused
 *   OUTPUTS: none
 *   Return: 0
 */

int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes) {
    //block until an interrupt happens
	rtcFlag = 0;
    while(!rtcFlag) {}
    return 0;
}

/* 
 *   rtc_write
 *   DESCRIPTION: Changes the rtc frequency
 *   INPUTS: fd-- file descriptor index
 *          buf-- buffer containing frequency that we would like to change our rtc to
 *          nbytes-- unused
 *   OUTPUTS: none
 *   Return: 0
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes) {
    
    int temp = *(int*)(buf); //Dongming help5
    rtc_change_frequency(temp);
    return 0;

}

/* 
 *   rtc_change_frequency
 *   DESCRIPTION: Changes the rtc frequency
 *   INPUTS: buf-- argument containing frequency that we would like to change our rtc to
 *   OUTPUTS: none
 *   Return: 0 if the frequency rate is a valid power of two. Otherwise it returns -1
 */
int32_t rtc_change_frequency(int buf){
    //log base 2 of frequency to find out which power of two frequency we are doing 
    int log = 0;
    while(buf != 1){
        buf /= 2;
        log++;
    }
    
    //* BUG : Shifting 0x8000 by buf-1 does not work. translated to use log funciton but still seems too fast in some cases
    // 32768 >> (rate - 1)
    char freq = (32768 - log) & 0x0F; //"Converts" our desired frequency to the char frequency rate 
    char prev;
    if(freq == 0x00){
        return -1;
    }else{
        //update the frequency rate
        cli();
        outb(0x8A, 0x70);     
        prev = inb(0x71);
        outb(0x8A, 0x70);     
        outb((prev & 0xF0) | freq, 0x71);
        sti();
        return 0;
    }
}
