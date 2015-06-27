#include "agc_engine.h"

/*I expended considerable effort on making these methods of agc_t. While it
 *CAN* be done, it is not as clean and elegant as it should be.

 The C way to do it is to create a bunch of static functions and put those
 functions into an array of function pointers. Each function would have the
 same signature, and would be passed an agc_t (perhaps by pointer).

 You can't do it like this in Java, since there are no such things as
 pointers. The Java way to do it would be to define an interface, create an
 implementation of the interface for each instruction, then put those
 instances in an array.

 In C++ there are pointers, so I thought it would be easy to make an array
 of function pointers, use the addresses of the methods as those pointers,
 and do it basically the C way. The Java way is possible, but it requires
 virtual inheritance, something I would like to avoid due to overhead.
 There IS a way to do it in C++, but it is very difficult and akward, and
 generates basically write-only code. The problem is that methods always
 have a hidden this parameter which must be handled internally. Also C++ is
 not English, and while this concept is relatively easy to describe in
 English, it is VERY akward in C++. As a result, The C++ implementation of
 pointers to methods has roughly the same order of magnitude of complexity
 as the virtual inheritance method. For instance, each element of the
 opcode table takes 8 bytes, while function pointers normally only require
 4.

 As C++ is basically a superset of C, you can do things the C way if you
 abandon the notion of making the instructions methods of agc_t, and just
 pass the appropriate agc_t to an otherwise void non-member function.
 After writing it both ways, I have elected to keep things as void non-member
 functions.
*/

/** Implements the TC, INDEX, RELINT, and EXTEND instructions.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void TC(agc_t& State) {
  // TC instruction (1 MCT).
  State.ValueK = State.Address12;	// Convert AGC numerical format to native CPU format.
  if (State.ValueK == 3)		// RELINT instruction.
    State.AllowInterrupt = 1;
  else if (State.ValueK == 4)	// INHINT instruction.
    State.AllowInterrupt = 0;
  else if (State.ValueK == 6) {	// EXTEND instruction.
    State.ExtraCode = 1;
    // Normally, ExtraCode will be reset when agc_engine is finished.
    // We inhibit that behavior with this flag.
    State.KeepExtraCode = 1;
  } else {
    State.BacktraceAdd(0);
    if (State.ValueK != agc_t::RegQ)	// If not a RETURN instruction ...
    State.Q() = 0177777 & State.NextZ;
    State.NextZ = State.Address12;
  }
}

/** Implements the CCS instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction

The "Count, Compare, and Skip"
instruction stores a variable from erasable memory into the accumulator
(which is decremented), and then performs
one of several jumps based on the original value of the variable.&nbsp;
This is the only "compare" instruction in the AGC instruction set.
<table>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Syntax:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">CCS K</span></td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Operand:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">K </span>is
the label of a memory location.&nbsp; It must assemble to a 10-bit
memory
address in erasable memory.<br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Extracode:</td>
      <td style="vertical-align: top;">This is not an extracode, and
therefore cannot be preceded by an <span style="font-family: monospace;">EXTEND</span> instruction.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Timing:<br>
      </td>
      <td style="vertical-align: top;">2 MCT (about 23.4 &mu;s)<br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; width: 20%; font-weight: bold;">Flags:<br>
      </td>
      <td style="vertical-align: top;">The Overflow is set according to
the result of the operation.&nbsp; The Extracode flag remains
cleared.<br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Editing:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-weight: bold;"></span>The
contents of <span style="font-family: monospace;">K</span> is edited
if <span style="font-family: monospace;">K</span> is one of the
special
registers CYR, SR, CYL, or EDOP.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Octal:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">10000 + K</span><br>
      </td>
    </tr>
    </table>
Notes:

The operation of this instruction is rather complex:

1. The "Diminished ABSolute value" of the contents of memory-location `K` is
loaded into the A register. The diminished absolute value is defined as 
DABS(x)=|x|-1 if |x|&gt;1, or +0 otherwise. (If `K` is a 16-bit register like A,
L, or Q, then its contents may contain + or - overflow; overflow correction is 
<span style="font-style: italic;">not</span> performed prior to the
operation.)
2. After computing the contents of the accumulator, the
contents of `K` is "edited", if `K` is one of
the registers CYR, SR, CYL, or EDOP, but is otherwise unchanged from
its original value.
3. A jump is performed, depending on the <span style="font-style: italic;">original</span> (unedited) contents of `K`: If
greater than +0, execution continues at the next instruction after the `CCS`. If equal to +0,
execution continues at the 2<sup>nd</sup> instruction after the `CCS`. If less than -0,
execution continues at the 3<sup>rd</sup> instruction after the `CCS`. If equal to -0,
execution continues at the 4<sup>th</sup> instruction after the `CCS`. 
(If `K` is 16 bits, then the original contents may contain + or - overflow; in
this case, the value is treated as + or - <span style="font-style: italic;">non-zero</span>, even if the
sign-corrected value would have been 0.)

A typical use of this instruction would be for loop control, with `CCS A`.

Note that the net effect of the way overflow is treated when `K` is A, L, or Q is to allow 15-bit
loop counters rather than mere 14-bit loop counters. For example,
if A contains +1 with +overflow, then `CCS A` will place +0 with
+overflow into A, and another `CCS
A` will place 037777 without overflow into A, and thus no anomaly
is seen when decrementing from +overflow to no overflow.

The overflow of the accumulator will generally be cleared by this
operation except in the kinds of cases decribed in the preceding
paragraph.
*/
void CCS(agc_t& State) {
  // CCS instruction (2 MCT).
  // Figure out where the data is stored, and fetch it.
  int16_t Operand16;
  if (State.Address10 < agc_t::REG16) {
    Operand16 = OverflowCorrected (0177777 & State.Erasable[0][State.Address10]);
    State.A() = agc_t::odabs (0177777 & State.Erasable[0][State.Address10]);
  } else {			// K!=accumulator.
    State.WhereWord = State.FindMemoryWord(State.Address10);
    Operand16 = *State.WhereWord & 077777;
    // Compute the "diminished absolute value", and save in accumulator.
    State.A() = agc_t::dabs (Operand16);
  }
  State.AssignFromPointer(State.WhereWord, Operand16);
  // Now perform the actual comparison and jump on the basis
  // of it.  There's no explanation I can find as to what
  // happens if we're already at the end of the memory bank,
  // so I'll just pretend that that can't happen.  Note,
  // by the way, that if the Operand is > +0, then NextZ
  // is already correct, and in the other cases we need to
  // increment it by 2 less because NextZ has already been
  // incremented.
  if (State.Address10 < agc_t::REG16 && ValueOverflowed (0177777 & State.Erasable[0][State.Address10]) == agc_t::agc_t::AGC_P1)
    State.NextZ += 0;
  else if (State.Address10 < agc_t::REG16 && ValueOverflowed (0177777 & State.Erasable[0][State.Address10]) == agc_t::agc_t::AGC_M1)
    State.NextZ += 2;
  else if (Operand16 == agc_t::agc_t::AGC_P0)
    State.NextZ += 1;
  else if (Operand16 == agc_t::agc_t::AGC_M0)
    State.NextZ += 3;
  else if (0 != (Operand16 & 040000))
    State.NextZ += 2;
}

