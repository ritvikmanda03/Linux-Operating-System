#include "keyboard.h"
#include "system_calls.h"

#define buffEnd 127
#define buffSize 128

//flags for holding down special keys
int capsFlag = 0;
int lshiftFlag = 0;
int rshiftFlag = 0;
int altFlag = 0;
int ctrlFlag = 0;

static char line_buf[buffSize];

/* buffer pointer array for typed in text for each terminal  */
int bufPtr_arr[3] = {0, 0, 0};
/* keep tracks of the buffer for each terminal*/
static char buf_arr[3][buffSize];

/* keeps track of x and y position for each terminal */
int x_arr[3] = {0,0,0};
int y_arr[3] = {0,0,0};

int bufPtr = 0;
int readPtr = 0;
int enterFlag = 0;
int terminal = 1;

/* terminal addresses for vidmem to be mapped to based on switch  */
void* VIDEO_PAGE_ADDRESS[4] = { 0, (void *)0xBA000, (void *)0xBC000, (void *)0xBE000 }; // 0 for indexing

/* keeps track of process index that is current for each terminal */
int top_terminal_pid[4] = { -1, -1, -1, -1 };

int get_terminal(int t){
    return (int)VIDEO_PAGE_ADDRESS[t];
}
/* 
 *   kb_init
 *   DESCRIPTION: Initializes the keyboard
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */

void kb_init(){
    //keyboard connected to corresponding irq
    enable_irq(1);

}

/* 
 *   keyboard_handler
 *   DESCRIPTION: Reads value from the keyboard and prints it to the screen to type
 *   INPUTS: none
 *   OUTPUTS: none
 *   Return: none
 */
