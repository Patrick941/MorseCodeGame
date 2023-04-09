#include "hardware/regs/addressmap.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/timer.h"
#include "hardware/regs/m0plus.h"



.syntax unified
.cpu    cortex-m0plus
.thumb
.global main_asm
.global branch_to_asm
.global getPointerToArray
.align  4

 

.equ    DFLT_STATE_STRT, 1            @ Specify the value to start flashing
.equ    DFLT_STATE_STOP, 0            @ Specify the value to stop flashing
.equ    DFLT_ALARM_TIME, 1000000      @ Specify the default alarm timeout
.equ    SLEEP_TIME, 100               @ Specify the amount of ms that we want to sleep for in the main_loop

 

.equ    GPIO21_EDGE_LOW, 0x00400000   @ Bit-22 for falling_edge_interrupt-edge event on GP21
.equ    GPIO21_EDGE_HIGH, 0x00800000   @ Bit-23 for rising_edge_interrupt-edge event on GP21
.equ    write_a_dash_TIME, 250000
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

    push  {lr}
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
    bl      asm_gpio_set_irq                        @ Call function to enable falling_edge_interrupt edge interrupt for GPIO 20


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
   
    bl      setUpArrays


    b       alarm_interrupt_init

alarm_init_return:

    b    gpio_interrupt_init                          @Initilise the GPIO Interrupt interrupt
gpio_init_return:
    ldr     r1, = TIMER_BASE
    ldr     r2, = TIMER_INTE_OFFSET  @ Enable the alarm by writing 1 to the register 
    adds    r2, r1
    movs    r1, #0
    str     r1, [r2]
    ldr     r4, =0                                      @Break main_loop flag
    bl      playGame
    pop   {pc}



main_loop:
                      
    wfi                       // Wait here until any interrupt fires
    cmp r4, #0
    bne return_to_c

    b     main_loop                // Always branch back to main_loop


branch_to_asm:
    push {lr}                      
    ldr r4, =0    
    b       main_loop

return_to_c:
    pop {pc}

 

updateAlarm:
    /// Testing         ldr     r0, =error_string
    /// Testing         bl      printf
    /// Testing         b       main_main_loop

 

    ldr     r2, =TIMER_BASE                            @Get the current timer count from (TIMER_BASE + TIMER_TIMELR_OFFSET) register
    ldr     r1, =TIMER_TIMELR_OFFSET
    add     r2, r1
    ldr     r1, [r2]                                    @
    ldr     r0, =DFLT_ALARM_TIME
    add     r1, r0                                      @ Add the time delay you want to wait for to the current timer count
    ldr     r2, = TIMER_BASE                             @ Push the updated value to (TIMER_BASE + TIMER_ALARM0_OFFSET)
    ldr     r3, = TIMER_ALARM0_OFFSET
    add     r2, r2, r3
    str     r1, [r2]                                    @
    ldr     r2, =TIMER_BASE                             @ Enable the alarm by writing 1 to the register
    ldr     r1, =TIMER_INTE_OFFSET
    add     r2, r2, r1
    movs    r1, #1
    str     r1, [r2]
    bx      lr                                          @Exit subroutine

 

//

// Sets up the alrm_isr in the RAM vector table and enables the correct interrupt

//

alarm_interrupt_init:

        ldr     r1, [r5]                                @ This loads the at address r2 (VTOR register) into r1
        movs    r2, #ALRM_ISR_OFFSET                    @ This loads the at the memory address ALARM_ISR_OFFSET which is used to get the interrput for the alarm
        add     r2, r1                                  @ This adds the two memory addresses together and stores the result in r2
        ldr     r0, = alrm_isr                        @ This loads the address of the alarm_isr function
        str     r0, [r2]                                @ Store the address of this function to the memory address stored in r2
        movs    r0, #1                                  @ Mov 1 into r0
        str     r0, [r6]                                @ Store the data in r0 at the address calculated in the above instruction
        b       alarm_init_return                       @ Return back to where the function was called from