/** Implements the TCF instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void TCF(agc_t& State) {
      State.BacktraceAdd(0);
      // TCF instruction (1 MCT).
      State.NextZ = State.Address12;
      // THAT was easy ... too easy ...
}

/** Implements the DAS instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction

The "Double Add to Storage"
instruction does a double-precision (DP) add of the A,L register pair
to a pair of variables in erasable memory. <br>
<table>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Syntax:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">DAS K</span></td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Operand:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">K </span>is the label of a memory
location.&nbsp; It must assemble to a 10-bit memory address in erasable
memory.&nbsp; The location <span style="font-family: monospace;">K</span>
contains the more-significant word of a pair of variables containing a
DP value, while <span style="font-family: monospace;">K+1</span>
contains the less-significant word.<span style="font-family: monospace;"></span><br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Extracode:</td>
      <td style="vertical-align: top;">This is not an extracode, and
therefore cannot be preceded by an <span style="font-family: monospace;">EXTEND</span> instruction.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Timing:<br>
      </td>
      <td style="vertical-align: top;">3 MCT (about 35.1 &mu;s)<br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; width: 20%; font-weight: bold;">Flags:<br>
      </td>
      <td style="vertical-align: top;">The Overflow is cleared.&nbsp;
The Extracode flag remains clear.&nbsp; <br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Editing:<br>
      </td>
      <td style="vertical-align: top;">Editing is done if the <span style="font-family: monospace;">K,K+1</span>
variable pair overlaps the CYR, SR, CYL, and EDOP
registers.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Octal:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">20001 + K</span><br>
      </td>
    </tr>
    </table>
Notes:

A variant on this instruction is
the case `DAS A`. Refer to the <span style="font-family: monospace;">DDOUBL</span>
instruction for an explanation of this case.

Prior to the instruction, the A,L register pair and the `K,K+1` pair each contain a
double precision (DP) value, with the more-significant word first and
the less-significant word second. The signs of the contents of A and L
need not
agree, nor need the signs of `K`
and `K+1`

16-bit values (the A, L, and Q
registers) are not overflow-corrected prior to the addition. The
words of the sum are overflow-corrected when saved to 15-bit registers
but not when saved to 16-bit registers.

The two DP values are added together, and the result is stored back in
the `K,K+1` pair. The signs of the
resulting words <span style="font-style: italic;">need not</span>
agree; the sign of the less significant word is the same as the sign
from an SP addition of the less-significant words. Any overflow
or underflow from addition of the less-significant words rolls over
into the addition of the more-significant words.

If either of `K` or `K+1` are editing registers (CYR,
SR, CYL, or EDOP), then the appropriate editing occurs when `K,K+1` are written.

Note that the normal result of AGC arithmetic such as (+1)+(-1) is -0.

After the addition, the L register is set to +0, and the A register is
set to +1, -1, or +0, depending on whether there had been positive
overflow, negative overflow, or no overflow during the addition.
*/
void DAS(agc_t& State) {
  // We add the less-significant words (as SP values), and thus
  // the sign of the lower word of the output does not necessarily
  // match the sign of the upper word.
  int Msw, Lsw;
  if (State.IsL(State.Address10)) {	// DDOUBL
    Lsw = AddSP16 (0177777 & State.L(), 0177777 & State.L());
    Msw = AddSP16 (State.Accumulator, State.Accumulator);
    if ((0140000 & Lsw) == 0040000)
      Msw = AddSP16 (Msw, agc_t::AGC_P1);
    else if ((0140000 & Lsw) == 0100000)
      Msw = AddSP16 (Msw, SignExtend (agc_t::AGC_M1));
    Lsw = OverflowCorrected (Lsw);
    State.A() = 0177777 & Msw;
    State.L() = 0177777 & SignExtend (Lsw);
    return;
  }
  State.WhereWord = State.FindMemoryWord(State.Address10);
  if (State.Address10 < agc_t::REG16)
    Lsw = AddSP16 (0177777 & State.L(), 0177777 & State.Erasable[0][State.Address10]);
  else
    Lsw = AddSP16 (0177777 & State.L(), SignExtend (*State.WhereWord));
  if (State.Address10 < agc_t::REG16 + 1)
    Msw = AddSP16 (State.Accumulator, 0177777 & State.Erasable[0][State.Address10 - 1]);
  else
    Msw = AddSP16 (State.Accumulator, SignExtend (State.WhereWord[-1]));

  if ((0140000 & Lsw) == 0040000)
    Msw = AddSP16 (Msw, agc_t::AGC_P1);
  else if ((0140000 & Lsw) == 0100000)
    Msw = AddSP16 (Msw, SignExtend (agc_t::AGC_M1));
  Lsw = OverflowCorrected (Lsw);

  if ((0140000 & Msw) == 0100000)
    State.A() = SignExtend (agc_t::AGC_M1);
  else if ((0140000 & Msw) == 0040000)
    State.A() = agc_t::AGC_P1;
  else
    State.A() = agc_t::AGC_P0;
  State.L() = agc_t::AGC_P0;
  // Save the results.
  if (State.Address10 < agc_t::REG16)
    State.Erasable[0][State.Address10] = SignExtend (Lsw);
  else
    State.AssignFromPointer(State.WhereWord, Lsw);
  if (State.Address10 < agc_t::REG16 + 1)
    State.Erasable[0][State.Address10 - 1] = Msw;
  else
    State.AssignFromPointer(State.WhereWord - 1, OverflowCorrected (Msw));
}

