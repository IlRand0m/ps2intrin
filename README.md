<h1>ps2intrin</h1>
ps2intrin is a header-only library providing C-function wrappers for EE-Core-specific instructions.

This library works like other <*intrin.h> headers provided for other targets, i.e., <x86intrin.h>.

There is also a minimal CMakeLists.txt provided for integration into CMake projects.

<h3>Correctness</h3>

By default, ps2intrin is in safe mode.
This mode ensures correct behavior as is intended by the intrinsics used.
It achieves this by saving the expected state of the LO/HI/SA registers in user-allocated variables, as well as splitting 128-bit types into 2 registers.
This comes at a large cost to performance but is necessary to achieve correct results in debug builds (-O0).
Safe mode can be turned off by defining 'PS2INTRIN_UNSAFE' before including the header.
In unsafe mode, 128-bit types will no longer be split up, and external tracking of the LO/HI/SA registers is disabled.
It is no longer guaranteed to produce the same results as safe mode, as the meaning of the resulting code is dependent on surrounding instructions and the
presence or absence of compiler optimizations.
Using an optimization level higher than -O0 is necessary.
All instructions operating on the LO/HI/SA registers or on 128-bit types are affected by this switch.
To achieve both correct and fast code, it is recommended to first write a correct routine using safe mode, then switch to unsafe mode, and check the resulting
output for correctness (using tests or manually checking the assembly).

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
