#ifndef IRQ_H
#define IRQ_H

typedef void (*irqHandler)(void);

class IRQHandler {
private:
  static void DefaultVICHandler(void);
  static const int IRQ_SLOT_EN=(1 << 5); ///< bit 5 in Vector control register
  static const int VIC_SIZE	=16; ///<Number of VIC slots
public:
  static void begin( void );
  static bool install(unsigned int IntNumber, irqHandler HandlerAddr );
  static bool uninstall(unsigned int IntNumber );
  static void __attribute__ ((interrupt("IRQ"))) IRQ_Wrapper();

  static const int WDT		= 0; ///< Watchdog timer  
  static const int SWI		= 1; ///< Software interrupt, triggered by SWI x instruction
  static const int ARM_CORE0	= 2; ///< ARMCore0, used by EmbeddedICE RX
  static const int ARM_CORE1	= 3; ///< ARMCore1, Used by EmbeddedICE TX
  static const int TIMER0	= 4; ///< Timer 0 match or capture 
  static const int TIMER1	= 5; ///< Timer 1 match or capture
  static const int UART0	= 6; ///< UART 0 interrupt
  static const int UART1	= 7; ///< UART 1 interrupt
  static const int PWM0		= 8; ///< PWM match
  static const int I2C0		= 9; ///< I2C 0 interrupt
  static const int SPI0		=10; ///< SPI 0 interrupt
  static const int SPI1		=11; ///< SPI 1 interrupt
  static const int PLL		=12; ///< Phase lock loop in lock
  static const int RTC		=13; ///< Real-time clock increment or alarm
  static const int EINT0	=14; ///< External interrupt 0
  static const int EINT1	=15; ///< External interrupt 1
  static const int EINT2	=16; ///< External interrupt 2
  static const int EINT3	=17; ///< External interrupt 3
  static const int ADC0		=18; ///< Analog-to-Digital Converter 0 end of conversion
  static const int I2C1		=19; ///< I2C 1 interrupt
  static const int BOD		=20; ///< Brownout detected
  static const int ADC1		=21; ///< Analog-to-Digital Converter 1 end of conversion
  static const int USB		=22; ///< USB and DMA interrupt

};

#endif


