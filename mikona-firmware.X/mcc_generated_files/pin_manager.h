/**
  @Generated Pin Manager Header File

  @Company:
    Microchip Technology Inc.

  @File Name:
    pin_manager.h

  @Summary:
    This is the Pin Manager file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description
    This header file provides APIs for driver for .
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.8
        Device            :  PIC16F15224
        Driver Version    :  2.11
    The generated drivers are tested against the following:
        Compiler          :  XC8 2.36 and above
        MPLAB 	          :  MPLAB X 6.00	
*/

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/

#ifndef PIN_MANAGER_H
#define PIN_MANAGER_H

/**
  Section: Included Files
*/

#include <xc.h>

#define INPUT   1
#define OUTPUT  0

#define HIGH    1
#define LOW     0

#define ANALOG      1
#define DIGITAL     0

#define PULL_UP_ENABLED      1
#define PULL_UP_DISABLED     0

// get/set LED aliases
#define LED_TRIS                 TRISAbits.TRISA2
#define LED_LAT                  LATAbits.LATA2
#define LED_PORT                 PORTAbits.RA2
#define LED_WPU                  WPUAbits.WPUA2
#define LED_OD                   ODCONAbits.ODCA2
#define LED_ANS                  ANSELAbits.ANSA2
#define LED_SetHigh()            do { LATAbits.LATA2 = 1; } while(0)
#define LED_SetLow()             do { LATAbits.LATA2 = 0; } while(0)
#define LED_Toggle()             do { LATAbits.LATA2 = ~LATAbits.LATA2; } while(0)
#define LED_GetValue()           PORTAbits.RA2
#define LED_SetDigitalInput()    do { TRISAbits.TRISA2 = 1; } while(0)
#define LED_SetDigitalOutput()   do { TRISAbits.TRISA2 = 0; } while(0)
#define LED_SetPullup()          do { WPUAbits.WPUA2 = 1; } while(0)
#define LED_ResetPullup()        do { WPUAbits.WPUA2 = 0; } while(0)
#define LED_SetPushPull()        do { ODCONAbits.ODCA2 = 0; } while(0)
#define LED_SetOpenDrain()       do { ODCONAbits.ODCA2 = 1; } while(0)
#define LED_SetAnalogMode()      do { ANSELAbits.ANSA2 = 1; } while(0)
#define LED_SetDigitalMode()     do { ANSELAbits.ANSA2 = 0; } while(0)

// get/set Done aliases
#define Done_TRIS                 TRISAbits.TRISA4
#define Done_LAT                  LATAbits.LATA4
#define Done_PORT                 PORTAbits.RA4
#define Done_WPU                  WPUAbits.WPUA4
#define Done_OD                   ODCONAbits.ODCA4
#define Done_ANS                  ANSELAbits.ANSA4
#define Done_SetHigh()            do { LATAbits.LATA4 = 1; } while(0)
#define Done_SetLow()             do { LATAbits.LATA4 = 0; } while(0)
#define Done_Toggle()             do { LATAbits.LATA4 = ~LATAbits.LATA4; } while(0)
#define Done_GetValue()           PORTAbits.RA4
#define Done_SetDigitalInput()    do { TRISAbits.TRISA4 = 1; } while(0)
#define Done_SetDigitalOutput()   do { TRISAbits.TRISA4 = 0; } while(0)
#define Done_SetPullup()          do { WPUAbits.WPUA4 = 1; } while(0)
#define Done_ResetPullup()        do { WPUAbits.WPUA4 = 0; } while(0)
#define Done_SetPushPull()        do { ODCONAbits.ODCA4 = 0; } while(0)
#define Done_SetOpenDrain()       do { ODCONAbits.ODCA4 = 1; } while(0)
#define Done_SetAnalogMode()      do { ANSELAbits.ANSA4 = 1; } while(0)
#define Done_SetDigitalMode()     do { ANSELAbits.ANSA4 = 0; } while(0)