//

// Sets up the gpio_isr in the RAM vector table and enables the correct interrupt

//

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

//

// Service the pending interrupt from the GPIO


/// Testing 
/// Testing
/// Testing check_up:
/// Testing         push    {lr}
/// Testing         movs    r0, #GPIO_BTN_UP
/// Testing         bl      asm_gpio_get
/// Testing         pop     {pc}
/// Testing 
/// Testing         @cmp     r0, #BUT_VAL_ON
/// Testing         @beq     turnOn
/// Testing         @b       turnOff
/// Testing 
/// Testing //Functions for testing
/// Testing turnOn:
/// Testing         movs    r0, #GPIO_LED_PIN
/// Testing         movs    r1, #LED_VAL_ON
/// Testing         bl      asm_gpio_put
/// Testing         //movs    r0, error_string
/// Testing         ldr     r0, =error_string
/// Testing         bl      printf
/// Testing         b       check_up
/// Testing 
/// Testing turnOff:
/// Testing         movs    r0, #GPIO_LED_PIN
/// Testing         movs    r1, #LED_VAL_OFF
/// Testing         bl      asm_gpio_put
/// Testing         //movs    r0, error_string
/// Testing         ldr     r0, =error_string
/// Testing         bl      printf
/// Testing         b       check_up
//

 

.thumb_func
alrm_isr:

    /// Testing         //movs    r0, error_string
    /// Testing         ldr     r0, =alarm_string
    /// Testing         bl      printf

    push    {lr}                     
    ldr     r2, =TIMER_BASE
    ldr     r1, =TIMER_INTR_OFFSET
    add     r2, r1
    movs    r0, #1                                  
    str     r0, [r2]
    bl      check_buffer
    pop     {pc}                      

 

.thumb_func
gpio_isr:

    /// Testing         //movs    r0, error_string
    /// Testing         ldr     r0, =event_string
    /// Testing         bl      printf

    push {lr}          
    ldr     r2, =IO_BANK0_BASE
    ldr     r1, = IO_BANK0_PROC0_INTS2_OFFSET
    add     r2, r1
    ldr     r1, [r2]                                                
    ldr     r0, =GPIO21_EDGE_LOW                                    
    cmp     r0, r1                                                  
    beq     rising_edge_interrupt
  
    ldr     r0, =GPIO21_EDGE_HIGH                                  
    cmp     r0, r1                                                  
    beq     falling_edge_interrupt

// Doesn't work quite right but is close
// Doesn't work quite right but is closeload_interrupt_low:
// Doesn't work quite right but is close    /// Testing         //movs    r0, falling_edge_interrupt_string
// Doesn't work quite right but is close    /// Testing         ldr     r0, =falling_edge_interrupt_string
// Doesn't work quite right but is close    /// Testing         bl      printf
// Doesn't work quite right but is close    ldr     r1,= IO_BANK0_BASE
// Doesn't work quite right but is close    ldr     r2, =IO_BANK0_INTR2_OFFSET         
// Doesn't work quite right but is close    adds    r2,r1
// Doesn't work quite right but is close    ldr     r1,=GPIO21_EDGE_LOW                                   
// Doesn't work quite right but is close    str     r1,[r2] 
// Doesn't work quite right but is close    bx      lr
// Doesn't work quite right but is close
// Doesn't work quite right but is closeload_interrupt_high: 
// Doesn't work quite right but is close    /// Testing         //movs    r0, rising_edge_interrupt_string
// Doesn't work quite right but is close    /// Testing         ldr     r0, =rising_edge_interrupt_string
// Doesn't work quite right but is close    /// Testing         bl      printf
// Doesn't work quite right but is close    ldr     r1,= IO_BANK0_BASE
// Doesn't work quite right but is close    ldr     r2, =IO_BANK0_INTR2_OFFSET         
// Doesn't work quite right but is close    adds    r2,r1
// Doesn't work quite right but is close    ldr     r1,=GPIO21_EDGE_HIGH                                   
// Doesn't work quite right but is close    str     r1,[r2]                                               
// Doesn't work quite right but is close    bx      lr
// Doesn't work quite right but is close

