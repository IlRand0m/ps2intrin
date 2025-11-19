<h1>ps2intrin</h1>
ps2intrin is a header-only library providing C-Function wrappers for EE-Core-specific instructions.

This library works like other <*intrin.h> headers provided for other targets, i.e. <x86intrin.h>.

There is also a minimal CMakeLists.txt provided for integration into CMake projects.

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
- MADD : Multiply - Add word
- MADDU : Multiply - Add unsigned word
- MADD1 : Multiply - Add word pipeline 1
- MADDU1 : Multiply - Add unsigned word pipeline 1
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
- PSLLH : Parallel Shift Left Logical Halfword
- PSLLVW : Parallel Shift Left Logical Variable Word
- PSLLW : Parallel Shift Left Logical Word
- PSRAH : Parallel Shift Right Arithmetic Halfword
- PSRAVW : Parallel Shift Right Arithmetic Variable Word
- PSRAW : Parallel Shift Right Arithmetic Word
- PSRLH : Parallel Shift Right Logical Halfword
- PSRLVW : Parallel Shift Right Logical Variable Word
- PSRLW : Parallel Shift Right Logical Word
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
- PABSH : Parallel Absolute Halfword
- PABSW : Parallel Absolute Word
- PMAXH : Parallel Maximum Halfword
- PMAXW : Parallel Maximum Word
- PMINH : Parallel Minimum Halfword
- PMINW : Parallel Minimum Word
- PCEQB : Parallel Compare for Equal Byte
- PCEQH : Parallel Compare for Equal Halfword
- PCEQW : Parallel Compare for Equal Word
- PCGTB : Parallel Compare for Greater Than Byte
- PCGTH : Parallel Compare for Greater Than Halfword
- PCGTW : Parallel Compare for Greater Than Word
- PCPYH : Parallel Copy Halfword
- PCPYLD : Parallel Copy Lower Doubleword
- PCPYUD : Parallel Copy Upper Doubleword
- PEXCH : Parallel Exchange Center Halfword
- PEXCW : Parallel Exchange Center Word
- PEXEH : Parallel Exchange Even Halfword
- PEXEW : Parallel Exchange Even Word
- PEXT5 : Parallel Extent from 5 bits
- PEXTLB : Parallel Extent Lower from Byte
- PEXTLH : Parallel Extent Lower from Halfword
- PEXTLW : Parallel Extent Lower from Word
- PEXTUB : Parallel Extent Upper from Byte
- PEXTUH : Parallel Extent Upper from Halfword
- PEXTUW : Parallel Extent Upper from Word
- PINTEH : Parallel Interleave Even Halfword
- PINTH : Parallel Interleave Halfword
- PPAC5 : Parallel Pack to 5 bits
- PPACB : Parallel Pack to Byte
- PPACH : Parallel Pack to Halfword
- PPACW : Parallel Pack to Word
- PREVH : Parallel Reverse Word
- PROT3W : Parallel Rotate 3 Words Left
- PLZCW : Parallel Leading Zero or One Count Word

  ---

  There are also helper functions provided for ease of use.
  Those may compile to significantly more than a single instruction.
