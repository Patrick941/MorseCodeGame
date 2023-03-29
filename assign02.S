#include "hardware/regs/addressmap.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/timer.h"
#include "hardware/regs/m0plus.h"

.syntax unified
.cpu    cortex-m0plus
.thumb
.global main_asm
.align  4

.equ    DFLT_STATE_STRT, 1            @ Specify the value to start flashing
.equ    DFLT_STATE_STOP, 0            @ Specify the value to stop flashing
.equ    DFLT_ALARM_TIME, 1000000      @ Specify the default alarm timeout

.equ    GPIO_BTN_DN_MSK, 0x00040000   @ Bit-18 for falling-edge event on GP20
.equ    GPIO_BTN_EN_MSK, 0x00400000   @ Bit-22 for falling-edge event on GP21
.equ    GPIO_BTN_UP_MSK, 0x04000000   @ Bit-26 for falling-edge event on GP22

.equ    GPIO_BTN_DN,  20              @ Specify pin for the "down" button
.equ    GPIO_BTN_EN,  21              @ Specify pin for the "enter" button
.equ    GPIO_BTN_UP,  22              @ Specify pin for the "up" button
.equ    GPIO_LED_PIN, 25              @ Specify pin for the built-in LED
.equ    GPIO_DIR_IN,   0              @ Specify input direction for a GPIO pin
.equ    GPIO_DIR_OUT,  1              @ Specify output direction for a GPIO pin

.equ    LED_VAL_ON,    1              @ Specify value that turns the LED "on"
.equ    LED_VAL_OFF,   0              @ Specify value that turns the LED "off"
.equ    BUT_VAL_ON,    0
.equ    BUT_VAL_OFF,   1

.equ    GPIO_ISR_OFFSET, 0x74         @ GPIO is int #13 (vector table entry 29)
.equ    ALRM_ISR_OFFSET, 0x40         @ ALARM0 is int #0 (vector table entry 16)

.equ    SLEEP_TIME, 500
.equ    FLASHING_STATE, 0x002FFFC0

@ Entry point to the ASM portion of the program
main_asm:
        movs    r0, #GPIO_LED_PIN                       @ Load the LED pin into r0 to use as first parameter
        bl      asm_gpio_init                           @ Call asm_gpio_init to initialise this pin
        movs    r0, #GPIO_LED_PIN                       @ Load the LED pin into r0 to use as first parameter
        movs    r1, #GPIO_DIR_OUT                       @ Load the output value into r1 to use as second parameter
        bl      asm_gpio_set_dir                        @ Set the LED pin to be an output

        movs    r0, #GPIO_BTN_EN                        @ Load the Enter button pin into r0 to use as first parameter       
        bl      asm_gpio_init                           @ Call asm_gpio_init to intialise this pin
        movs    r0, #GPIO_BTN_EN                        @ Load the Enter button pin into r0 to use as first parameter
        movs    r1, GPIO_DIR_IN                         @ Load the input value into r1 to use as second parameter
        bl      asm_gpio_set_dir                        @ Set the Enter button pin to be an input
        movs    r0, #GPIO_BTN_UP                        @ Load the Up button pin into r0 to use as first parameter
        bl      asm_gpio_init                           @ Call asm_gpio_init to initialise this pin
        movs    r0, #GPIO_BTN_UP                        @ Load the Up button pin into r0 to use as first parameter
        movs    r1, GPIO_DIR_IN                         @ Load the input value into r1 to use as second parameter
        bl      asm_gpio_set_dir                        @ Set the Up button pin to be an input
        movs    r0, #GPIO_BTN_DN                        @ Load the Down button pin into r0 to use as first parameter
        bl      asm_gpio_init                           @ Call asm_gpio_init to initialise this pin
        movs    r0, #GPIO_BTN_DN                        @ Load the Down button pin into r0 to use as first parameter
        movs    r1, GPIO_DIR_IN                         @ Load the input value into r1 to use as second parameter
        bl      asm_gpio_set_dir                        @ Set the Down button pin to be an input
   
        ldr     r5, =PPB_BASE                           @ Load the base for all the input and output pins
        ldr     r2, =M0PLUS_VTOR_OFFSET                 @ Load the offset to get address for the vector table
        add     r5, r5, r2                              @ Save the result in r5 for future use
        ldr     r2, =PPB_BASE                           @ Load the base for all the input and output pins
        ldr     r6, =M0PLUS_NVIC_ISER_OFFSET            @ Load the offset to get address for the alarm / gpio
        add     r6, r6, r2                              @ Combine the two addresses together and save in r6 for future use
        ldr     r2, =TIMER_BASE                         @ Load the base for all the timer pins
        ldr     r7, =TIMER_ALARM0_OFFSET                @ Load the offset to get address for the alarm
        add     r7, r7, r2                              @ Combine the two addresses together and save in r7 for future use
        ldr     r2, =TIMER_BASE                         @ Load the base for all the timer pins
        
        b       alarm_interrupt_init                    @ Branch to alarm interrupt initialisation
