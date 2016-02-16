#define TMR0_BASE_ADDR 0xE0004000
#define TMR1_BASE_ADDR 0xE0008000

/** Timer Interrupt Register. TIR can be written to clear interrupts. TIR can be
read to identify which of eight possible interrupt sources are pending. */
rw1(time,TIR  ,1,TMR0_BASE_ADDR+(TMR1_BASE_ADDR-TMR0_BASE_ADDR)*i+0x00);

/** Timer Control Register. TCR is used to control the Timer Counter
functions. The Timer Counter can be disabled or reset through TCR. */
rw1(time,TTCR ,1,TMR0_BASE_ADDR+(TMR1_BASE_ADDR-TMR0_BASE_ADDR)*i+0x04);

/** Timer Counter. The 32-bit TTC is incremented every TPR+1 cycles of PCLK. The
TC is controlled through the TCR. If the timer is configured as a counter with
TCTCR, then TTC is incremented every TPR+1 count events instead.*/
rw1(time,TTC  ,1,TMR0_BASE_ADDR+(TMR1_BASE_ADDR-TMR0_BASE_ADDR)*i+0x08);

/** Prescale Register. When the Prescale Counter (TPC) is equal to this value,
 *the next clock increments the TC and clears the PC. */
rw1(time,TPR  ,1,TMR0_BASE_ADDR+(TMR1_BASE_ADDR-TMR0_BASE_ADDR)*i+0x0C);

/** Prescale Counter. The 32-bit TPC is a counter which is incremented to the
value stored in TPR. When the value in TPR is reached, the TTC is incremented
and the TPC is cleared. The TPC is observable and controllable through the bus
interface. The effect is that if TPC is zero, TTC counts at PCLK rate. If it
is 1, TTC counts every second PCLK tick. If it is 2, TTC counts every third
PCLK, and in general if it is N, TTC increments every N+1 ticks of PCLK. If the
timer is configured as a counter with TCTCR, then TTC is incremented every TPR+1
count events instead.*/
rw1(time,TPC  ,1,TMR0_BASE_ADDR+(TMR1_BASE_ADDR-TMR0_BASE_ADDR)*i+0x10);

/** Match Control Register. TMCR is used to control on a per-match-channel basis
if an interrupt is generated and if the TC is reset or stopped when a match
occurs */
rw1(time,TMCR ,1,TMR0_BASE_ADDR+(TMR1_BASE_ADDR-TMR0_BASE_ADDR)*i+0x14);

/** Match register. TMR can be enabled through TMCR to reset TTC, stop
both TTC and TPC, and/or generate an interrupt every time TMR matches
TTC. */
rw2(time,TMR  ,1,4,TMR0_BASE_ADDR+(TMR1_BASE_ADDR-TMR0_BASE_ADDR)*i+0x18+4*j);

/** Capture control register. TCCR controls on a per-capture-channel basis which
edge of the capture inputs are used to load TCR and whether or not an interrupt
is generated when a capture takes place. */
rw1(time,TCCR ,1,TMR0_BASE_ADDR+(TMR1_BASE_ADDR-TMR0_BASE_ADDR)*i+0x28);

/** Capture register. TCR(m,n) is loaded with the value of TTC when there is an
event on the CAPm.n input, where m is the timer used, 0 or 1, and n is the
capture channel, 0-3. Datasheet claims that this is read-only, but used as 
read/write in the code (to set to 0).*/
rw2(time,TCR  ,1,4,TMR0_BASE_ADDR+(TMR1_BASE_ADDR-TMR0_BASE_ADDR)*i+0x2C+4*j);

/** External Match Register. The EMR controls on a per-channel-basis the
external match pins MATm.n where m is the timer used, 0 or 1, and n is the
capture channel, 0-3. */
rw1(time,TEMR ,1,TMR0_BASE_ADDR+(TMR1_BASE_ADDR-TMR0_BASE_ADDR)*i+0x3C);

/** Count Control Register. The TCTCR selects between Timer and Counter mode,
and in Counter mode selects the signal and edge(s) for counting. */
rw1(time,TCTCR,1,TMR0_BASE_ADDR+(TMR1_BASE_ADDR-TMR0_BASE_ADDR)*i+0x70);

#undef TMR0_BASE_ADDR
#undef TMR1_BASE_ADDR
