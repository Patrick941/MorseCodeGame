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

// Declarations // 

.equ    DFLT_STATE_STRT, 1            // Specify the value to start flashing
.equ    DFLT_STATE_STOP, 0            // Specify the value to stop flashing
.equ    DFLT_ALARM_TIME, 1000000      // Specify the default alarm timeout
.equ    SLEEP_TIME, 100               // Specify the amount of ms that we want to sleep for in the main_loop

 

.equ    GPIO21_EDGE_LOW, 0x00400000    // Bit-22 for falling_edge_interrupt-edge event on GP21
.equ    GPIO21_EDGE_HIGH, 0x00800000   // Bit-23 for rising_edge_interrupt-edge event on GP21
.equ    write_a_dash_TIME, 250000
.equ    SPACE, 1000000



.equ    GPIO_BTN_EN,  21              // Specify pin for the "enter" button
.equ    GPIO_DIR_IN,   0              // Specify input direction for a GPIO pin
.equ    GPIO_DIR_OUT,  1              // Specify output direction for a GPIO pin
.equ    GPIO_LED_PIN, 25              // Specify pin for the built-in LED

.equ    LED_VAL_ON,    1              // Specify value that turns the LED "on"
.equ    LED_VAL_OFF,   0              // Specify value that turns the LED "off"
.equ    GPIO_ISR_OFFSET, 0x74         // GPIO is int #13 (vector table entry 29)
.equ    ALRM_ISR_OFFSET, 0x40         // ALARM0 is int #0 (vector table entry 16)

//main function (called by the C code)//
// re-used code from previous labs //

main_asm:

    push  {lr}
    movs    r0, #GPIO_LED_PIN                       // Load the LED pin into r0 to use as first parameter
    bl      asm_gpio_init                           // Call asm_gpio_init to initialise this pin
    movs    r0, #GPIO_LED_PIN                       // Load the LED pin into r0 to use as first parameter
    movs    r1, #GPIO_DIR_OUT                       // Load the output value into r1 to use as second parameter
    bl      asm_gpio_set_dir                        // Set the LED pin to be an output

    movs    r0, #GPIO_BTN_EN                        // Load the Enter button pin into r0 to use as first parameter      
    bl      asm_gpio_init                           // Call asm_gpio_init to intialise this pin
    movs    r0, #GPIO_BTN_EN                        // Load the Enter button pin into r0 to use as first parameter
    movs    r1, GPIO_DIR_IN                         // Load the input value into r1 to use as second parameter
    bl      asm_gpio_set_dir                        // Set the Enter button pin to be an input
    bl      asm_gpio_set_irq                        // Call function to enable falling_edge_interrupt edge interrupt for GPIO 20


    ldr     r5, =PPB_BASE                           // Load the base for all the input and output pins
    ldr     r2, =M0PLUS_VTOR_OFFSET                 // Load the offset to get address for the vector table
    add     r5, r5, r2                              // Save the result in r5 for future use
    ldr     r2, =PPB_BASE                           // Load the base for all the input and output pins
    ldr     r6, =M0PLUS_NVIC_ISER_OFFSET            // Load the offset to get address for the alarm / gpio
    add     r6, r6, r2                              // Combine the two addresses together and save in r6 for future use
    ldr     r2, =TIMER_BASE                         // Load the base for all the timer pins
    ldr     r7, =TIMER_ALARM0_OFFSET                // Load the offset to get address for the alarm
    add     r7, r7, r2                              // Combine the two addresses together and save in r7 for future use
    ldr     r2, =TIMER_BASE                         // Load the base for all the timer pins
   
    bl      setUpArrays


    b       alarm_interrupt_init

//Function call for initilisation of the gpio //

alarm_init_return:

    b    gpio_interrupt_init         
                 
//Initilisation of the gpio//