// get/set Charge aliases
#define Charge_TRIS                 TRISAbits.TRISA5
#define Charge_LAT                  LATAbits.LATA5
#define Charge_PORT                 PORTAbits.RA5
#define Charge_WPU                  WPUAbits.WPUA5
#define Charge_OD                   ODCONAbits.ODCA5
#define Charge_ANS                  ANSELAbits.ANSA5
#define Charge_SetHigh()            do { LATAbits.LATA5 = 1; } while(0)
#define Charge_SetLow()             do { LATAbits.LATA5 = 0; } while(0)
#define Charge_Toggle()             do { LATAbits.LATA5 = ~LATAbits.LATA5; } while(0)
#define Charge_GetValue()           PORTAbits.RA5
#define Charge_SetDigitalInput()    do { TRISAbits.TRISA5 = 1; } while(0)
#define Charge_SetDigitalOutput()   do { TRISAbits.TRISA5 = 0; } while(0)
#define Charge_SetPullup()          do { WPUAbits.WPUA5 = 1; } while(0)
#define Charge_ResetPullup()        do { WPUAbits.WPUA5 = 0; } while(0)
#define Charge_SetPushPull()        do { ODCONAbits.ODCA5 = 0; } while(0)
#define Charge_SetOpenDrain()       do { ODCONAbits.ODCA5 = 1; } while(0)
#define Charge_SetAnalogMode()      do { ANSELAbits.ANSA5 = 1; } while(0)
#define Charge_SetDigitalMode()     do { ANSELAbits.ANSA5 = 0; } while(0)

// get/set RC0 procedures
#define RC0_SetHigh()            do { LATCbits.LATC0 = 1; } while(0)
#define RC0_SetLow()             do { LATCbits.LATC0 = 0; } while(0)
#define RC0_Toggle()             do { LATCbits.LATC0 = ~LATCbits.LATC0; } while(0)
#define RC0_GetValue()              PORTCbits.RC0
#define RC0_SetDigitalInput()    do { TRISCbits.TRISC0 = 1; } while(0)
#define RC0_SetDigitalOutput()   do { TRISCbits.TRISC0 = 0; } while(0)
#define RC0_SetPullup()             do { WPUCbits.WPUC0 = 1; } while(0)
#define RC0_ResetPullup()           do { WPUCbits.WPUC0 = 0; } while(0)
#define RC0_SetAnalogMode()         do { ANSELCbits.ANSC0 = 1; } while(0)
#define RC0_SetDigitalMode()        do { ANSELCbits.ANSC0 = 0; } while(0)

// get/set RC1 procedures
#define RC1_SetHigh()            do { LATCbits.LATC1 = 1; } while(0)
#define RC1_SetLow()             do { LATCbits.LATC1 = 0; } while(0)
#define RC1_Toggle()             do { LATCbits.LATC1 = ~LATCbits.LATC1; } while(0)
#define RC1_GetValue()              PORTCbits.RC1
#define RC1_SetDigitalInput()    do { TRISCbits.TRISC1 = 1; } while(0)
#define RC1_SetDigitalOutput()   do { TRISCbits.TRISC1 = 0; } while(0)
#define RC1_SetPullup()             do { WPUCbits.WPUC1 = 1; } while(0)
#define RC1_ResetPullup()           do { WPUCbits.WPUC1 = 0; } while(0)
#define RC1_SetAnalogMode()         do { ANSELCbits.ANSC1 = 1; } while(0)
#define RC1_SetDigitalMode()        do { ANSELCbits.ANSC1 = 0; } while(0)

// get/set RC2 procedures
#define RC2_SetHigh()            do { LATCbits.LATC2 = 1; } while(0)
#define RC2_SetLow()             do { LATCbits.LATC2 = 0; } while(0)
#define RC2_Toggle()             do { LATCbits.LATC2 = ~LATCbits.LATC2; } while(0)
#define RC2_GetValue()              PORTCbits.RC2
#define RC2_SetDigitalInput()    do { TRISCbits.TRISC2 = 1; } while(0)
#define RC2_SetDigitalOutput()   do { TRISCbits.TRISC2 = 0; } while(0)
#define RC2_SetPullup()             do { WPUCbits.WPUC2 = 1; } while(0)
#define RC2_ResetPullup()           do { WPUCbits.WPUC2 = 0; } while(0)
#define RC2_SetAnalogMode()         do { ANSELCbits.ANSC2 = 1; } while(0)
#define RC2_SetDigitalMode()        do { ANSELCbits.ANSC2 = 0; } while(0)

