 /**
   PWM3 Generated Driver File
 
   @Company
     Microchip Technology Inc.
 
   @File Name
     pwm3.c
 
   @Summary
     This is the generated driver implementation file for the PWM3 driver using PIC10 / PIC12 / PIC16 / PIC18 MCUs
 
   @Description
     This source file provides implementations for driver APIs for PWM3.
     Generation Information :
         Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.8
         Device            :  PIC16F15224
         Driver Version    :  2.01
     The generated drivers are tested against the following:
         Compiler          :  XC8 2.36 and above or later
         MPLAB             :  MPLAB X 6.00
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
 
 /**
   Section: Included Files
 */

 #include <xc.h>
 #include "pwm3.h"

 /**
   Section: PWM Module APIs
 */

 void PWM3_Initialize(void)
 {
    // Set the PWM to the options selected in the PIC10 / PIC12 / PIC16 / PIC18 MCUs.
    // PWM3POL active_hi; PWM3EN enabled; 
    PWM3CON = 0x80;   

    // DCH 0; 
    PWM3DCH = 0x00;   

    // DCL 0; 
    PWM3DCL = 0x00;   

 }

 void PWM3_LoadDutyValue(uint16_t dutyValue)
 {
     // Writing to 8 MSBs of PWM duty cycle in PWMDCH register
     PWM3DCH = (dutyValue & 0x03FC)>>2;
     
     // Writing to 2 LSBs of PWM duty cycle in PWMDCL register
     PWM3DCL = (dutyValue & 0x0003)<<6;
 }
 /**
  End of File
 */