gpio_init_return:
    ldr     r1, = TIMER_BASE                         // Load the base timer value into r1 (TIMER_BASE)
    ldr     r2, = TIMER_INTE_OFFSET                  // Load the base timer offset value into r2 (TIMER_INTE_OFFSET)  
    adds    r2, r1                                   // Adding r1 and r2 to get the current timer count in r2
    movs    r1, #0                                   // Reseting r1 value back to 0
    str     r1, [r2]                                 // Storing the current timer vaue in r1
    ldr     r4, =0                                   // Setting the flag in r4 to break the loop
    bl      playGame                                 // Branching to playGame function
    pop   {pc}

// Main loop of the program //

main_loop:
                      
    wfi                                              // Keeps waiting until an interrupt disrupts it
    cmp r4, #0                                       // Checks for a flag update
    bne return_to_c                                  // If the flag is not 0 it branches to return_to_c function
    b     main_loop                                  // Keeps looping to check for next interrupt

// Function call for reseting the flag and returning to the loop//

branch_to_asm:
    push {lr}                      
    ldr r4, =0                                      // Resets the flag back to 0
    b       main_loop                               // Branches back to the main loop

// Simple return function//

return_to_c:
    pop {pc}                                        // Returns back to last push

// Function used to update the alarm // 

updateAlarm:
    /// Testing         ldr     r0, =error_string
    /// Testing         bl      printf
    /// Testing         b       main_main_loop

 

    ldr     r1, = TIMER_BASE                           // Load the base timer value into r1 (TIMER_BASE)
    ldr     r2, = TIMER_INTE_OFFSET                    // Load the base timer offset value into r2 (TIMER_INTE_OFFSET)  
    adds    r2, r1                                     // Adding r1 and r2 to get the current timer count in r2
    ldr     r1, [r2]                                   // Storing the current timer vaue in r1
    ldr     r0, =DFLT_ALARM_TIME                       // Load the preset default alarm wait time into r0
    add     r1, r0                                     // Adding the alarm wait time to current timer count
    ldr     r2, = TIMER_BASE                           // Load the base timer value into r2 (TIMER_BASE)
    ldr     r3, = TIMER_ALARM0_OFFSET                  // Load the base timer alarm offset value into r3 (IMER_ALARM0_OFFSET)  
    add     r2, r2, r3                                 // Adding the base timer with the alarm offset 
    str     r1, [r2]                                   // Updating the base value by the offset
    ldr     r2, =TIMER_BASE                            // Load the base timer value into r2 (TIMER_BASE)
    ldr     r1, =TIMER_INTE_OFFSET                     // Load the base timer offset value into r1 (TIMER_INTE_OFFSET)
    add     r2, r2, r1                                 // Adding r1 and r2 to get the current timer count in r2
    movs    r1, #1                                     // Loading 1 int r0 to enable the alarm
    str     r1, [r2]                                   // Storing the updated value in r1
    bx      lr                                         // Function exit

// Intialising the alarm interrupt//

alarm_interrupt_init:

        ldr     r1, [r5]                                // Load the r5 address into r1
        movs    r2, #ALRM_ISR_OFFSET                    // Load the memory address (ALARM_ISR_OFFSET) to get the alarm interrput
        add     r2, r1                                  // Adding the two memory addresses together and stores the result in r2
        ldr     r0, = alrm_isr                          // Load alarm isr function address into r0
        str     r0, [r2]                                // Store r2 in r0
        movs    r0, #1                                  // Resets r0 back to 1
        str     r0, [r6]                                // Store the r6 in r0
        b       alarm_init_return                       // Branch back to orignial function call

// Intialising the gpio interrupt//

gpio_interrupt_init:

        ldr     r1, [r5]                                // Load the r5 address into r1
        movs    r2, #GPIO_ISR_OFFSET                    // Load the memory address (GPIO_ISR_OFFSET) to get the gpio interrput
        add     r2, r1                                  // Adding the two memory addresses together and stores the result in r2
        ldr     r0, =gpio_isr                           // Load alarm isr function address into r0
        str     r0, [r2]                                // Store r2 in r0
        ldr     r0, =0x2000                             // Load 0x2000 into r0
        ldr     r5, =TIMER_BASE                         // Load (TIMER_BASE) address into r5 
        ldr     r2, =TIMER_INTE_OFFSET                  // Load (TIMER_INTE_OFFSET) address into r2
        add     r5, r5, r2                              // Adding the two addresses together and storing in r5
        str     r0, [r6]                                // Store the r6 in r0
        b       gpio_init_return                        // Branch back to orignial function call                  