/** Implements the LXCH instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void LXCH(agc_t& State) {
      // "LXCH K" instruction (2 MCT).
      int16_t Operand16;
      if (State.IsL(State.Address10))
	return;
      if (State.IsReg(State.Address10, agc_t::RegZERO))	// ZL
	State.L() = agc_t::AGC_P0;
      else if (State.Address10 < agc_t::REG16)
	{
	  Operand16 = State.L();
	  State.L() = State.Erasable[0][State.Address10];
	  if (State.Address10 >= 020 && State.Address10 <= 023)
	    State.AssignFromPointer(State.WhereWord,
			       OverflowCorrected (0177777 & Operand16));
	  else
	    State.Erasable[0][State.Address10] = Operand16;
	  if (State.Address10 == agc_t::RegZ)
	    State.NextZ = State.Z();
	}
      else
	{
	  State.WhereWord = State.FindMemoryWord(State.Address10);
	  Operand16 = *State.WhereWord;
	  State.AssignFromPointer(State.WhereWord,
			     OverflowCorrected (0177777 & State.L()));
	  State.L() = SignExtend (Operand16);
	}
}
/** Implements the INCR instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void INCR(agc_t& State) {
      // INCR instruction (2 MCT).
      {
	int Sum;
	State.WhereWord = State.FindMemoryWord(State.Address10);
	if (State.Address10 < agc_t::REG16)
	  State.Erasable[0][State.Address10] = AddSP16 (agc_t::AGC_P1, 0177777 & State.Erasable[0][State.Address10]);
	else
	  {
	    Sum = AddSP16 (agc_t::AGC_P1, SignExtend (*State.WhereWord));
	    State.AssignFromPointer(State.WhereWord, OverflowCorrected (Sum));
	    State.InterruptRequest(State.Address10, Sum);
	  }
      }
}
/**
Implements the ADS instruction
  @param State AGC instance to operate on
  @param ins Decoded instruction

The "Add to Storage" instruction adds the accumulator to an erasable-memory location
<table>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Syntax:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">ADS K</span></td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Operand:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">K </span>is the label of a memory
location.&nbsp; It must assemble to a 10-bit memory address in erasable
memory.&nbsp; <span style="font-family: monospace;"></span><br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Extracode:</td>
      <td style="vertical-align: top;">This is not an extracode, and
therefore cannot be preceded by an <span style="font-family: monospace;">EXTEND</span> instruction.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Timing:<br>
      </td>
      <td style="vertical-align: top;">2 MCT (about 23.4 &mu;s)<br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; width: 20%; font-weight: bold;">Flags:<br>
      </td>
      <td style="vertical-align: top;">The Overflow is set according to the result of the addition. The Extracode flag remains clear.
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Editing:<br>
      </td>
      <td style="vertical-align: top;">Editing is done upon writing to <span style="font-family: monospace;">K</span>, if <span style="font-family: monospace;">K</span> is CYR, SR, CYL, or EDOP.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Octal:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">26000 + K</span><br>
      </td>
    </tr>
    </table>

Notes:

The contents of the accumulator and `K` are added together, and the
result is stored both in the accumulator and in `K`. The accumulator is
neither overflow-corrected prior to the addition nor after it. However, the sum is 
overflow-corrected prior to being saved at 
`K` if `K` is a 15-bit register. If `K` is a 16-bit register like `L` or `Q`, then the sum is not overflow corrected before
storage.

Note that the normal result of AGC arithmetic such as (+1)+(-1) is -0.

If the destination register is 16-bits (L or Q register), then the non-overflow-corrected values added.
*/
void ADS(agc_t& State) {
      // Reviewed against Blair-Smith.
      // (2 MCT).
      {
	State.WhereWord = State.FindMemoryWord(State.Address10);
	if (State.IsA(State.Address10))
	  State.Accumulator = AddSP16 (State.Accumulator, State.Accumulator);
	else if (State.Address10 < agc_t::REG16)
	  State.Accumulator = AddSP16 (State.Accumulator, 0177777 & State.Erasable[0][State.Address10]);
	else
	  State.Accumulator = AddSP16 (State.Accumulator, SignExtend (*State.WhereWord));
	State.A() = State.Accumulator;
	if (State.IsA(State.Address10))
	  ; //Intentionally do nothing
	else if (State.Address10 < agc_t::REG16)
	  State.Erasable[0][State.Address10] = State.Accumulator;
	else
	  State.AssignFromPointer(State.WhereWord,
			     OverflowCorrected (State.Accumulator));
      }
}
/** Implement the CA instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction

The "Clear and Add" (or "Clear and Add Erasable" or "Clear and Add Fixed") instruction
moves the contents of a memory location into the accumulator.<br>
<table>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Syntax:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">CA K<br>
      </span><span style="font-style: italic;">or</span><span style="font-family: monospace;"><br>
CAE K<br>
      </span><span style="font-style: italic;">or</span><span style="font-family: monospace;"><br>
CAF K<br>
      </span></td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Operand:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">K </span>is the label of a memory
location.&nbsp; It must assemble to a 12-bit memory address.&nbsp; The <span style="font-family: monospace;">CAE</span> or <span style="font-family: monospace;">CAF</span> variants differ from the
generic <span style="font-family: monospace;">CA</span>, only in that
the assembler is supposed to display error messages if <span style="font-family: monospace;">K</span> is not in erasable or fixed
memory, respectively.<span style="font-family: monospace;"></span><br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Extracode:</td>
      <td style="vertical-align: top;">This is not an extracode, and
therefore cannot be preceded by an <span style="font-family: monospace;">EXTEND</span> instruction.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Timing:<br>
      </td>
      <td style="vertical-align: top;">2 MCT (about 23.4 &mu;s)<br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; width: 20%; font-weight: bold;">Flags:<br>
      </td>
      <td style="vertical-align: top;">The Overflow is cleared, unless <span style="font-family: monospace;">K</span> is the accumulator or the Q
register.&nbsp;
The Extracode flag remains clear.&nbsp; <br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Editing:<br>
      </td>
      <td style="vertical-align: top;">Editing is done upon writing to <span style="font-family: monospace;">K</span>, if <span style="font-family: monospace;">K</span> is CYR, SR, CYL, or EDOP.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Octal:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">30000 + K</span><br>
      </td>
    </tr>
    </table>

Notes:

A side-effect of this instruction is that `K` is rewritten after its value
is written to the accumulator; this means that if `K` is CYR, SR, CYL, or EDOP,
then it is re-edited.

Note that if the source register contains 16-bits (like the L or Q register),
then all 16 bits will be transferred to the accumulator, and thus the
overflow will be transferred into A. On the other hand, if
the source register is 15 bits, then it will be sign-extended to 16
bits when placed in A.

For the special case `CA A`,
refer instead to the `NOOP`
instruction.
*/
void CA(agc_t& State) {
      if (State.IsA(State.Address12))	// NOOP
	return;
      if (State.Address12 < agc_t::REG16)
	{
	  State.A() = State.Erasable[0][State.Address12];;
	  return;
	}
      State.WhereWord = State.FindMemoryWord(State.Address12);
      State.A() = SignExtend (*State.WhereWord);
      State.AssignFromPointer(State.WhereWord, *State.WhereWord);
}

/** Implements the CS and COM instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction

The "Clear and Subtract"
instruction moves the 1's-complement (i.e., the negative) of a memory
location into the accumulator.<br>
<table>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Syntax:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">CS K</span><span style="font-family: monospace;"><br>
      </span></td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Operand:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">K </span>is the label of a memory
location.&nbsp; It must assemble to a 12-bit memory address. <span style="font-family: monospace;"></span><br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Extracode:</td>
      <td style="vertical-align: top;">This is not an extracode, and
therefore cannot be preceded by an <span style="font-family: monospace;">EXTEND</span> instruction.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Timing:<br>
      </td>
      <td style="vertical-align: top;">2 MCT (about 23.4 &mu;s)<br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; width: 20%; font-weight: bold;">Flags:<br>
      </td>
      <td style="vertical-align: top;">The Overflow is cleared, unless <span style="font-family: monospace;">K</span> is the accumulator.&nbsp;
The Extracode flag remains clear.&nbsp; <br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Editing:<br>
      </td>
      <td style="vertical-align: top;">Editing is done upon writing to <span style="font-family: monospace;">K</span>, if <span style="font-family: monospace;">K</span> is CYR, SR, CYL, or EDOP.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Octal:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">40000 + K</span><br>
      </td>
    </tr>
    </table>
Notes:

A side-effect of this instruction is that <span style="font-family: monospace;">K</span> is rewritten with its
original value after the accumulator is written; this means that if <span style="font-family: monospace;">K</span> is CYR, SR, CYL, or EDOP,
then it is re-edited.

Note that if the source register contains 16 bits (the L or Q
register),
then all 16 bits will be complemented and transferred to the
accumulator, and thus the
overflow in the source register will be inverted and transferred into
A. (For example, +overflow in Q will turn into -overflow in A.)&nbsp; On the
other hand, if the
source register is 15 bits, then it will be complemented and
sign-extended to 16 bits when placed in A.

For the special case `CS A`, refer instead to the `COM`instruction.

The "Complement the Contents of A" bitwise complements the accumulator<br>
<table>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Syntax:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">COM</span><span style="font-family: monospace;"><br>
      </span></td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Operand:<br>
      </td>
      <td style="vertical-align: top;">This instruction has no operand.<span style="font-family: monospace;"></span><br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Extracode:</td>
      <td style="vertical-align: top;">This is not an extracode, and
therefore cannot be preceded by an <span style="font-family: monospace;">EXTEND</span>
instruction.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Timing:<br>
      </td>
      <td style="vertical-align: top;">2 MCT (about 23.4 &mu;s). </td>
    </tr>
    <tr>
      <td style="vertical-align: top; width: 20%; font-weight: bold;">Flags:<br>
      </td>
      <td style="vertical-align: top;">The Overflow is unaffected. <span style="color: rgb(0, 153, 0);"></span>
The Extracode flag remains clear.&nbsp; <br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Editing:<br>
      </td>
      <td style="vertical-align: top;">The editing registers CYR, SR,
CYL, or EDOP are unaffected.<br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Octal:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;"><span style="font-family: monospace;">40000</span></span><br>
      </td>
    </tr>
    </table>
Notes:

All 16 bits of the accumulator are complemented. Therefore, in addition to negating the contents
of the register (i.e., converting plus to minus and minus to plus), the overflow is preserved.

This instruction assembles as `CS A`
*/
void CS(agc_t& State) {
	// CS
      if (State.IsA(State.Address12))	// COM
	{
	  State.A() = ~State.Accumulator;;
	  return;
	}
      if (State.Address12 < agc_t::REG16)
	{
	  State.A() = ~State.Erasable[0][State.Address12];
	  return;
	}
      State.WhereWord = State.FindMemoryWord(State.Address12);
      State.A() = SignExtend (NegateSP (*State.WhereWord));
      State.AssignFromPointer(State.WhereWord, *State.WhereWord);
}

