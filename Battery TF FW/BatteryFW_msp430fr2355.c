/* Battery Test Fixure MSP430FR2355 Firmware 
// Author: N. Ramirez
// __________________________________________________________________________________
//
//  Mainloop:
//  Monitor ADC (n12vFlt)
//  if ADC is High
//      Disable VBat1 and VBat2
//      Enable ChrgEn1 and ChrgEn2
//  else
//      Enable VBat1 and VBat2
//      Disable ChrgEn1 and ChrgEn2
//
//  This works on Single-Channel Single-Conversion Mode.
//  Software sets ADCSC to start sample and conversion - ADCSC automatically
//  cleared at EOC. ADC internal oscillator times sample (16x) and conversion.
//  In Mainloop MSP430 waits in LPM0 to save power until ADC conversion complete,
//  ADC_ISR will force exit from LPM0 in Mainloop on reti.
//  
//  ACLK = default REFO ~32768Hz, MCLK = SMCLK = default DCODIV ~1MHz.
//
//               MSP430FR2355
//            -----------------
//        /|\|                 |
//         | |                 |
//         --|RST              |
//           |             P1.0|--> LED
//       >---|P1.1/A1      P1.2|--> LED
//
//
//  __________________________________________________________________________________*/
#include <msp430.h>

#define LED_1   (BIT0)                      // P1.0 LED output

unsigned int ADC_Result;



int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                                 // Stop WDT

    // Configure GPIO
    P6      // P6.2 (n12VFlt) 
    // P1DIR |= BIT0 + BIT2;                                            // Set P1.0 and P1.2/LED to output direction
//    P1OUT &= ~BIT2;                                           // P1.2 LED off
    // P1OUT |= BIT0 + BIT2;                                    // Set P1.0 and P1.2 LED on

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
    // ADCMCTL0 |= ADCINCH_1;                                   // A1 ADC input select; Vref=AVCC
    // ADCMCTL0 = ADCINCH_13 | ADCSREF_0;      // A13 ADC input select = 1.5V Ref
                                                // Vref = DVCC

    // Configure reference module
    PMMCTL0_H = PMMPW_H;                                      // Unlock the PMM registers
    PMMCTL2 = INTREFEN | REFVSEL_0;                           // Enable internal 1.5V reference
    while(!(PMMCTL2 & REFGENRDY));                            // Poll till internal reference settles

    while(1)
    {
        ADCCTL0 |= ADCENC | ADCSC;                            // Sampling and conversion start
        __bis_SR_register(LPM0_bits | GIE);                   // LPM0, ADC_ISR will force exit


 // To calculate DVCC, the following equation is used
        // DVCC = (4095 * 1.5) / adcResult
        // The following equation is modified to use only integers instead
        // of using float. All results needs to be divided by 100 to obtain
        // the final value.
        // DVCC = (4095 * 150) / adcResult
        // dvccValue = ((unsigned long)4095 * (unsigned long)150) / (unsigned long) (adcResult);
        // if (dvccValue < 250)                // DVCC < 2.50V?

// Power Selection
        if (ADC_Result < 0x555)                               // 0x555 represent 0.5V
            P1OUT &= ~BIT0 + BIT2;                                   // Clear P1.0 and P1.2 LED off
//            P1OUT |= BIT0;                                    // Set P1.2 LED on

            // Enable VBat1 
            // Enable VBat2
            // Disable ChrgEn1
            // Disable ChrgEn2

        else
            P1OUT |= BIT0 + BIT2;                                    // Set P1.0 and P1.2 LED on
//            P1OUT &= ~BIT0;                                   // Clear P1.2 LED off

            // Disable VBat1
            // Disable VBat2
            // Enable ChrgEn1 
            // Enable ChrgEn2

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