// Gpio Testing //


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
// Alarm isr function//

alrm_isr:

    /// Testing       //movs    r0, error_string
    /// Testing         ldr     r0, =alarm_string
    /// Testing         bl      printf

    push    {lr}                      
    ldr     r2, =TIMER_BASE                              // Load (TIMER_BASE) into r2
    ldr     r1, =TIMER_INTR_OFFSET                       // Load (TIMER_INTR_OFFSET) into r1
    add     r2, r1                                       // Adding r2 and r1 to get updated value
    movs    r0, #1                                       // Reseting r0 to 1                           
    str     r0, [r2]                                     // Store the updated value in r0
    bl      check_buffer                                 // Branching to check_buffer function
    pop     {pc}                      

 

.thumb_func
// Gpio isr function//

gpio_isr:

    /// Testing         //movs    r0, error_string
    /// Testing         ldr     r0, =event_string
    /// Testing         bl      printf

    push {lr}          
    ldr     r2, =IO_BANK0_BASE                            // Load (IO_BANKBASE) address into r2
    ldr     r1, =IO_BANK0_PROC0_INTS2_OFFSET              // Load (IO_BANK0_PROC0_INTS2_OFFSET ) address into r1
    add     r2, r1                                        // Adding r2 and r1 to get updated value
    ldr     r1, [r2]                                      // Load the address of the update value into r1                                          
    ldr     r0, =GPIO21_EDGE_LOW                          // Load (GPIO21_EDGE_LOW) address into r0   
    cmp     r0, r1                                        // Compare low edge address with the updated value address
    beq     rising_edge_interrupt                         // Branch to rising edge interrupt function if the adresses are equal
    ldr     r0, =GPIO21_EDGE_HIGH                         // Load (GPIO21_EDGE_HIGH) address into r0                               
    cmp     r0, r1                                        // Compare high edge address with the updated value address 
    beq     falling_edge_interrupt                        // Branch to falling edge interrupt function if the adresses are equal

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

//Function used for loading interrupts//

load_interrupt:
    ldr     r0,= IO_BANK0_BASE                          // Load (IO_BANK0_BASE) address into r0
    ldr     r1, =IO_BANK0_INTR2_OFFSET                  // Load (IO_BANK0_INTR2_OFFSET) address into r1
    adds    r0,r1                                       // Adding r0 and r1 to get interrupt
    bx      lr                                          // Branch back 

//Loading low edge//

edgeLow:
    ldr     r2,=GPIO21_EDGE_LOW                         // Load (GPIO21_EDGE_LOW ) address into r2                          
    str     r2,[r0]                                     // Store r0 addres in r2
    bx      lr                                          // Branch back 

// loading High edge//

edgeHigh:   
    ldr     r2,=GPIO21_EDGE_HIGH                        // Load (GPIO21_EDGE_HIGH) address into r2                  
    str     r2,[r0]                                     // Store r0 addres in r2                  
    bx      lr                                          // Branch back 

// Funtction operating the rising edge interrupt//

rising_edge_interrupt:
    /// Testing         //movs    r0, falling_edge_interrupt_string
    /// Testing         ldr     r0, =falling_edge_interrupt_string
    /// Testing         bl      printf

    bl      load_interrupt                              // Branch to load_interrupt function                
    bl      edgeLow                                     // Branch to edgeLow function
    ldr     r1,= TIMER_BASE                             // Load (TIMER_BASE) address into r1  
    ldr     r3, =TIMER_TIMELR_OFFSET                    // Load (TIMER_TIMELR_OFFSET) address into r3  
    adds    r3,r1                                       // Adding r3 and r1 to get updated value     
    ldr     r6, [r3]                                    // Load address of updated value into r6
    bl updateAlarm                                      // Branch to updateAlarm function
    pop {pc} 