/** Implements the INDEX instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void INDEX(agc_t& State) {
  if(State.ExtendedOpcode<0100) {

// INDEX
    if (State.Address10 == 017) goto Resume;
    if (State.Address10 < agc_t::REG16) {
      State.IndexValue = OverflowCorrected (State.Erasable[0][State.Address10]);
    } else {
      State.WhereWord = State.FindMemoryWord(State.Address10);
      State.IndexValue = *State.WhereWord;
    }
    return;
  } else {
    // INDEX (continued)
    if (State.Address12 == 017 << 1) {
Resume:
      if (State.InIsr) State.BacktraceAdd(255); else State.BacktraceAdd(0);
      State.NextZ = State.ZRUPT();
      State.InIsr = 0;
#ifdef ALLOW_BSUB
      State.SubstituteInstruction = 1;
#endif
    } else {
      if (State.Address12 < agc_t::REG16) State.IndexValue = OverflowCorrected (State.Erasable[0][State.Address12]); else {
        State.WhereWord = State.FindMemoryWord(State.Address12);
        State.IndexValue = *State.WhereWord;
      }
      State.KeepExtraCode = 1;
    }
  }
}

/** Implements the DXCH instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void DXCH(agc_t& State) {
      // Remember, in the following comparisons, that the address is pre-incremented.
    int16_t Operand16;
      if (State.IsL(State.Address10))
	{
	  State.L() = SignExtend (OverflowCorrected (State.L()));
	  return;
	}
      State.WhereWord = State.FindMemoryWord(State.Address10);
      // Topmost word.
      if (State.Address10 < agc_t::REG16)
	{
	  Operand16 = State.Erasable[0][State.Address10];
	  State.Erasable[0][State.Address10] = State.L();
	  State.L() = Operand16;
	  if (State.Address10 == agc_t::RegZ)
	    State.NextZ = State.Z();
	}
      else
	{
	  Operand16 = SignExtend (*State.WhereWord);
	  State.AssignFromPointer(State.WhereWord, OverflowCorrected (State.L()));
	  State.L() = Operand16;
	}
      State.L() = SignExtend (OverflowCorrected (State.L()));
      // Bottom word.
      if (State.Address10 < agc_t::REG16 + 1)
	{
	  Operand16 = State.Erasable[0][State.Address10 - 1];
	  State.Erasable[0][State.Address10 - 1] = State.A();
	  State.A() = Operand16;
	  if (State.Address10 == agc_t::RegZ + 1)
	    State.NextZ = State.Z();
	}
      else
	{
	  Operand16 = SignExtend (State.WhereWord[-1]);
	  State.AssignFromPointer(State.WhereWord - 1,
			     OverflowCorrected (State.A()));
	  State.A() = Operand16;
	}
}

/** Implements the TS instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void TS(agc_t& State) {
	// TS
      if (State.IsA(State.Address10))	// OVSK
	{
	  if (State.Overflow)
	    State.NextZ += agc_t::AGC_P1;
	}
      else if (State.IsZ(State.Address10))	// TCAA
	{
	  State.NextZ = (077777 & State.Accumulator);
	  if (State.Overflow)
	    State.A() = SignExtend (ValueOverflowed (State.Accumulator));
	}
      else			// Not OVSK or TCAA.
	{
	  State.WhereWord = State.FindMemoryWord(State.Address10);
	  if (State.Address10 < agc_t::REG16)
	    State.Erasable[0][State.Address10] = State.Accumulator;
	  else
	    State.AssignFromPointer(State.WhereWord,
			       OverflowCorrected (State.Accumulator));
	  if (State.Overflow)
	    {
	      State.A() = SignExtend (ValueOverflowed (State.Accumulator));
	      State.NextZ += agc_t::AGC_P1;
	    }
	}
}

/** Implements the XCH instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void XCH(agc_t& State) {
      if (State.IsA(State.Address10))
	return;
      if (State.Address10 < agc_t::REG16)
	{
	  State.A() = State.Erasable[0][State.Address10];
	  State.Erasable[0][State.Address10] = State.Accumulator;
	  if (State.Address10 == agc_t::RegZ)
	    State.NextZ = State.Z();
	  return;
	}
      State.WhereWord = State.FindMemoryWord(State.Address10);
      State.A() = SignExtend (*State.WhereWord);
      State.AssignFromPointer(State.WhereWord, OverflowCorrected (State.Accumulator));
}


/**
Implements the AD and DOUBLE instructions
  @param State AGC instance to operate on
  @param ins Decoded instruction

The "Add" instruction adds the contents of a memory location into the accumulator.
<table>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Syntax:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">AD K</span><span style="font-family: monospace;"><br>
      </span></td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Operand:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">K </span>is the label of a memory
location.&nbsp; It must assemble to a 12-bit memory address. <span style="font-family: monospace;"></span><br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Extracode:</td>
      <td style="vertical-align: top;">This is not an extracode, and
therefore cannot be preceded by an <span style="font-family: monospace;">EXTEND</span> instruction.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Timing:<br>
      </td>
      <td style="vertical-align: top;">2 MCT (about 23.4 &mu;s)<br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; width: 20%; font-weight: bold;">Flags:<br>
      </td>
      <td style="vertical-align: top;">The Overflow depends on the
result of the operation, and can be positive, negative, or none.&nbsp;
The Extracode flag remains clear.&nbsp; <br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Editing:<br>
      </td>
      <td style="vertical-align: top;">Editing is done upon writing to <span style="font-family: monospace;">K</span>, if <span style="font-family: monospace;">K</span> is CYR, SR, CYL, or EDOP.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Octal:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">60000 + K</span><br>
      </td>
    </tr>
</table>
Notes
: The accumulator is not overflow-corrected prior to the addition.  The contents of K are added to the accumulator, which retains any overflow that resulted from the addition.
A side-effect of this instruction is that K is rewritten after its value is written to the accumulator; this means that if K is CYR, SR, CYL, or EDOP, then it is re-edited.

Note that the normal result of AGC arithmetic such as (+1)+(-1) is -0.

For the special case "AD A", refer instead to the DOUBLE instruction.

*/
void AD(agc_t& State) {
      if (State.IsA(State.Address12))	// DOUBLE
	State.Accumulator = AddSP16 (State.Accumulator, State.Accumulator);
      else if (State.Address12 < agc_t::REG16)
	State.Accumulator = AddSP16 (State.Accumulator, 0177777 & State.Erasable[0][State.Address12]);
      else
	{
	  State.WhereWord = State.FindMemoryWord(State.Address12);
	  State.Accumulator = AddSP16 (State.Accumulator, SignExtend (*State.WhereWord));
	  State.AssignFromPointer(State.WhereWord, *State.WhereWord);
	}
      State.A() = State.Accumulator;
}