alarm_init_return:
        b       gpio_interrupt_init                     @ Branch to gpio interrupt initialisation
gpio_init_return:
        ldr     r4, =DFLT_ALARM_TIME                    @ Load the default alarm time set above into r4 and do not touch r4 again for any other purpose
        ldr     r1, =TIMER_BASE                         @ Load the base address for the RP2040 timer component
        ldr     r2, =TIMER_INTE_OFFSET                  @ Load the offset address for the timer interrupt enable register
        add     r1, r1, r2                              @ Add these two addresses together
        movs    r0, #1                                  @ Move 0 into r0
        str     r0, [r1]                                @ Store 0 in the calculated address location
        b       update_timer                            @ Run initial timer to begin program
@ Main loop of code
main_loop:
        wfi                                             @ Wait for interrupt
        b       update_timer                            @ Run timer again

@ Enable alarm interrupt
alarm_interrupt_init:
        ldr     r1, [r5]                                @ This loads the at address r2 (VTOR register) into r1
        movs    r2, #ALRM_ISR_OFFSET                    @ This loads the at the memory address ALARM_ISR_OFFSET which is used to get the interrput for the alarm
        add     r2, r1                                  @ This adds the two memory addresses together and stores the result in r2
        ldr     r0, =led_blink                          @ This loads the address of the alarm_isr function
        str     r0, [r2]                                @ Store the address of this function to the memory address stored in r2
        str     r0, [r6]                                @ Store the data in r0 at the address calculated in the above instruction
        b       alarm_init_return                       @ Return back to where the function was called from

@ Enable interrupt for GPIO pins
gpio_interrupt_init:
        movs    r0, #GPIO_BTN_DN                        @ Load GPIO 20 into r0 to be used as the first parameter
        bl      asm_gpio_set_irq                        @ Call function to enable falling edge interrupt for GPIO 20
        movs    r0, #GPIO_BTN_EN                        @ Load GPIO 21 into r0 to be sued as the first parameter
        bl      asm_gpio_set_irq                        @ Call function to enable falling edge interrupt for GPIO 21
        movs    r0, #GPIO_BTN_UP                        @ Load GPIO 22 into r0 tobe used as the first parameter
        bl      asm_gpio_set_irq                        @ Call function to enable falling edge interrupt for GPIO 22
        ldr     r1, [r5]                                @ This loads the at address r2 (VTOR register) into r1
        movs    r2, #GPIO_ISR_OFFSET                    @ This loads the at the memory address ALARM_ISR_OFFSET which is used to get the interrput for the alarm
        add     r2, r1                                  @ This adds the two memory addresses together and stores the result in r2
        ldr     r0, =button_pressed                     @ This loads the address of the alarm_isr function
        str     r0, [r2]                                @ Store the address of this function to the memory address stored in r2
        ldr     r0, =0x2000                             @ Load 0x2000 into r0
        ldr     r5, =TIMER_BASE                         @ Load the base address for the RP2040 timer component
        ldr     r2, =TIMER_INTE_OFFSET                  @ Load the offset address for the timer interrupt enable register
        add     r5, r5, r2                              @ Combine the two addresses together
        str     r0, [r6]                                @ Store the data in r0 at the address calculated in the above instruction
        b       gpio_init_return                        @ Return back to where the function was called from

@ Function to blink LED if program not in static state
.thumb_func
led_blink:
        push    {lr}                                    @ Push the link register onto the stack (for returning)
        cmp     r6, #0                                  @ Compare r6 to 0 (Where the LED should blink or not)
        beq     return                                  @ If should not blink return
        movs    r0, #GPIO_LED_PIN                       @ Move LED pin into r0 to use as first parameter
        bl      asm_gpio_get                            @ Poll state of button and return this value into r0
        cmp     r0, #LED_VAL_ON                         @ Compare returned value and on state
        beq     set_off                                 @ If it is on branch to set_off
        b       set_on                                  @ If it is off branch to set_on

