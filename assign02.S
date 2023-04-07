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
        bl      asm_gpio_set_irq                        @ Call function to enable falling edge interrupt for GPIO 20
   
 
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
   
        b       alarm_interrupt_init

alarm_init_return:

        b    gpio_interrupt_init                          @Initilise the GPIO Interrupt interrupt
gpio_init_return:


alarm_interrupt_init:

        ldr     r1, [r5]                                @ This loads the at address r2 (VTOR register) into r1
        movs    r2, #ALRM_ISR_OFFSET                    @ This loads the at the memory address ALARM_ISR_OFFSET which is used to get the interrput for the alarm
        add     r2, r1                                  @ This adds the two memory addresses together and stores the result in r2
        ldr     r0, = alrm_isr                        @ This loads the address of the alarm_isr function
        str     r0, [r2]                                @ Store the address of this function to the memory address stored in r2
        movs    r0, #1                                  @ Mov 1 into r0
        str     r0, [r6]                                @ Store the data in r0 at the address calculated in the above instruction
        b       alarm_init_return                       @ Return back to where the function was called from


gpio_interrupt_init:

        ldr     r1, [r5]                                @ This loads the at address r2 (VTOR register) into r1
        movs    r2, #GPIO_ISR_OFFSET                    @ This loads the at the memory address ALARM_ISR_OFFSET which is used to get the interrput for the alarm
        add     r2, r1                                  @ This adds the two memory addresses together and stores the result in r2
        ldr     r0, =gpio_isr                  @ This loads the address of the alarm_isr function
        str     r0, [r2]                                @ Store the address of this function to the memory address stored in r2
        ldr     r0, =0x2000                             @ Load 0x2000 into r0
        ldr     r5, =TIMER_BASE                         @ Load the base address for the RP2040 timer component
        ldr     r2, =TIMER_INTE_OFFSET                  @ Load the offset address for the timer interrupt enable register
        add     r5, r5, r2                              @ Combine the two addresses together
        str     r0, [r6]                                @ Store the data in r0 at the address calculated in the above instruction
        b       gpio_init_return                        @ Return back to where the function was called from              
