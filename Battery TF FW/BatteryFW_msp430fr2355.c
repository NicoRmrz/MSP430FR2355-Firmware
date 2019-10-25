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
//           |             P3.0|--> LED 0
//       >---|             P3.1|--> LED 1
//       >---|             P3.2|--> LED 2
//       >---|             P3.3|--> LED 3
//       >---|             P3.4|--> LED 4
//       >---|             P3.5|--> LED 4
//
//
//  __________________________________________________________________________________*/
#include <msp430.h>

// Port 1 definitions
#define IOUT1   (BIT0)                      // P1.0 IOUT1 input
#define IOUT2   (BIT1)                      // P1.1 IOUT2 input
#define i2cData1  (BIT2)                      // P1.2 I2C Data 1 
#define i2cClk1   (BIT3)                      // P1.3 I2C Clock 1 

// Port 2 definitions
#define DM_12V_En   (BIT0)                      // P2.0 DM 12V Enable output
#define DM_VBat2_En (BIT1)                      // P2.1 DM Battery Voltage 2 Enable output
#define DM_VBat1_En (BIT2)                      // P2.2 DM Battery Voltage 1 output
#define nSWTurnOFFPower  (BIT3)                      // P2.3 SW Turn off power - active low output
#define nPWR_OFF_Int (BIT4)                      // P2.4 Power off intterrupt - active low input

// Port 3 definitions
#define LED_1   (BIT0)                      // P3.0 LED output
#define LED_2   (BIT1)                      // P3.1 LED output
#define LED_3   (BIT2)                      // P3.2 LED output
#define LED_4   (BIT3)                      // P3.3 LED output
#define LED_5   (BIT4)                      // P3.4 LED output
#define LED_6   (BIT5)                      // P3.5 LED output

// Port 4 definitions
#define VBAT1_OFF   (BIT4)                      // 4.4 Battery Voltage 1 Off output
#define VBAT2_OFF   (BIT5)                      // 4.5 Battery Voltage 2 Off output
#define i2cData2    (BIT6)                      // 4.6 I2C Data 2 
#define i2cClk2     (BIT7)                      // 4.7 I2C Clock 2 

// Port 5 definitions
#define ChrgEn1 (BIT0)                      // P5.0 Enable Battery 1 Charging output
#define ChrgEn2 (BIT1)                      // P5.1 Enable Battery 1 Charging output
#define TH1     (BIT2)                      // P5.2 TH1 intput
#define TH2     (BIT3)                      // P5.3 TH2 intput

// Port 6 definitions
#define DisChg1 (BIT0)                      // P6.0 LED output
#define DisChg2 (BIT1)                      // P6.1 LED output
#define n12VFlt (BIT2)                      // P6.2 12V Fault - active low input
#define nBat1Flt (BIT3)                      // P6.3 Battery 1 Fault - active low input
#define nBat2Flt (BIT4)                      // P6.4 Battery 2 Fault - active low input
#define ACOK2 (BIT5)                      // P6.5 ACOK2 input
#define ACOK1 (BIT6)                      // P6.6 ACOK1 input


// unsigned int ADC_Result;


int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                      // Stop WDT

    // Configure GPIO
    P3DIR |= LED_1 + LED_2 + LED_3 + LED_4 + LED_5 + LED_6       // Set P3.0 - P3.5 LEDs as outputs
    // P3OUT &= ~(LED_1 + LED_2 + LED_3 + LED_4 + LED_5 + LED_6);  // Set all LEDs off at start up

    P6DIR &= ~n12VFlt;      // Set P6.2 (n12VFlt) as Input
    P6REN = n12VFlt;        // Enable Pullup/down resistor for P6.2 (n12VFlt)
    P6OUT = n12VFlt;        // Select Pullup

    P4DIR |= VBAT1_OFF + VBAT2_OFF;     // Set P4.4(VBAT1_OFF) and P4.5(VBAT2_OFF) as outputs
    // P4OUT |= VBAT1_OFF + VBAT2_OFF;     //drive output HIGH initially

    P5DIR |= ChrgEn1 + ChrgEn2;     // Set P5.0(ChrgEn1) and P5.1(ChrgEn2) as outputs
    // P5OUT |= ChrgEn1 + ChrgEn2;     //drive output HIGH initially

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    // PM5CTL0 &= ~LOCKLPM5;

    while(1)
    {
        // ADCCTL0 |= ADCENC | ADCSC;                            // Sampling and conversion start
        // __bis_SR_register(LPM0_bits | GIE);                   // LPM0, ADC_ISR will force exit

        // Power Selection
        if( !(P6IN & n12VFlt) ) //Evaluates to True for a 'LOW' on P6.2(n12VFlt)
        {
            // Enable VBat
            P4OUT |= VBAT1_OFF + VBAT2_OFF;       // Set P4.4(VBAT1_OFF) and P4.5(VBAT2_OFF) HIGH
            // Disable Battery Charge
            P5OUT &= ~(ChrgEn1 + ChrgEn2);        // Set P5.0(ChrgEn1) and P5.1(ChrgEn2) LOW

            // Enable First 3 LEDs
            P4OUT |= LED_1 + LED_2 + LED_3;
        }
        else                    //P6.2(n12VFlt) is HIGH
        {
            // Disable VBat
            P4OUT &= ~(VBAT1_OFF + VBAT2_OFF);       // Set P4.4(VBAT1_OFF) and P4.5(VBAT2_OFF) LOW
            // Enable Battery Charge
            P5OUT |= ChrgEn1 + ChrgEn2;        // Set P5.0(ChrgEn1) and P5.1(ChrgEn2) HIGH
            // Disable First 3 LEDs
            P4OUT &= ~(LED_1 + LED_2 + LED_3);
        }

    }
}