/** Implements the MASK instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void MASK(agc_t& State) {
		// MASK
      if (State.Address12 < agc_t::REG16)
	State.A() = State.Accumulator & State.Erasable[0][State.Address12];
      else
	{
	  State.A() = OverflowCorrected (State.Accumulator);
	  State.WhereWord = State.FindMemoryWord(State.Address12);
	  State.A() = SignExtend (State.A() & *State.WhereWord);
	}
}

/** Implements the READ instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void READ(agc_t& State) {
      if (State.IsL(State.Address9) || State.IsQ(State.Address9))
	State.A() = State.Erasable[0][State.Address9];
      else
	State.A() = SignExtend (State.CpuReadIO(State.Address9));
}

/** Implements the WRITE instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void WRITE(agc_t& State) {
      if (State.IsL(State.Address9) || State.IsQ(State.Address9))
	State.Erasable[0][State.Address9] = State.Accumulator;
      else
	State.CpuWriteIO(State.Address9, OverflowCorrected (State.Accumulator));
}

/** Implements the RAND instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void RAND(agc_t& State) {
      if (State.IsL(State.Address9) || State.IsQ(State.Address9))
	State.A() = (State.Accumulator & State.Erasable[0][State.Address9]);
      else
	{
	  int16_t Operand16 = OverflowCorrected (State.Accumulator);
	  Operand16 &= State.CpuReadIO(State.Address9);
	  State.A() = SignExtend (Operand16);
	}
}

/** Implements the WAND instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void WAND(agc_t& State) {
      if (State.IsL(State.Address9) || State.IsQ(State.Address9))
	State.A() = State.Erasable[0][State.Address9] = (State.Accumulator & State.Erasable[0][State.Address9]);
      else
	{
	  int16_t Operand16 = OverflowCorrected (State.Accumulator);
	  Operand16 &= State.CpuReadIO(State.Address9);
	  State.CpuWriteIO(State.Address9, Operand16);
	  State.A() = SignExtend (Operand16);
	}
}

/** Implements the ROR instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void ROR(agc_t& State) {
      if (State.IsL(State.Address9) || State.IsQ(State.Address9))
	State.A() = (State.Accumulator | State.Erasable[0][State.Address9]);
      else
	{
	  int16_t Operand16 = OverflowCorrected (State.Accumulator);
	  Operand16 |= State.CpuReadIO(State.Address9);
	  State.A() = SignExtend (Operand16);
	}
}

/** Implements the WOR instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void WOR(agc_t& State) {
      if (State.IsL(State.Address9) || State.IsQ(State.Address9))
	State.A() = State.Erasable[0][State.Address9] = (State.Accumulator | State.Erasable[0][State.Address9]);
      else
	{
	  int16_t Operand16 = OverflowCorrected (State.Accumulator);
	  Operand16 |= State.CpuReadIO(State.Address9);
	  State.CpuWriteIO(State.Address9, Operand16);
	  State.A() = SignExtend (Operand16);
	}
}

/** Implements the RXOR instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void RXOR(agc_t& State) {
      if (State.IsL(State.Address9) || State.IsQ(State.Address9))
	State.A() = (State.Accumulator ^ State.Erasable[0][State.Address9]);
      else
	{
	  int16_t Operand16 = OverflowCorrected (State.Accumulator);
	  Operand16 ^= State.CpuReadIO(State.Address9);
	  State.A() = SignExtend (Operand16);
	}
}

/** Implements the EDRUPT instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void EDRUPT(agc_t& State) {
      //State.InIsr = 0;
      //State.SubstituteInstruction = 1;
      //if (State.InIsr)
      //  State.InterruptRequests[State.InterruptRequests[0]] = 0;
      State.ZRUPT() = State.Z();
      State.InIsr = 1;
      State.BacktraceAdd(0);
#if 0
      if (State.InIsr)
        {
	  int Count = 0;
	  printf ("EDRUPT w/ ISR %d\n", ++Count);
	}
      else
        {
	  int Count = 0;
	  printf ("EDRUPT w/o ISR %d\n", ++Count);
	}
#endif // 0
      State.NextZ = 0;
}

/** Implements the DV instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void DV(agc_t& State) {
      {
	int16_t AccPair[2], AbsA, AbsL, AbsK, Div16, Operand16;
	int Dividend, Divisor, Quotient, Remainder;
	if (State.IsA(State.Address10))
	  {
	    Div16 = OverflowCorrected (State.Accumulator);
	    State.WhereWord = &Div16;
	  }
	else if (State.Address10 < agc_t::REG16)
	  {
	    Div16 = OverflowCorrected (State.Erasable[0][State.Address10]);
	    State.WhereWord = &Div16;
	  }
	else
	  State.WhereWord = State.FindMemoryWord(State.Address10);
	// Fetch the values;
	AccPair[0] = OverflowCorrected (State.Accumulator);
	AccPair[1] = State.L();
	Dividend = SpToDecent (&AccPair[1]);
	DecentToSp (Dividend, &AccPair[1]);
	// Check boundary conditions.
	AbsA = AbsSP (AccPair[0]);
	AbsL = AbsSP (AccPair[1]);
	AbsK = AbsSP (*State.WhereWord);
	if (AbsA > AbsK || (AbsA == AbsK && AbsL != agc_t::AGC_P0))
	  {
	    //printf ("Acc=%06o L=%06o\n", State.Accumulator, State.L());
	    //printf ("A,K,L=%06o,%06o,%06o abs=%06o,%06o,%06o\n",
	    //  AccPair[0],*State.WhereWord,AccPair[1],AbsA,AbsK,AbsL);
	    // The divisor is smaller than the dividend.  In this case,
	    // we return "total nonsense". Code formerly threw in some random number as well
	    State.L() = (0177777);
	    State.A() = (0177777);
	  }
	else if (AbsA == AbsK && AbsL == agc_t::AGC_P0)
	  {
	    // The divisor is equal to the dividend.
	    if (AccPair[0] == *State.WhereWord)	// Signs agree?
	      {
		Operand16 = 037777;	// Max positive value.
		State.L() = SignExtend (*State.WhereWord);
	      }
	    else
	      {
		Operand16 = (077777 & ~037777);	// Max negative value.
		State.L() = SignExtend (*State.WhereWord);
	      }
	    State.A() = SignExtend (Operand16);
	  }
	else
	  {
	    // The divisor is larger than the dividend.  Okay to actually divide!
	    // Fortunately, the sign conventions agree with those of the normal
	    // C operators / and %, so all we need to do is to convert the
	    // 1's-complement values to native CPU format to do the division,
	    // and then convert back afterward.  Incidentally, we know we
	    // aren't dividing by zero, since we know that the divisor is
	    // greater (in magnitude) than the dividend.
	    Dividend = agc2cpu2 (Dividend);
	    Divisor = agc2cpu (*State.WhereWord);
	    Quotient = Dividend / Divisor;
	    Remainder = Dividend % Divisor;
	    State.A() = SignExtend (cpu2agc(Quotient));
	    if (Remainder == 0)
	      {
		// In this case, we need to make an extra effort, because we
		// might need -0 rather than +0.
		if (Dividend >= 0)
		  State.L() = agc_t::AGC_P0;
		else
		  State.L() = SignExtend (agc_t::AGC_M0);
	      }
	    else
	      State.L() = SignExtend (cpu2agc(Remainder));
	  }
      }
}

/**Implement the BZF instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction

The "Branch Zero to Fixed"
instruction jumps to a memory location in fixed (as opposed to
erasable) memory if the accumulator is zero. 
<table>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Syntax:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">BZF K</span></td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Operand:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">K </span>is the label of a memory
location.&nbsp; It must assemble to a 12-bit memory address in fixed
memory.&nbsp; (In other words, the two most significant bits of address
      <span style="font-family: monospace;">K</span> cannot be 00.)<br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Extracode:</td>
      <td style="vertical-align: top;">This is an extracode, and
therefore must be preceded by an <span style="font-family: monospace;">EXTEND</span>
instruction.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Timing:<br>
      </td>
      <td style="vertical-align: top;">1 MCT (about 11.7 &mu;s) if
the accumulator is plus zero or minus zero, or 2 MCT (about 23.4
&mu;s) if the accumulator is non-zero.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; width: 20%; font-weight: bold;">Flags:<br>
      </td>
      <td style="vertical-align: top;">The Overflow is not
affected.&nbsp; The Extracode flag is cleared.&nbsp; The Q register
is unaffected.<br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Editing:<br>
      </td>
      <td style="vertical-align: top;">The CYR, SR, CYL, and EDOP
registers are not affected.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Octal:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">10000 + K</span><br>
      </td>
    </tr>
    </table>
Notes:

If the accumulator is non-zero, then control proceeds to the next
instruction. Only if the accumulator is plus zero or minus zero
does the branch to address `K` occur. The accumulator (and its stored overflow) are not actually
modified.

Note that if the accumulator contains overflow, then the accumulator is
      <span style="font-style: italic;">not</span> treated as being
zero, even if the sign-corrected value would be +0 or -0.

This instruction <span style="font-style: italic;">does not</span> set
up a later return. Use the @ref TC instruction instead for that.

<span style="text-decoration: underline;">Indirect conditional branch</span>: 
For an indirect conditional branch, it is necessary to combine an @ref INDEX instruction with a <span style="font-family: monospace;">BZF</span> 
instruction. Refer to the entry for the @ref INDEX instruction.
*/
void BZF(agc_t& State) {
	// BZF
      //Operand16 = OverflowCorrected (Accumulator);
      //if (Operand16 == agc_t::AGC_P0 || Operand16 == agc_t::AGC_M0)
      if (State.Accumulator == 0 || State.Accumulator == 0177777)
	{
	  State.BacktraceAdd(0);
	  State.NextZ = State.Address12;
	}
}