@ Function to return to location before interrupt
return:
        movs    r0, #1                                  @ Move 1 into r0
        ldr     r1, =TIMER_BASE                         @ Load the base address for the RP2040 timer component
        ldr     r2, =TIMER_INTR_OFFSET                  @ Load the offset address for the timer raw interrupts register
        add     r1, r1, r2                              @ Combine the two addresses together 
        str     r0, [r1]                                @ Load #1 from r0 into timer raw timer interrupts register
        pop     {pc}                                    @ Pop the register from the top of the stack to the program counter (return)

@ Function to determine what button was pressed to cause the interrupt and perform appropriate action
.thumb_func
button_pressed:
        b       check_buttons                           @ Skip to correct section of code
@ Function to set LED state to on
set_on:
        movs    r0, #GPIO_LED_PIN                       @ Move the LED pin into r0 to use as first parameter
        movs    r1, #LED_VAL_ON                         @ Move the Positive LED value into r1 to use as second parameter
        bl      asm_gpio_put                            @ Turn on LED
        b       return                                  @ Branch to return

@ Function to set LED state to off
set_off:
        movs    r0, #GPIO_LED_PIN                       @ Move the LED pin into r0 to use as first parameter
        movs    r1, #LED_VAL_OFF                        @ Move the Off LED value into r1 to use as second parameter
        bl      asm_gpio_put                            @ Turn off LED
        b       return                                  @ Branch to return

@ Function to check the status of the buttons
check_buttons:
        push    {lr}                                    @ Push the link register onto the stack to be able to pop later to return
        ldr     r2, =IO_BANK0_BASE                      @ Load the Base address for the rp2040 gpio component
        ldr     r1, =IO_BANK0_PROC0_INTS2_OFFSET        @ Load the offset address for the GPIO interrupt status #2 register
        add     r2, r2, r1                              @ Add the two addresses together
        ldr     r1, [r2]                                @ Load the data stored at the interrupt status register into r1
        ldr     r0 ,=0x2000                             @ Load 0x2000 into r0                
        str     r0,[r2]                                 @ Store 0x200 into the interrupt status register
        ldr     r2,=GPIO_BTN_UP_MSK                     @ Load the address for the up button mask into r2
        cmp     r2, r1                                  @ Compare mask against the mask that trigerred the interrupt
        beq     button_up                               @ If equal, up was pressed so branch to button_up
        ldr     r2,=GPIO_BTN_DN_MSK                     @ Load the address for the down button mask into r2
        cmp     r2, r1                                  @ Compare mask against the mask that trigerred the interrupt
        beq     button_down                             @ If equal, down was pressed so branch to button_down
        ldr     r2,=GPIO_BTN_EN_MSK                     @ Load the address for the enter button mask into r2    
        cmp     r2, r1                                  @ Compare mask against the mask that trigerred the interrupt
        beq     button_enter                            @ If equal, enter was pressed so branch to button_enter
@ Function to put program in flashing state
flash:
        movs    r6 , #1                                 @ Move 1 into r6 to indicate the LED should flash
        b       button_return                           @ Return
@ Function to put program in static state
static:
        movs    r6, #0                                  @ Move 0 into r6 to indicate the LED should remain static
        b       button_return                           @ Return
@ Function to reset flash duration
reset_flash:
        ldr     r4, =DFLT_ALARM_TIME                    @ Set the alarm time (flash duration) to its original value
        b       button_return                           @ Return
@ Function to perform when enter is pressed, toggle program state
button_enter:
        ldr     r2, =IO_BANK0_BASE                      @ Load the base address for the rp2040 timer componenet
        ldr     r1, =IO_BANK0_INTR2_OFFSET              @ Load the offset address for the GPIO raw interrupts #2 register
        add     r2, r2, r1                              @ Combine the two addresses together
        ldr     r1,=GPIO_BTN_EN_MSK                     @ Load the address for the enter button ask into r1
        str     r1,[r2]                                 @ Store the mask in the raw gpio interrupts address
        cmp     r6, #0                                  @ Check whether program is currently static or blinking
        beq     flash                                   @ If currently static branch to flashing
        bne     static                                  @ If currently flashing branch to static