// Funtction operating the falling edge interrupt//

falling_edge_interrupt:
    /// Testing         //movs    r0, rising_edge_interrupt_string
    /// Testing         ldr     r0, =rising_edge_interrupt_string
    /// Testing         bl      printf

    bl      load_interrupt                              // Branch to load_interrupt function 
    bl      edgeHigh                                    // Branch to edgeHigh function
    ldr     r1,= TIMER_BASE                             // Load (TIMER_BASE) address into r1            
    ldr     r3, =TIMER_TIMELR_OFFSET                    // Load (TIMER_TIMELR_OFFSET) address into r3  
    adds    r3,r1                                       // Adding r3 and r1 to get updated value
    ldr     r7, [r3]                                    // Load address of updated value into r7
    subs    r7, r6                                      // Subtract r6 from updated value to get the diffrence
    ldr     r1, =write_a_dash_TIME                      // Load the preset time value which needs be surpassed for input to count as a dash
    cmp     r7, r1                                      // Compare the updated value with the minimum daash preset value 
    bhs     write_a_dash                                // Branch to write_a _dash function if the updated value is higher than the preset
    b       write_a_dot                                 // Branch to write_a _dot function

//Function used to write the dot in the terminal//

write_a_dot: 
    ldr     r0,=write_a_dot_string                      // Loads the string  "." into r0
    bl      printf                                      // Prints the contents of r0 in the terminal
    bl      loader                                      // Branch to loader function
    ldr     r1, =0x2E                                   // Loads the ASCII characte "." into r1
    str     r1, [r0]                                    // Stores the addrss of the string r0 in r1
    bl      increment                                   // Branch to increment function

//Function used to write the dash in the terminal//

write_a_dash: 
    ldr     r0,=write_a_dash_string                     // Loads the string  "-" into r0
    bl      printf                                      // Prints the contents of r0 in the terminal
    bl      loader                                      // Branch to loader function
    ldr     r1, =0x2D                                   // Loads the ASCII characte "-" into r1
    str     r1, [r0]                                    // Stores the addrss of the string r0 in r1
    bl      increment                                   // Branch to increment function

//Function used to load current index of the array//

loader:
    ldr     r0, =reserved_array                         // Load (reserved_array) memory address into r0
    ldr     r1, =iter                                   // Load (inter) memory address into r1
    ldr     r2 ,[r1]                                    // Load address value of the iteration
    adds    r0, r2                                      // Adding r0 and r2 to get the current index value
    bx      lr                                          // Branch back 

//Function used to increment the iteration//

increment:
    bl      loader                                      // Branch to loader function
    adds    r2, #4                                      // Adding 4 to r2 to account for the number of bytes
    str     r2, [r1]                                    // Stores the updates value
    bl      updateAlarm                                 // Branch to updateAlarm function
    pop     {pc}                             

//Function used to check the buffer//

check_buffer:
    bl      loader                                      // Branch to loader function
    ldr     r1, =0x0                                    // Load 0 into r1
    str     r1, [r0]                                    // Store r0 in r1
    ldr     r0, =reserved_array                         // Load (reserved_array) memory address into r0
    ldr     r1, =iter                                   // Load (inter) memory address into r1
    ldr     r0, =0x0                                    // Load 0 into r0
    str     r0, [r1]                                    // Store r1 in r0
    ldr     r1,= TIMER_BASE                             // Load (TIMER_BASE) address into r1
    ldr     r2, =TIMER_INTE_OFFSET                      // Load (TIMER_INTE_OFFSET ) address into r2
    adds    r2,r1                                       // Adding r2 and r1 to get updated value
    movs    r1, #0                                      // Reseting r0 to 1
    str     r1, [r2]                                    // Store the updated value in r0
    ldr     r4, =1                                      // Setting the flag in r4 
    pop     {pc}

// Function used in the c code to load the array//

getPointerToArray:
    mov     r9, lr                                      // Sets a return point
    ldr     r0, =reserved_array                         // Load (reserved_array) memory address into r0 (always the start of the array)
    mov     pc, r9                                      // Returns back to return point

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
