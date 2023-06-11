/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.8
        Device            :  PIC16F15224
        Driver Version    :  2.00
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

#include "mcc_generated_files/mcc.h"

#include "i2c-callbacks.h"

/*
                         Main application
 */

uint16_t v_out_raw = 0;
float v_out = 0.0;

void main(void)
{
    // initialize the device
    SYSTEM_Initialize();

    // When using interrupts, you need to set the Global and Peripheral Interrupt Enable bits
    // Use the following macros to:

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();

    // Disable the Global Interrupts
    //INTERRUPT_GlobalInterruptDisable();

    // Disable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptDisable();

    I2C1_Open();
    I2CSetCallbacks();
    
    while (1)
    {
        // Add your application code
        
        v_out_raw = ADC_GetConversion(VOut);
        v_out = v_out_raw * 0.0050857544f;
        //v_out = 25.0;
        
        if (!Done_GetValue())
        {
            PWM3_LoadDutyValue(16);
        }
        else
        {
            PWM3_LoadDutyValue(0);
        }
        
        if (reg_charge)
        {
            Charge_SetHigh();
        }
        else
        {
            Charge_SetLow();
        }
        
        if (reg_kick_a)
        {
            KickA_SetHigh();
            
            for(;reg_kick_a > 0; --reg_kick_a)
                __delay_us(100);
            
            KickA_SetLow();
            reg_kick_a = 0;
        }
        
        if (reg_kick_b)
        {
            KickB_SetHigh();
            
            for(;reg_kick_b > 0; --reg_kick_b)
                __delay_us(100);
            
            KickB_SetLow();
            reg_kick_b = 0;
        }
        
        if (0)//!Button_GetValue())
        {
            //Discharge_SetHigh();
        }
        else
        {
            //Discharge_SetLow();
        }
        
    }
    
    I2C1_Close();
}
/**
 End of File
*/