<h1>ps2intrin</h1>
ps2intrin is a header-only library providing C-Function wrappers for EE-Core-specific instructions.

This library works like other <*intrin.h> headers provided for other targets, i.e. <x86intrin.h>.

There is also a minimal CMakeLists.txt provided for integration into CMake projects.

<h3>Warning</h3>

128-bit multimedia instructions only work with high optimization settings currently.
This is due to GCC assuming 128-bit values take up 2 adjacent 64-bit registers rather than one 128-bit register.
Loading a quadword using the LQ instruction will correctly populate a single register but GCC will think it has actually written to that and the next register.
These 2 registers are then i.e. spilled on the stack, etc.
This obviously writes garbage.
Only workaround is using i.e. -O3 on functions using these multimedia instructions and checking if the compiler actually allocated all values to registers
and didn't have to spill any to the stack.

The multiply and the multiply-accummulate instructions in pipeline 0 behave similarly.
Because they use the LO and HI registers, any modification to those will be observed when finally querying the result.
This means that any multiplication in the code, even for i.e. address calculations, will overwrite the result of the multiply/multiply-accummulate instruction
if it is not saved using the move-from-lo/hi instructions (and then restored using move-to-lo/hi).
High optimization levels tend to remove extraneous multiplactions, making those roundtrips unnecessary.

<h3>List of covered instructions : </h3>

- BREAK : Breakpoint
- PREF : Prefetch

---

- MTLO : Move to LO register
- MFLO : Move from LO register
- MTHI : Move to HI register
- MFHI : Move from HI register
- MTLO1 : Move to LO1 register
- MFLO1 : Move from LO1 register
- MTHI1 : Move to HI1 register
- MFHI1 : Move from HI1 register

---

- MTSA : Move to SA register
- MFSA : Move from SA register
- MTSAB : Move byte count to SA register
- MTSAH : Move halfword count to SA register
- QFSRV : Quadword Funnel Shift Right Variable

---

- MULT : Multiply word
- MULTU : Multiply unsigned word
- MULT1 : Multiply word pipeline 1
- MULTU1 : Multiply unsigned word pipeline 1
- MADD : Multiply-Add word
- MADDU : Multiply-Add unsigned word
- MADD1 : Multiply-Add word pipeline 1
- MADDU1 : Multiply-Add unsigned word pipeline 1
- DIV : Divide word
- DIVU : Divide unsigned word
- DIV1 : Divide word pipeline 1
- DIVU1 : Divide unsigned word pipeline 1

---

- LQ : Load Quadword
- SQ : Store Quadword
- PMFLO : Parallel Move From LO Register
- PMFHI : Parallel Move From HI Register
- PMFHL(.LH/.LW/.SH/.SLW/.UW) : Parallel Move from HI/LO Register
- PMTLO : Parallel Move To LO Register
- PMTHI : Parallel Move To HI Register
- PMTHL.LW : Parallel Move To HI/LO Register
- PAND : Parallel And
- POR : Parallel Or
- PXOR : Parallel Exclusive Or
- PNOR : Parallel Not Or
- PCEQB : Parallel Compare for Equal Byte
- PCEQH : Parallel Compare for Equal Halfword
- PCEQW : Parallel Compare for Equal Word
- PCGTB : Parallel Compare for Greater Than Byte
- PCGTH : Parallel Compare for Greater Than Halfword
- PCGTW : Parallel Compare for Greater Than Word
- PSLLH : Parallel Shift Left Logical Halfword
- PSLLW : Parallel Shift Left Logical Word
- PSLLVW : Parallel Shift Left Logical Variable Word
- PSRAH : Parallel Shift Right Arithmetic Halfword
- PSRAW : Parallel Shift Right Arithmetic Word
- PSRAVW : Parallel Shift Right Arithmetic Variable Word
- PSRLH : Parallel Shift Right Logical Halfword
- PSRLW : Parallel Shift Right Logical Word
- PSRLVW : Parallel Shift Right Logical Variable Word
- PABSH : Parallel Absolute Halfword
- PABSW : Parallel Absolute Word
- PMAXH : Parallel Maximum Halfword
- PMAXW : Parallel Maximum Word
- PMINH : Parallel Minimum Halfword
- PMINW : Parallel Minimum Word
- PADDB : Parallel Add Byte
- PADDH : Parallel Add Halfword
- PADDW : Parallel Add Word
- PADDSB : Parallel Add Byte Signed Saturation
- PADDSH : Parallel Add Halfword Signed Saturation
- PADDSW : Parallel Add Word Signed Saturation
- PADDUB : Parallel Add Byte Unsigned Saturation
- PADDUH : Parallel Add Halfword Unsigned Saturation
- PADDUW : Parallel Add Word Unsigned Saturation
- PSUBB : Parallel Subtract Byte
- PSUBH : Parallel Subtract Halfword
- PSUBW : Parallel Subtract Word
- PSUBSB : Parallel Subtract Byte Signed Saturation
- PSUBSH : Parallel Subtract Halfword Signed Saturation
- PSUBSW : Parallel Subtract Word Signed Saturation
- PSUBUB : Parallel Subtract Byte Unsigned Saturation
- PSUBUH : Parallel Subtract Halfword Unsigned Saturation
- PSUBUW : Parallel Subtract Word Unsigned Saturation
- PMULTH : Parallel Multiply Halfword
- PMULTW : Parallel Multiply Word
- PMULTUW : Parallel Multiply Unsigned Word
- PMADDH : Parallel Multiply-Add Halfword
- PMSUBH : Parallel Multiply-Subtract Halfword
- PMADDW : Parallel Multiply-Add Word
- PMSUBW : Parallel Multiply-Subtract Word
- PMADDUW : Parallel Multiply-Add Unsigned Word
- PHMADH : Parallel Horizontal Multiply-Add Halfword
- PHMSBH : Parallel Horizontal Multiply-Subtract Halfword
- PDIVW : Parallel Divide Word
- PDIVBW : Parallel Divide Broadcast Word
- PDIVUW : Parallel Divide Unsigned Word
- PCPYH : Parallel Copy Halfword
- PCPYLD : Parallel Copy Lower Doubleword
- PCPYUD : Parallel Copy Upper Doubleword
- PEXCH : Parallel Exchange Center Halfword
- PEXCW : Parallel Exchange Center Word
- PEXEH : Parallel Exchange Even Halfword
- PEXEW : Parallel Exchange Even Word
- PREVH : Parallel Reverse Halfword
- PROT3W : Parallel Rotate 3 Words Left
- PEXTLB : Parallel Extend Lower from Byte
- PEXTLH : Parallel Extend Lower from Halfword
- PEXTLW : Parallel Extend Lower from Word
- PEXTUB : Parallel Extend Upper from Byte
- PEXTUH : Parallel Extend Upper from Halfword
- PEXTUW : Parallel Extend Upper from Word
- PINTEH : Parallel Interleave Even Halfword
- PINTH : Parallel Interleave Halfword
- PPACB : Parallel Pack to Byte
- PPACH : Parallel Pack to Halfword
- PPACW : Parallel Pack to Word
- PEXT5 : Parallel Extend from 5 bits
- PPAC5 : Parallel Pack to 5 bits
- PLZCW : Parallel Leading Zero or One Count Word

  ---

  There are also helper functions provided for ease of use.
  Those may compile to significantly more than a single instruction.