@ Function to run when down pressed, half duration or reset duration
button_down:
        ldr     r2, =IO_BANK0_BASE                      @ Load the base address for the rp2040 timer componenet
        ldr     r1, =IO_BANK0_INTR2_OFFSET              @ Load the offset address for the GPIO raw interrupts #2 register
        add     r2, r2, r1                              @ Combine the two addresses together
        ldr     r1,=GPIO_BTN_DN_MSK                     @ Load the address for the down button ask into r1
        str     r1,[r2]                                 @ Store the mask in the raw gpio interrupts address
        cmp     r6, #0                                  @ Check if currently static
        beq     reset_flash                             @ If static branch to reset flash
        lsrs    r4, r4, #1                              @ Half the flash duration by a logical shift right
        ldr     r2, =0x00000004                         @ Load a safeguard value into r2
        cmp     r4, r2                                  @ Compare the current flash duration to the safeguard value
        ble     upper_button_safeguard                  @ If the duration is less than the safeguard duration branch to upper_safeguard
        b       button_return                           @ Return
@ Function to run when up prssed, double duration or reset duration
button_up:
        ldr     r2, =IO_BANK0_BASE                      @ Load the base address for the rp2040 timer componenet 
        ldr     r1, =IO_BANK0_INTR2_OFFSET              @ Load the offset address for the GPIO raw interrupts #2 register
        add     r2, r2, r1                              @ Combine the two addresses together
        ldr     r1,=GPIO_BTN_UP_MSK                     @ Load the address for the UP button ask into r1            
        str     r1,[r2]                                 @ Store the mask in the raw gpio interrupts address
        cmp     r6, #0                                  @ Check if currently static
        beq     reset_flash                             @ If statuc, branch to reset flash
        lsls    r4, r4, #1                              @ Double the flash duration with a logical shift left 
        ldr     r2, =0x20000000                         @ Load a safeguard value into r2
        cmp     r4, r2                                  @ Compare the current flash duration to safeguard value
        bge     upper_button_safeguard                  @ If the duration is greater than the safeguard duration branch to the lower safeguard
        b       button_return                           @ Return
@ Function to return to position in program before interrupt
button_return:  
        pop     {pc}                                    @ Pop the previosuly stored link register from the stack into the program counter

@ Function to prevent overflow of duration register
upper_button_safeguard:
        ldr     r4, =0b00100000000000000000000000000000 @ Load this binary value into r4 (set flash duration to lowest accepted duration)
        b       button_return                           @ Return 

@ Function to prevent overflow of duration register
lower_button_safeguard:
        ldr     r4, =0b00000000000000000000000000000100 @ Load this binary value into r4 (set flash duration to largest accepted duration)
        b       button_return                           @ Return
@ Function to update the values for the timer
update_timer:     
        ldr     r2, =TIMER_BASE                         @ Load the base address for the rp2040 timer component                   
        ldr     r1, =TIMER_TIMELR_OFFSET                @ Load the offset address for the lower 32 bits of the timer register
        add     r2, r2, r1                              @ Add the two addresses together
        ldr     r1, [r2]                                @ Load the value from the lower 32 bits of the timer register into r1
        add     r1, r1, r4                              @ Add this value to the current duration
        str     r1, [r7]                                @ Store this output in alarm control register
        b       main_loop                               @ Return to main loop


// Functions for testing
check_enter:
        push    {lr}
        movs    r0, #GPIO_BTN_EN
        bl      asm_gpio_get
        pop     {pc}

check_down:
        push    {lr}
        movs    r0, #GPIO_BTN_DN
        bl      asm_gpio_get
        pop     {pc}
        
check_up:
        push    {lr}
        movs    r0, #GPIO_BTN_UP
        bl      asm_gpio_get
        pop     {pc}

        @cmp     r0, #BUT_VAL_ON
        @beq     turnOn
        @b       turnOff

//Functions for testing
turnOn:
        movs    r0, #GPIO_LED_PIN
        movs    r1, #LED_VAL_ON
        bl      asm_gpio_put
        b       check_enter

turnOff:
        movs    r0, #GPIO_LED_PIN
        movs    r1, #LED_VAL_OFF
        bl      asm_gpio_put
        b       check_enter

.align 4
msg:    .asciz "Hello World!\n"

.data
lstate: .word   DFLT_STATE_STRT
ltimer: .word   DFLT_ALARM_TIME

