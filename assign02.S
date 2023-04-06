#include "hardware/regs/addressmap.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/timer.h"
#include "hardware/regs/m0plus.h"



.syntax unified
.cpu    cortex-m0plus
.thumb
.global main_asm
.global input
.global grabArray
.align  4

 

.equ    DFLT_STATE_STRT, 1            @ Specify the value to start flashing
.equ    DFLT_STATE_STOP, 0            @ Specify the value to stop flashing
.equ    DFLT_ALARM_TIME, 3000000      @ Specify the default alarm timeout
.equ    SLEEP_TIME, 100               @ Specify the amount of ms that we want to sleep for in the loop

 

.equ    GPIO21_EDGE_LOW, 0x00400000   @ Bit-22 for falling-edge event on GP21
.equ    GPIO21_EDGE_HIGH, 0x00800000   @ Bit-23 for rising-edge event on GP21
.equ    DASH_TIME, 250000
.equ    SPACE, 1000000



.equ    GPIO_BTN_EN,  21              @ Specify pin for the "enter" button
.equ    GPIO_DIR_IN,   0              @ Specify input direction for a GPIO pin
.equ    GPIO_DIR_OUT,  1              @ Specify output direction for a GPIO pin
.equ    GPIO_LED_PIN, 25              @ Specify pin for the built-in LED

.equ    LED_VAL_ON,    1              @ Specify value that turns the LED "on"
.equ    LED_VAL_OFF,   0              @ Specify value that turns the LED "off"
.equ    GPIO_ISR_OFFSET, 0x74         @ GPIO is int #13 (vector table entry 29)
.equ    ALRM_ISR_OFFSET, 0x40         @ ALARM0 is int #0 (vector table entry 16)


main_asm:


  .align 
  4lstate: .word   DFLT_STATE_STRT
  ltimer: .word   DFLT_ALARM_TIME
  iter: .word     0