/** Implements the MSU instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void MSU(agc_t& State) {
	unsigned ui, uj;
	int diff;
        int16_t Operand16;
	State.WhereWord = State.FindMemoryWord(State.Address10);
	if (State.Address10 < agc_t::REG16)
	  {
	    ui = 0177777 & State.Accumulator;
	    uj = 0177777 & State.Erasable[0][State.Address10];
	  }
	else
	  {
	    ui = (077777 & OverflowCorrected (State.Accumulator));
	    uj = (077777 & *State.WhereWord);
	  }
	diff = ui - uj;
	if (diff < 0)
	  diff--;
	if (State.IsQ(State.Address10))
	  State.A() = 0177777 & diff;
	else
	  {
	    Operand16 = (077777 & diff);
	    State.A() = SignExtend (Operand16);
	  }
	if (State.Address10 >= 020 && State.Address10 <= 023)
	  State.AssignFromPointer(State.WhereWord, *State.WhereWord);
}

/** Implements the QXCH instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void QXCH(agc_t& State) {
    int16_t Operand16;
      if (State.IsQ(State.Address10))
	return;
      if (State.IsReg(State.Address10, agc_t::RegZERO))	// ZQ
	State.Q() = agc_t::AGC_P0;
      else if (State.Address10 < agc_t::REG16)
	{
	  Operand16 = State.Q();
	  State.Q() = State.Erasable[0][State.Address10];
	  State.Erasable[0][State.Address10] = Operand16;
	  if (State.Address10 == agc_t::RegZ)
	    State.NextZ = State.Z();
	}
      else
	{
	  State.WhereWord = State.FindMemoryWord(State.Address10);
	  Operand16 = OverflowCorrected (State.Q());
	  State.Q() = SignExtend (*State.WhereWord);
	  State.AssignFromPointer(State.WhereWord, Operand16);
	}
}

/** Implements the AUG instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction

The "Augment" instruction increments a positive value in an erasable-memory location
in-place by +1, or a negative value by -1.
<table>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Syntax:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">AUG K</span></td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Operand:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">K </span>is the label of a memory
location.&nbsp; It must assemble to a 10-bit memory address in erasable
memory.&nbsp; <span style="font-family: monospace;"></span><br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Extracode:</td>
      <td style="vertical-align: top;">This is an extracode, and
therefore must be preceded by an <span style="font-family: monospace;">EXTEND</span>
instruction.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Timing:<br>
      </td>
      <td style="vertical-align: top;">2 MCT (about 23.4 &mu;s)<br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; width: 20%; font-weight: bold;">Flags:<br>
      </td>
      <td style="vertical-align: top;">The Overflow is set according to
the result of the operation.&nbsp;
The Extracode flag is cleared. <br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Editing:<br>
      </td>
      <td style="vertical-align: top;">Editing is done upon writing to <span style="font-family: monospace;">K</span>, if <span style="font-family: monospace;">K</span> is CYR, SR, CYL, or EDOP.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Octal:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">24000 + K</span><br>
      </td>
    </tr>
</table>
Notes:

If `K` is a 16-bit register like A, L, or Q, then arithmetic is performed on the full 15-bit value (plus sign). Otherwise, only the available
14-bit value (plus sign) is used.

If the contents of `K` before the operation is greater than or equal to +0, it is incremented
by +1. On the other hand, if it is less than or equal to -0, it
is decremented.

If `K` is one of the
counter registers which triggers an interrupt upon overflow, then an
oveflow caused by `AUG` will trigger the interrupt also. These registers include
TIME3-TIME6.&nbsp; Furthermore, if `K` is the TIME1 counter and the `AUG` causes an overflow, the
TIME2 counter will be incremented. Some of the counter registers
such as CDUX-CDUZ are formatted in 2's-complement format, but the `AUG` instruction is insensitive
to this distinction and always uses normal 1's-complement arithmetic.
*/
void AUG(agc_t& State) {
	int Sum;
	int Operand16, Increment;
	State.WhereWord = State.FindMemoryWord(State.Address10);
	if (State.Address10 < agc_t::REG16)
	  Operand16 = State.Erasable[0][State.Address10];
	else
	  Operand16 = SignExtend (*State.WhereWord);
	Operand16 &= 0177777;
	if (0 == (0100000 & Operand16))
	  Increment = agc_t::AGC_P1;
	else
	  Increment = SignExtend (agc_t::AGC_M1);
	Sum = AddSP16 (0177777 & Increment, 0177777 & Operand16);
	if (State.Address10 < agc_t::REG16)
	  State.Erasable[0][State.Address10] = Sum;
	else
	  {
	    State.AssignFromPointer(State.WhereWord, OverflowCorrected (Sum));
	    State.InterruptRequest(State.Address10, Sum);
	  }
}

/** Implements the DIM instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void DIM(agc_t& State) {
	// DIM
      {
	int Sum;
	int Operand16, Increment;
	State.WhereWord = State.FindMemoryWord(State.Address10);
	if (State.Address10 < agc_t::REG16)
	  Operand16 = State.Erasable[0][State.Address10];
	else
	  Operand16 = SignExtend (*State.WhereWord);
	Operand16 &= 0177777;
	if (Operand16 == agc_t::AGC_P0 || Operand16 == SignExtend (agc_t::AGC_M0))
	  return;
	if (0 == (0100000 & Operand16))
	  Increment = SignExtend (agc_t::AGC_M1);
	else
	  Increment = agc_t::AGC_P1;
	Sum = AddSP16 (0177777 & Increment, 0177777 & Operand16);
	if (State.Address10 < agc_t::REG16)
	  State.Erasable[0][State.Address10] = Sum;
	else
	  State.AssignFromPointer(State.WhereWord, OverflowCorrected (Sum));
      }
}

/** Implements the DCA instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction

The "Double Clear and Add"
instruction
moves the contents of a pair of memory locations into the A,L register
pair.
<table>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Syntax:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">DCA K</span><span style="font-family: monospace;"><br>
      </span></td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Operand:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">K </span>is the label of a memory
location.&nbsp; It must assemble to a 12-bit memory address.&nbsp; <span style="font-family: monospace;"></span><br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Extracode:</td>
      <td style="vertical-align: top;">This is an extracode, and
therefore must be preceded by an <span style="font-family: monospace;">EXTEND</span>
instruction.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Timing:<br>
      </td>
      <td style="vertical-align: top;">3 MCT (about 35.1 &mu;s)<br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; width: 20%; font-weight: bold;">Flags:<br>
      </td>
      <td style="vertical-align: top;">The Overflow is cleared.&nbsp;
The Extracode flag is cleared.&nbsp; <br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Editing:<br>
      </td>
      <td style="vertical-align: top;">Editing is done after the
operation, if <span style="font-family: monospace;">K,K+1</span>
coincides with<span style="font-family: monospace;"></span> CYR, SR,
CYL, or EDOP.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Octal:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">30001 + K</span><br>
      </td>
    </tr>
    </table>
Notes:

The value from <span style="font-family: monospace;">K</span> is
transferred into the accumulator, while the value from <span style="font-family: monospace;">K+1</span> is transferred into the L
register.

A side-effect of this instruction is that `K,K+1` are rewritten after their
values are written to the A,L register pair; this means that if `K` or `K+1` is CYR, SR, CYL, or EDOP,
then they are re-edited.

The instruction `DCA L` is
an unusual case. Since the less-significant word is
processed first and then the more-significant word, the effect will be
to first load the L register with the contents of the Q register, and
then to load the A register with the contents of L. In other
words, A and L will <span style="font-style: italic;">both</span> be
loaded with the contents of the 16-bit register Q.

On the other hand, the instruction `DCA Q` will cause the full 16-bit contents (including overflow) of Q
to be loaded into A, and the 15-bit contents of EB to be sign-extended
to 16 bits and loaded into L.

<span style="font-weight: bold;">Note:</span> The final
contents of the L register will be overflow-corrected.
*/
void DCA(agc_t& State) {
      if (State.IsL(State.Address12))
	{
	  State.L() = SignExtend (OverflowCorrected (State.L()));
	  return;
	}
      State.WhereWord = State.FindMemoryWord(State.Address12);
      // Do topmost word first.
      if (State.Address12 < agc_t::REG16)
	State.L() = State.Erasable[0][State.Address12];
      else
	State.L() = SignExtend (*State.WhereWord);
      State.L() = SignExtend (OverflowCorrected (State.L()));
      // Now do bottom word.
      if (State.Address12 < agc_t::REG16 + 1)
	State.A() = State.Erasable[0][State.Address12 - 1];
      else
	State.A() = SignExtend (State.WhereWord[-1]);
      if (State.Address12 >= 020 && State.Address12 <= 023)
	State.AssignFromPointer(State.WhereWord, State.WhereWord[0]);
      if (State.Address12 >= 020 + 1 && State.Address12 <= 023 + 1)
	State.AssignFromPointer(State.WhereWord - 1, State.WhereWord[-1]);
}

