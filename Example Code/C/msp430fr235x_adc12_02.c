/* --COPYRIGHT--,BSD_EX
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 *
 *                       MSP430 CODE EXAMPLE DISCLAIMER
 *
 * MSP430 code examples are self-contained low-level programs that typically
 * demonstrate a single peripheral function or device feature in a highly
 * concise manner. For this the code may rely on the device's power-on default
 * register values and settings such as the clock configuration and care must
 * be taken when combining code from several examples to avoid potential side
 * effects. Also see www.ti.com/grace for a GUI- and www.ti.com/msp430ware
 * for an API functional library-approach to peripheral configuration.
 *
 * --/COPYRIGHT--*/
//******************************************************************************
//  MSP430FR235x Demo - ADC, Sample A1, internal 1.5V Ref, Set P1.0 if A1 > 0.5V
//
//  Description: This example works on Single-Channel Single-Conversion Mode.
//  Software sets ADCSC to start sample and conversion - ADCSC automatically
//  cleared at EOC. ADC internal oscillator times sample (16x) and conversion.
//  In Mainloop MSP430 waits in LPM0 to save power until ADC conversion complete,
//  ADC_ISR will force exit from LPM0 in Mainloop on reti.
//  If A1 > 0.5V, P1.2 set, else reset.
//  ACLK = default REFO ~32768Hz, MCLK = SMCLK = default DCODIV ~1MHz.
//
//               MSP430FR2355
//            -----------------
//        /|\|                 |
//         | |                 |
//         --|RST              |
//           |                 |
//       >---|P1.1/A1      P1.2|--> LED
//
//
//   Cash Hao
//   Texas Instruments Inc.
//   November 2016
//   Built with IAR Embedded Workbench v6.50.0 & Code Composer Studio v6.2.0
//******************************************************************************
#include <msp430.h>

unsigned int ADC_Result;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                                 // Stop WDT

    // Configure GPIO
    P1DIR |= BIT2;                                            // Set P1.2/LED to output direction
    P1OUT &= ~BIT2;                                           // P1.2 LED off

    // Configure ADC A1 pin
    P1SEL0 |= BIT1;
    P1SEL1 |= BIT1;

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Configure ADC10
    ADCCTL0 |= ADCSHT_2 | ADCON;                              // ADCON, S&H=16 ADC clks
    ADCCTL1 |= ADCSHP;                                        // ADCCLK = MODOSC; sampling timer
    ADCCTL2 &= ~ADCRES;                                       // clear ADCRES in ADCCTL
    ADCCTL2 |= ADCRES_2;                                      // 12-bit conversion results
    ADCIE |= ADCIE0;                                          // Enable ADC conv complete interrupt
    ADCMCTL0 |= ADCINCH_1 | ADCSREF_1;                        // A1 ADC input select; Vref=1.5V

    // Configure reference module
    PMMCTL0_H = PMMPW_H;                                      // Unlock the PMM registers
    PMMCTL2 = INTREFEN | REFVSEL_0;                           // Enable internal 1.5V reference
    while(!(PMMCTL2 & REFGENRDY));                            // Poll till internal reference settles

    while(1)
    {
        ADCCTL0 |= ADCENC | ADCSC;                            // Sampling and conversion start
        __bis_SR_register(LPM0_bits | GIE);                   // LPM0, ADC_ISR will force exit
        if (ADC_Result < 0x555)                               // 0x555 represent 0.5V
            P1OUT &= ~BIT2;                                   // Clear P1.2 LED off
        else
            P1OUT |= BIT2;                                    // Set P1.2 LED on
        __delay_cycles(5000);
    }
}

// ADC interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC_VECTOR))) ADC_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(ADCIV,ADCIV_ADCIFG))
    {
        case ADCIV_NONE:
            break;
        case ADCIV_ADCOVIFG:
            break;
        case ADCIV_ADCTOVIFG:
            break;
        case ADCIV_ADCHIIFG:
            break;
        case ADCIV_ADCLOIFG:
            break;
        case ADCIV_ADCINIFG:
            break;
        case ADCIV_ADCIFG:
            ADC_Result = ADCMEM0;
            __bic_SR_register_on_exit(LPM0_bits);              // Clear CPUOFF bit from LPM0
            break;
        default:
            break;
    }
}