void keyboard_handler(){
    //mask all interrupts    
    cli();
    //kernel_paging();
    // initialize color
    colorFlag = 1;
    int scancode = inb(0x60); //read from keyboard port
    char scan;
    enterFlag = 0; //flag to see if enter has been pressed or not

    //If the key we press is a special character then we raise a flag to use later
    switch(scancode){
        case caps_clicked:
            if(capsFlag){
                capsFlag = 0;
            }else{
                capsFlag = 1;
            }
            break;
        case left_shift_held:
            lshiftFlag = 1;
            break;
        case left_shift_released:
            lshiftFlag = 0;
            break;
        case alt_held:
            altFlag = 1;
            break;
        case alt_released:
            altFlag = 0;
            break;
        case right_shift_held:
            rshiftFlag = 1;
            break;
        case right_shift_released:
            rshiftFlag = 0;
            break;
        case ctrl_held:
            ctrlFlag = 1;
            break;
        case ctrl_released:
            ctrlFlag = 0;
            break;
        default:
            break;
    }

    if((scancode < 58) && scancode != 1 && scancode != 0x3A){
        //Shift keys
        if(rshiftFlag | lshiftFlag){
            if(capsFlag){
                //Shift and Caps lock pressed
                scan = keyMapShift2[scancode];
            }else{
                //Only shift pressed
                scan = keyMapShift[scancode];
            }
        }else if(capsFlag){
            //Only caps lock pressed
            scan = keyMapCaps[scancode];
        }else{
            scan = keyMap[scancode]; //see what character we are printing and then
        }
        //We do not want to print the special keys so we move on
        if((scan == F1) || (scan == F2) || (scan == F3) || (scancode == caps_clicked) || (scancode == alt_held) || (scancode == left_shift_held) || (scancode == right_shift_held) || (scancode == ctrl_held)){
            colorFlag = 0;
            send_eoi(1);
            sti();
            return;
        }

        //backspace handling in buffer (clear curr character and move ptr back)
        if((scan == '\b') && (bufPtr > 0)){
            line_buf[bufPtr] = ' ';
            bufPtr--;
        } 
        
        //save last spot for enter key. Only add to buffer if it isn't the last spot
        if((bufPtr < buffEnd) && (scan != '\b') && (scan != '\t')){
            line_buf[bufPtr] = scan;
            if(scan == '\n'){
                enterFlag = 1;
                //program_paging(top_terminal_pid[terminal], terminal);
            }else{
                bufPtr++;
            }
        }
        //if enter key then we have to save the line buffer to the user buffer in terminal_read
        if((scan == '\n') && (bufPtr == buffEnd)){
            line_buf[bufPtr] = scan;
            enterFlag = 1;
            //program_paging(top_terminal_pid[terminal], terminal);
        }
        //special case for tab 
        if(scan != '\t'){
            putc(scan);//print to the screen 
        }else{
            //only add part of tab to the line buffer if it is near the end so we do not overflow
            int tab;
            int count;
            switch(bufPtr){
                //only one space left in buffer to fill
                case 126:
                    count = 1;
                    break;
                //only two space left in buffer to fill
                case 125:
                    count = 2;
                    break;
                //only three space left in buffer to fill
                case 124:
                    count = 3;
                    break;
                //only more than 3 spaces left in buffer to fill
                default:
                    count = 4;
                    break;
            }
            if((count >= 1) && (count <=4)){
                for(tab = 0; tab < count; tab++){
                    line_buf[bufPtr] = ' ';
                    bufPtr++;
                }
            }
            //always print tab on the screen as four spaces
            putc(' ');
            putc(' ');
            putc(' ');
            putc(' ');
        }
    }

    //Ctrl + l or Ctrl + L clears the screen and the line_buffer
    if(ctrlFlag && ((keyMap[scancode] == 'l') || (keyMapShift[scancode] == 'L') || (keyMapCaps[scancode] == 'L') ) ){
        clearFlag = 1;
        clear();
        int clear;
        for(clear = 0; clear < bufPtr; clear++){
            line_buf[bufPtr] = ' ';
        }
        bufPtr = 0;
    }
    // checks to see which terminal to switch to
    if(altFlag && (scancode == F1)){
        //putc('1');
        if(terminal != 1){
            // save the previous terminal
            int prev_terminal = terminal;
            setTerminal(1);
            {
                // store the process index in the current process pid
                pcb_t* curr_pcb;
                curr_pcb = (pcb_t *) (END_KERNEL - ((top_terminal_pid[prev_terminal] + 1) * (PCB_SIZE)));
                // save ebp
                asm __volatile__(
                    "movl %%ebp, %0;"
                    : "=r"(curr_pcb->old_ebp)
                );    
            }
            //program_paging(top_terminal_pid[terminal], terminal);
            if (top_terminal_pid[terminal] == -1) {
                send_eoi(1);
                sti();
                // execute shell again if the base shell
                execute((const uint8_t*)"shell");
            } else {
                send_eoi(1);
                sti();
                // restore execute to handle process index 
                restoreExec(top_terminal_pid[terminal]);
            }
        }
        //program_paging(top_terminal_pid[terminal], terminal);
    }
    if(altFlag && (scancode == F2)){
        // putc('2');
        if(terminal != 2){
            // save the previous terminal
            int prev_terminal = terminal;
            setTerminal(2);
            {
                // store the process index in the current process pid
                pcb_t* curr_pcb;
                curr_pcb = (pcb_t *) (END_KERNEL - ((top_terminal_pid[prev_terminal] + 1) * (PCB_SIZE)));
                // save ebp
                asm __volatile__(
                    "movl %%ebp, %0;"
                    : "=r"(curr_pcb->old_ebp)
                );    
            }
            //program_paging(top_terminal_pid[terminal], terminal);
            if (top_terminal_pid[terminal] == -1) {
                send_eoi(1);
                sti();
                // execute shell again if the base shell
                execute((const uint8_t*)"shell");
            } else {
                send_eoi(1);
                sti();
                // restore execute to handle process index 
                restoreExec(top_terminal_pid[terminal]);
            }
        }
    }
    if(altFlag && (scancode == F3)){
        //putc('3');
        if(terminal != 3){
            // save the previous terminal
            int prev_terminal = terminal;
            setTerminal(3);
            {
                // store the process index in the current process pid
                pcb_t* curr_pcb;
                curr_pcb = (pcb_t *) (END_KERNEL - ((top_terminal_pid[prev_terminal] + 1) * (PCB_SIZE)));
                // save ebp
                asm __volatile__(
                    "movl %%ebp, %0;"
                    : "=r"(curr_pcb->old_ebp)
                );    
            }
            //program_paging(top_terminal_pid[terminal], terminal);
            if (top_terminal_pid[terminal] == -1) {
                send_eoi(1);
                sti();
                // execute shell again if the base shell
                execute((const uint8_t*)"shell");
            } else {
                send_eoi(1);
                sti();
                // restore execute to handle process index 
                restoreExec(top_terminal_pid[terminal]);
            }
        }
        //program_paging(top_terminal_pid[terminal], terminal);
    }
    colorFlag = 0;
    send_eoi(1);
    sti();
}