/** Implements the DCS and DCOM instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction

The "Double Clear and Subtract"
instruction
moves the 1's-complement (i.e., the negative) of the contents of a pair
of memory locations into the A,L register pair.<br>
<table>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Syntax:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">DCS K</span><span style="font-family: monospace;"><br>
      </span></td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Operand:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">K </span>is the label of a memory
location.&nbsp; It must assemble to a 12-bit memory address.&nbsp; <span style="font-family: monospace;"></span><br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Extracode:</td>
      <td style="vertical-align: top;">This is an extracode, and
therefore must be preceded by an <span style="font-family: monospace;">EXTEND</span>
instruction.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Timing:<br>
      </td>
      <td style="vertical-align: top;">3 MCT (about 35.1 &mu;s)<br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; width: 20%; font-weight: bold;">Flags:<br>
      </td>
      <td style="vertical-align: top;">The Overflow is cleared.&nbsp;
The Extracode flag is cleared.&nbsp; <br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Editing:<br>
      </td>
      <td style="vertical-align: top;">Editing is done after the
operation, if <span style="font-family: monospace;">K,K+1</span>
coincides with<span style="font-family: monospace;"></span> CYR, SR,
CYL, or EDOP.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Octal:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">40001 + K</span><br>
      </td>
    </tr>
    </table>
      <td style="vertical-align: top; font-weight: bold;">
Notes:

The negative of the value from `K` is transferred into the accumulator, while the negative of the value
from `K+1` is transferred into the L register.

A side-effect of this instruction is that `K,K+1` are rewritten after their
values are written to the A,L register pair; this means that if `K` or `K+1` is CYR, SR, CYL, or EDOP,
then they are re-edited.

For the special case `DCS A`, refer to the `DCOM` instruction.

The instruction `DCS L` is
an unusual case. Since the less-significant word is
processed first
and then the more-significant word, the effect will be to first load
the L register with the negative of the contents of the 16-bit Q
register, and
then to load
the A register with the negative of the contents of L. In other
words, A will be loaded with the contents of Q, and L will be loaded
with the negative of the contents of Q.

On the other hand, the instruction `DCS Q` will load A with the
full 16-bit complement of Q, and will load L with the 15-bit complement
of EB as extended to 16 bits.

<span style="font-weight: bold;">Note:</span> The final contents of the L register will be overflow-corrected.

The
"Double Complement" bitwise complements the register pair A,L
<table>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Syntax:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">DCOM</span><span style="font-family: monospace;"><br>
      </span></td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Operand:<br>
      </td>
      <td style="vertical-align: top;">This instruction has no operand.<span style="font-family: monospace;"></span><br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Extracode:</td>
      <td style="vertical-align: top;">This is an extracode, and
therefore must be preceded by an <span style="font-family: monospace;">EXTEND</span>
instruction.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Timing:<br>
      </td>
      <td style="vertical-align: top;">3 MCT (about 35.1 &mu;s). </td>
    </tr>
    <tr>
      <td style="vertical-align: top; width: 20%; font-weight: bold;">Flags:<br>
      </td>
      <td style="vertical-align: top;">The Overflow is unaffected. <span style="color: rgb(0, 153, 0);"></span>
The Extracode flag is cleared.&nbsp; <br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Editing:<br>
      </td>
      <td style="vertical-align: top;">The editing registers CYR, SR,
CYL, or EDOP are unaffected.<br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Octal:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;"><span style="font-family: monospace;">40001</span></span><br>
      </td>
    </tr>
    </table>
Notes:

All
16 bits of the accumulator and the L register are
complemented. Therefore, in addition to
negating the DP value (i.e., converting plus to minus
and minus to plus), the overflow is preserved.

This instruction assembles as `DCS A`.
*/
void DCS(agc_t& State) {
	// DCS
      if (State.IsL(State.Address12))	// DCOM
	{
	  State.A() = ~State.Accumulator;
	  State.L() = ~State.L();
	  State.L() = SignExtend (OverflowCorrected (State.L()));
	  return;
	}
      State.WhereWord = State.FindMemoryWord(State.Address12);
      // Do topmost word first.
      if (State.Address12 < agc_t::REG16)
	State.L() = ~State.Erasable[0][State.Address12];
      else
	State.L() = ~SignExtend (*State.WhereWord);
      State.L() = SignExtend (OverflowCorrected (State.L()));
      // Now do bottom word.
      if (State.Address12 < agc_t::REG16 + 1)
	State.A() = ~State.Erasable[0][State.Address12 - 1];
      else
	State.A() = ~SignExtend (State.WhereWord[-1]);
      if (State.Address12 >= 020 && State.Address12 <= 023)
	State.AssignFromPointer(State.WhereWord, State.WhereWord[0]);
      if (State.Address12 >= 020 + 1 && State.Address12 <= 023 + 1)
	State.AssignFromPointer(State.WhereWord - 1, State.WhereWord[-1]);
}

/** Implements the SU instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void SU(agc_t& State) {
	// SU
      if (State.IsA(State.Address10))
	State.Accumulator = SignExtend (agc_t::AGC_M0);
      else if (State.Address10 < agc_t::REG16)
	State.Accumulator = AddSP16 (State.Accumulator, 0177777 & ~State.Erasable[0][State.Address10]);
      else
	{
	  State.WhereWord = State.FindMemoryWord(State.Address10);
	  State.Accumulator =
	    AddSP16 (State.Accumulator, SignExtend (NegateSP (*State.WhereWord)));
	  State.AssignFromPointer(State.WhereWord, *State.WhereWord);
	}
      State.A() = State.Accumulator;
}

/** Implement the BZMF instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction

The "Branch Zero or Minus to
Fixed"
instruction jumps to a memory location in fixed (as opposed to
erasable) memory if the accumulator is zero or negative. 

<table>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Syntax:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">BZMF K</span></td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Operand:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">K </span>is the label of a memory
location.&nbsp; It must assemble to a 12-bit memory address in fixed
memory.&nbsp; (In other words, the two most significant bits of address
      <span style="font-family: monospace;">K</span> cannot be 00.)<br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Extracode:</td>
      <td style="vertical-align: top;">This is an extracode, and
therefore must be preceded by an <span style="font-family: monospace;">EXTEND</span>
instruction.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold; width: 20%;">Timing:<br>
      </td>
      <td style="vertical-align: top;">1 MCT (about 11.7 &mu;s) if
the accumulator is zero or negative, or 2 MCT (about 23.4 &mu;s) if
the accumulator is positive non-zero.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; width: 20%; font-weight: bold;">Flags:<br>
      </td>
      <td style="vertical-align: top;">The Overflow is not
affected.&nbsp; The Extracode flag is cleared.&nbsp; The Q register
is unaffected.<br>
      </td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Editing:<br>
      </td>
      <td style="vertical-align: top;">The CYR, SR, CYL, and EDOP
registers are not affected.</td>
    </tr>
    <tr>
      <td style="vertical-align: top; font-weight: bold;">Octal:<br>
      </td>
      <td style="vertical-align: top;"><span style="font-family: monospace;">60000 + K</span><br>
      </td>
    </tr>
    </table>
Notes:

If the accumulator is positive non-zero, then control proceeds to the next instruction. Only if
the accumulator is plus zero or negative does the branch to address `K` occur. The accumulator
and its stored oveflow are not actually modified.

Note that if the accumulator contains +overflow, then the accumulator
is <span style="font-style: italic;">not</span> treated as being zero,
even if the sign-corrected value would be +0.&nbsp; If the accumulator
contains negative overflow, then the value is treated as being negative
non-zero, so the jump is taken.

This instruction <span style="font-style: italic;">does not</span> set
up a later return. Use the @ref TC instruction instead for that.<br>

<span style="text-decoration: underline;">Indirect conditional branch</span>:
For an indirect conditional branch, it is necessary to combine an @ref INDEX instruction with a <span style="font-family: monospace;">BZMF</span> instruction. Refer
to the entry for the @ref INDEX instruction.
*/
void BZMF(agc_t& State) {
	// BZMF
      //Operand16 = OverflowCorrected (Accumulator);
      //if (Operand16 == agc_t::AGC_P0 || State.IsNegativeSP(Operand16))
      if (State.Accumulator == 0 || 0 != (State.Accumulator & 0100000))
	{
	  State.BacktraceAdd(0);
	  State.NextZ = State.Address12;
	}
}