load_interrupt:
    ldr     r0,= IO_BANK0_BASE
    ldr     r1, =IO_BANK0_INTR2_OFFSET         @ Load interrupt status from (IO_BANK0_BASE + IO_BANK0_PROC0_INTS2_OFFSET)
    adds    r0,r1
    bx      lr

edgeLow:
    ldr     r2,=GPIO21_EDGE_LOW                               
    str     r2,[r0] 
    bx      lr

edgeHigh:   
    ldr     r2,=GPIO21_EDGE_HIGH                                
    str     r2,[r0]                                          
    bx      lr

rising_edge_interrupt:
    /// Testing         //movs    r0, falling_edge_interrupt_string
    /// Testing         ldr     r0, =falling_edge_interrupt_string
    /// Testing         bl      printf
    bl      load_interrupt 
    bl      edgeLow                     
    ldr     r1,= TIMER_BASE              
    ldr     r3, =TIMER_TIMELR_OFFSET      
    adds    r3,r1                         
    ldr     r6, [r3]
    bl updateAlarm
    pop {pc} 

falling_edge_interrupt:
    /// Testing         //movs    r0, rising_edge_interrupt_string
    /// Testing         ldr     r0, =rising_edge_interrupt_string
    /// Testing         bl      printf
    bl      load_interrupt  
    bl      edgeHigh   
    ldr     r1,= TIMER_BASE               
    ldr     r3, =TIMER_TIMELR_OFFSET      
    adds    r3,r1
    ldr     r7, [r3]
    subs    r7, r6
    ldr     r1, =write_a_dash_TIME                                      
    cmp     r7, r1
    bhs     write_a_dash
    b       write_a_dot

write_a_dot: 
    ldr     r0,=write_a_dot_string
    bl      printf
    bl      loader
    ldr     r1, =0x2E
    str     r1, [r0]
    bl      increment

write_a_dash: 
    ldr     r0,=write_a_dash_string
    bl      printf
    bl      loader
    ldr     r1, =0x2D
    str     r1, [r0]
    bl      increment

loader:
    ldr     r0, =reserved_array
    ldr     r1, =iter
    ldr     r2 ,[r1]
    adds    r0, r2
    bx      lr

increment:
    bl      loader
    adds    r2, #4
    str     r2, [r1]

    bl      updateAlarm
    pop     {pc} 

check_buffer:
    bl      loader
    ldr     r1, =0x0
    str     r1, [r0]
    ldr     r0, =reserved_array
    ldr     r1, =iter
    ldr     r0, =0x0
    str     r0, [r1]
    ldr     r1,= TIMER_BASE
    ldr     r2, =TIMER_INTE_OFFSET
    adds    r2,r1
    movs    r1, #0
    str     r1, [r2]
    ldr     r4, =1
    pop     {pc}

getPointerToArray:
    mov     r9, lr
    ldr     r0, =reserved_array
    mov     pc, r9

return:
    pop     {pc}

.section .rodata
.align 4
write_a_dot_string: .string "||                         .                         ||\n"
write_a_dash_string: .string "||                         -                         ||\n"
error_string: .string "Error occured, should not be here\n"
alarm_string: .string "Alarm entered\n"
event_string: .string "Event has caused interrupt\n"
rising_edge_interrupt_string: .string "Rising edge interrupt has occured\n"
falling_edge_interrupt_string: .string "Falling edge interrupt has occured\n"

.data
reserved_array: .space 24
lstate: .long DFLT_STATE_STRT
ltimer: .long DFLT_ALARM_TIME
iter: .long 0
