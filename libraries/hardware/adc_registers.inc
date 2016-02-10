#define AD0_BASE_ADDR 0xE0034000
#define AD1_BASE_ADDR 0xE0060000


/** A/D Control register. The ADCR register must be written to select the
 *  operating mode before A/D conversion can occur. */
rw1(adc,ADCR   ,2,AD0_BASE_ADDR+(AD1_BASE_ADDR-AD0_BASE_ADDR)*i+0x00);

/** A/D Global Data Register. This register contains the ADC DONE bit
 *  and the result of the most recent A/D conversion. */
rw1(adc,ADGDR  ,2,AD0_BASE_ADDR+(AD1_BASE_ADDR-AD0_BASE_ADDR)*i+0x04);

/** A/D Status Register. This register contains the DONE and OVERRUN 
 *  flags for all of the A/D channels, as well as the A/D interrupt flag */
ro1(adc,ADSTAT ,2,AD0_BASE_ADDR+(AD1_BASE_ADDR-AD0_BASE_ADDR)*i+0x30);

/** A/D Global Start Register. This address can be written to start conversions
 *  in both A/D converters simultaneously. */
wo0(adc,ADGSR,AD0_BASE_ADDR+0x08);

/** A/D Interrupt Enable Register. This register contains enable bits that allow
 *  the DONE flag of each A/D channel to be included or excluded from 
 *  contributing to the generation of an A/D interrupt. */
rw1(adc,ADINTEN,2,AD0_BASE_ADDR+(AD1_BASE_ADDR-AD0_BASE_ADDR)*i+0x0C);

/** A/D Data Register. This register contains the result of the most
 *  recent conversion completed on this channel. */
///*RO */static inline volatile unsigned int ADDR   (int adc, int channel) {return AbsRefBlock(AD0_BASE_ADDR,AD1_BASE_ADDR,adc,4,channel,0x10);}
ro2(adc,ADDR   ,2,8,AD0_BASE_ADDR+(AD1_BASE_ADDR-AD0_BASE_ADDR)*i+0x10+4*j);

#undef AD0_BASE_ADDR
#undef AD1_BASE_ADDR