// get/set RC3 procedures
#define RC3_SetHigh()            do { LATCbits.LATC3 = 1; } while(0)
#define RC3_SetLow()             do { LATCbits.LATC3 = 0; } while(0)
#define RC3_Toggle()             do { LATCbits.LATC3 = ~LATCbits.LATC3; } while(0)
#define RC3_GetValue()              PORTCbits.RC3
#define RC3_SetDigitalInput()    do { TRISCbits.TRISC3 = 1; } while(0)
#define RC3_SetDigitalOutput()   do { TRISCbits.TRISC3 = 0; } while(0)
#define RC3_SetPullup()             do { WPUCbits.WPUC3 = 1; } while(0)
#define RC3_ResetPullup()           do { WPUCbits.WPUC3 = 0; } while(0)
#define RC3_SetAnalogMode()         do { ANSELCbits.ANSC3 = 1; } while(0)
#define RC3_SetDigitalMode()        do { ANSELCbits.ANSC3 = 0; } while(0)

// get/set VOut aliases
#define VOut_TRIS                 TRISCbits.TRISC4
#define VOut_LAT                  LATCbits.LATC4
#define VOut_PORT                 PORTCbits.RC4
#define VOut_WPU                  WPUCbits.WPUC4
#define VOut_OD                   ODCONCbits.ODCC4
#define VOut_ANS                  ANSELCbits.ANSC4
#define VOut_SetHigh()            do { LATCbits.LATC4 = 1; } while(0)
#define VOut_SetLow()             do { LATCbits.LATC4 = 0; } while(0)
#define VOut_Toggle()             do { LATCbits.LATC4 = ~LATCbits.LATC4; } while(0)
#define VOut_GetValue()           PORTCbits.RC4
#define VOut_SetDigitalInput()    do { TRISCbits.TRISC4 = 1; } while(0)
#define VOut_SetDigitalOutput()   do { TRISCbits.TRISC4 = 0; } while(0)
#define VOut_SetPullup()          do { WPUCbits.WPUC4 = 1; } while(0)
#define VOut_ResetPullup()        do { WPUCbits.WPUC4 = 0; } while(0)
#define VOut_SetPushPull()        do { ODCONCbits.ODCC4 = 0; } while(0)
#define VOut_SetOpenDrain()       do { ODCONCbits.ODCC4 = 1; } while(0)
#define VOut_SetAnalogMode()      do { ANSELCbits.ANSC4 = 1; } while(0)
#define VOut_SetDigitalMode()     do { ANSELCbits.ANSC4 = 0; } while(0)

// get/set Discharge aliases
#define Discharge_TRIS                 TRISCbits.TRISC5
#define Discharge_LAT                  LATCbits.LATC5
#define Discharge_PORT                 PORTCbits.RC5
#define Discharge_WPU                  WPUCbits.WPUC5
#define Discharge_OD                   ODCONCbits.ODCC5
#define Discharge_ANS                  ANSELCbits.ANSC5
#define Discharge_SetHigh()            do { LATCbits.LATC5 = 1; } while(0)
#define Discharge_SetLow()             do { LATCbits.LATC5 = 0; } while(0)
#define Discharge_Toggle()             do { LATCbits.LATC5 = ~LATCbits.LATC5; } while(0)
#define Discharge_GetValue()           PORTCbits.RC5
#define Discharge_SetDigitalInput()    do { TRISCbits.TRISC5 = 1; } while(0)
#define Discharge_SetDigitalOutput()   do { TRISCbits.TRISC5 = 0; } while(0)
#define Discharge_SetPullup()          do { WPUCbits.WPUC5 = 1; } while(0)
#define Discharge_ResetPullup()        do { WPUCbits.WPUC5 = 0; } while(0)
#define Discharge_SetPushPull()        do { ODCONCbits.ODCC5 = 0; } while(0)
#define Discharge_SetOpenDrain()       do { ODCONCbits.ODCC5 = 1; } while(0)
#define Discharge_SetAnalogMode()      do { ANSELCbits.ANSC5 = 1; } while(0)
#define Discharge_SetDigitalMode()     do { ANSELCbits.ANSC5 = 0; } while(0)

/**
   @Param
    none
   @Returns
    none
   @Description
    GPIO and peripheral I/O initialization
   @Example
    PIN_MANAGER_Initialize();
 */
void PIN_MANAGER_Initialize (void);

/**
 * @Param
    none
 * @Returns
    none
 * @Description
    Interrupt on Change Handling routine
 * @Example
    PIN_MANAGER_IOC();
 */
void PIN_MANAGER_IOC(void);



#endif // PIN_MANAGER_H
/**
 End of File
*/