/** Implements the MP instruction.
  @param State AGC instance to operate on
  @param ins Decoded instruction
*/
void MP(agc_t& State) {
	// MP
      {
	// For MP A (i.e., SQUARE) the accumulator is NOT supposed to
	// be oveflow-corrected.  I do it anyway, since I don't know
	// what it would mean to carry out the operation otherwise.
	// Fix later if it causes a problem.
	// FIX ME: Accumulator is overflow-corrected before SQUARE.
	int16_t MsWord, LsWord, Operand16, OtherOperand16;
	int Product;
	State.WhereWord = State.FindMemoryWord(State.Address12);
	Operand16 = OverflowCorrected (State.Accumulator);
	if (State.Address12 < agc_t::REG16)
	  OtherOperand16 = OverflowCorrected (State.Erasable[0][State.Address12]);
	else
	  OtherOperand16 = *State.WhereWord;
	if (OtherOperand16 == agc_t::AGC_P0 || OtherOperand16 == agc_t::AGC_M0)
	  MsWord = LsWord = agc_t::AGC_P0;
	else if (Operand16 == agc_t::AGC_P0 || Operand16 == agc_t::AGC_M0)
	  {
	    if ((Operand16 == agc_t::AGC_P0 && 0 != (040000 & OtherOperand16)) ||
		(Operand16 == agc_t::AGC_M0 && 0 == (040000 & OtherOperand16)))
	      MsWord = LsWord = agc_t::AGC_M0;
	    else
	      MsWord = LsWord = agc_t::AGC_P0;
	  }
	else
	  {
	    int16_t WordPair[2];
	    Product =
	      agc2cpu (SignExtend (Operand16)) *
	      agc2cpu (SignExtend (OtherOperand16));
	    Product = cpu2agc2 (Product);
	    // Sign-extend, because it's needed for DecentToSp.
	    if (02000000000 & Product)
	      Product |= 004000000000;
	    // Convert back to DP.
	    DecentToSp (Product, &WordPair[1]);
	    MsWord = WordPair[0];
	    LsWord = WordPair[1];
	  }
	State.A() = SignExtend (MsWord);
	State.L() = SignExtend (LsWord);
      }
}

/**Opcode instruction table. As a bonus to doing this all with function pointers, we get
an opcode map. We could shave a bit off of the opcode size, and
save half of this table space, except for line 010X below. */
const opcode_f agc_t::opcode[0200]= {
    TC,   TC,   TC,   TC,   TC,   TC,   TC,   TC,     //0000-0007  All of the below are non-extracodes
    CCS,  CCS,  TCF,  TCF,  TCF,  TCF,  TCF,  TCF,    //0010-0017
    DAS,  DAS,  LXCH, LXCH, INCR, INCR, ADS,  ADS,    //0020-0027
    CA,   CA,   CA,   CA,   CA,   CA,   CA,   CA,     //0030-0037
    CS,   CS,   CS,   CS,   CS,   CS,   CS,   CS,     //0040-0047
    INDEX,INDEX,DXCH, DXCH, TS,   TS,   XCH,  XCH,    //0050-0057
    AD,   AD,   AD,   AD,   AD,   AD,   AD,   AD,     //0060-0067
    MASK, MASK, MASK, MASK, MASK, MASK, MASK, MASK,   //0070-0077
    READ, WRITE,RAND, WAND, ROR,  WOR,  RXOR, EDRUPT, //0100-0107  All of the below are Extracodes
    DV,   DV,   BZF,  BZF,  BZF,  BZF,  BZF,  BZF,    //0110-0117
    MSU,  MSU,  QXCH, QXCH, AUG,  AUG,  DIM,  DIM,    //0120-0127
    DCA,  DCA,  DCA,  DCA,  DCA,  DCA,  DCA,  DCA,    //0130-0137
    DCS,  DCS,  DCS,  DCS,  DCS,  DCS,  DCS,  DCS,    //0140-0147
    INDEX,INDEX,INDEX,INDEX,INDEX,INDEX,INDEX,INDEX,  //0150-0157
    SU,   SU,   BZMF, BZMF, BZMF, BZMF, BZMF, BZMF,   //0160-0167
    MP,   MP,   MP,   MP,   MP,   MP,   MP,   MP      //0170-0177
};

/**Opcode instruction table. As a bonus to doing this all with function pointers, we get
an opcode map. We could shave a bit off of the opcode size, and
save half of this table space, except for line 010X below. */
#ifdef USE_STD_STRING
const std::string agc_t::opcode_name[0200]= {
#else
const char* agc_t::opcode_name[0200]= {
#endif
    "TC",   "TC",   "TC",   "TC",   "TC",   "TC",   "TC",   "TC",     //0000-0007  All of the below are non-extracodes
    "CCS",  "CCS",  "TCF",  "TCF",  "TCF",  "TCF",  "TCF",  "TCF",    //0010-0017
    "DAS",  "DAS",  "LXCH", "LXCH", "INCR", "INCR", "ADS",  "ADS",    //0020-0027
    "CA",   "CA",   "CA",   "CA",   "CA",   "CA",   "CA",   "CA",     //0030-0037
    "CS",   "CS",   "CS",   "CS",   "CS",   "CS",   "CS",   "CS",     //0040-0047
    "INDEX","INDEX","DXCH", "DXCH", "TS",   "TS",   "XCH",  "XCH",    //0050-0057
    "AD",   "AD",   "AD",   "AD",   "AD",   "AD",   "AD",   "AD",     //0060-0067
    "MASK", "MASK", "MASK", "MASK", "MASK", "MASK", "MASK", "MASK",   //0070-0077
    "READ", "WRITE","RAND", "WAND", "ROR",  "WOR",  "RXOR", "EDRUPT", //0100-0107  All of the below are Extracodes
    "DV",   "DV",   "BZF",  "BZF",  "BZF",  "BZF",  "BZF",  "BZF",    //0110-0117
    "MSU",  "MSU",  "QXCH", "QXCH", "AUG",  "AUG",  "DIM",  "DIM",    //0120-0127
    "DCA",  "DCA",  "DCA",  "DCA",  "DCA",  "DCA",  "DCA",  "DCA",    //0130-0137
    "DCS",  "DCS",  "DCS",  "DCS",  "DCS",  "DCS",  "DCS",  "DCS",    //0140-0147
    "INDEX","INDEX","INDEX","INDEX","INDEX","INDEX","INDEX","INDEX",  //0150-0157
    "SU",   "SU",   "BZMF", "BZMF", "BZMF", "BZMF", "BZMF", "BZMF",   //0160-0167
    "MP",   "MP",   "MP",   "MP",   "MP",   "MP",   "MP",   "MP"      //0170-0177
};