/* 
 *   setTerminal()
 *   DESCRIPTION: changes the video memory mapping and restores the previous terminal attributes so that the
 *   terminals can be switched upon key press. Makes sure to keep track of the old attributes by putting them
 *   in the previosuly declared arrays
 *   INPUTS: int next_terminal
 *   OUTPUTS: none
 *   SIDE EFFECTS: changes the video memory mapping and restores the previous terminal attributes
 */
void setTerminal(int next_terminal) {
    if (terminal == next_terminal) return;

    /* copy video memory to terminal to save the current page */
    memcpy(VIDEO_PAGE_ADDRESS[terminal], (void *)VIDEO_MEM_ADDRESS, 4096);

    /* keep the buffer stored in the array so we dont lose track of it */
    strncpy(buf_arr[terminal-1], line_buf, bufPtr + 1);
    bufPtr_arr[terminal-1] = bufPtr;
    /* keep track of the current x and y positions of the cursor*/
    x_arr[terminal-1] = screen_x;
    y_arr[terminal-1] = screen_y;
    /* actually change the next terminal*/
    terminal = next_terminal;

    /* update the cursor */
    screen_x = x_arr[terminal-1];
    screen_y = y_arr[terminal-1];
    update_cursor();
    /* restore buffer  */
    bufPtr = bufPtr_arr[terminal-1];
    /* copy the buffer array back to the actual buffer */
    strncpy(line_buf, buf_arr[terminal-1], bufPtr + 1);

     /* copy the terminal address to video memory */
    memcpy((void *)VIDEO_MEM_ADDRESS, VIDEO_PAGE_ADDRESS[terminal], 4096);

    // program_paging(top_terminal_pid[terminal], terminal);
}


/* 
 *   terminal_open
 *   DESCRIPTION: Does not do anything
 *   INPUTS: filename -- name of file
 *   OUTPUTS: none
 *   Return: 0
 */
int32_t terminal_open(const uint8_t* filename){
    return 0;
}

/* 
 *   terminal_close
 *   DESCRIPTION: Does not do anything
 *   INPUTS: filename -- name of file
 *   OUTPUTS: none
 *   Return: 0
 */
int32_t terminal_close(int32_t fd){
    return 0;
}

/* 
 *   terminal_read
 *   DESCRIPTION: Blocks until the enter key is pressed. Then copies the line buffer into the argument buffer
 *   INPUTS: fd-- file descriptor index
 *          buf-- buffer to copy line buffer into
 *          nbytes-- number of bytes to copy 
 *   OUTPUTS: none
 *   Return: number of bytes
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    // If enter key is pressed then copy everything in line buffer to user buffer
    int retval;

    while(!enterFlag){}
    enterFlag = 0; //reset flag
    cli();
    strncpy((char*)buf, line_buf, bufPtr + 1);    //copy to buffer
    retval = bufPtr + 1;
    int clear;
    for(clear = 0; clear < bufPtr; clear++){
        line_buf[clear] = '\0';
    }
    bufPtr = 0; //reset ptr for next buffer
    sti();
    return retval;
}

/* 
 *   terminal_write
 *   DESCRIPTION: Writes an entire input buffer to the screen
 *   INPUTS: fd-- file descriptor index
 *          buf-- buffer print to the screen
 *          nbytes-- number of bytes to copy 
 *   OUTPUTS: none
 *   Return: number of bytes
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    int a;
    char scan;

    if(buf == NULL){
        return -1;
    }

    //iterate through the bytes we want to print and print them to the screen
    for(a = 0; a < nbytes; a++) {
        //Prints the given characters to the screen.
        if(((char*)buf)[a] == '\002'){
            break;
        }
        scan = ((char*)buf)[a];
        if(scan != '\0'){
            putc(scan);
        }
        //newline indicates the end of the buffer so we break
        
        
        /*BUG CP4*/
        // if(scan == '\n'){
        //     break;
        // }
    }
    return nbytes;
}   
