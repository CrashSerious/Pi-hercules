/* GENERAL3.C   (c) Copyright Roger Bowler, 2009                     */
/*              Additional General Instructions                      */

// $Id: general3.c 5125 2009-01-23 12:01:44Z bernard $

/*-------------------------------------------------------------------*/
/* This module implements additional general instructions introduced */
/* as later extensions to z/Architecture and described in the manual */
/* SA22-7832-06 z/Architecture Principles of Operation               */
/*-------------------------------------------------------------------*/

// $Log$
// Revision 1.22  2008/05/12 16:08:29  rbowler
// ECAG correction
//
// Revision 1.21  2008/03/30 00:07:58  rbowler
// Incorrect bit selection again for RNSBG,RISBG,ROSBG,RXSBG
//
// Revision 1.20  2008/03/28 23:03:54  rbowler
// Correct relative address calculation for RIL-format instructions
//
// Revision 1.19  2008/03/23 08:00:40  rbowler
// Fix incorrect bit selection for RNSBG,RISBG,ROSBG,RXSBG
//
// Revision 1.18  2008/03/12 18:23:55  jj
// Fix store operation of ASI, AGSI, ALSI and AGLSI
//
// Revision 1.17  2008/03/08 23:08:32  rbowler
// Add STHRL,STRL,STGRL instructions
//
// Revision 1.16  2008/03/08 22:54:25  rbowler
// Add LHRL,LGHRL,LLHRL,LLGHRL,LLGFRL,LRL,LGRL,LGFRL instructions
//
// Revision 1.15  2008/03/08 22:28:04  rbowler
// Add CHRL,CGHRL,CLRL,CLGRL,CLGFRL,CLHRL,CLGHRL,
// CRL,CGRL,CGFRL instructions
//
// Revision 1.14  2008/03/05 16:36:51  rbowler
// Add RNSBG,RISBG,ROSBG,RXSBG instructions
//
// Revision 1.13  2008/03/05 12:04:23  rbowler
// Add CRJ,CGRJ,CIJ,CGIJ,CLRJ,CLGRJ,CLIJ,CLGIJ instructions
//
// Revision 1.12  2008/03/04 17:09:14  rbowler
// Add CRT,CGRT,CIT,CGIT,CLRT,CLGRT,CLFIT,CLGIT instructions
//
// Revision 1.11  2008/03/04 15:42:50  rbowler
// Add CRB,CGRB,CIB,CGIB,CLRB,CLGRB,CLIB,CLGIB instructions
//
// Revision 1.10  2008/03/04 14:40:28  rbowler
// Add CLFHSI,CLHHSI,CLGHSI instructions
//
// Revision 1.9  2008/03/04 14:23:00  rbowler
// Add CHHSI,CGHSI,CHSI,CGH instructions
//
// Revision 1.8  2008/03/03 23:22:43  rbowler
// Add LTGF instruction
//
// Revision 1.7  2008/03/03 22:43:43  rbowler
// Add MVHI,MVHHI,MVGHI instructions
//
// Revision 1.6  2008/03/03 00:21:45  rbowler
// Add ECAG,LAEY,PFD,PFDRL instructions
//
// Revision 1.5  2008/03/02 23:29:49  rbowler
// Add MFY,MHY,MSFI,MSGFI instructions
//
// Revision 1.4  2008/03/01 23:07:06  rbowler
// Add ALSI,ALGSI instructions
//
// Revision 1.3  2008/03/01 22:49:31  rbowler
// ASI,AGSI treat I2 operand as 8-bit signed integer
//
// Revision 1.2  2008/03/01 22:41:51  rbowler
// Add ASI,AGSI instructions
//
// Revision 1.1  2008/03/01 14:19:29  rbowler
// Add new module general3.c for general-instructions-extension facility
//

#include "hstdinc.h"

#if !defined(_HENGINE_DLL_)
#define _HENGINE_DLL_
#endif

#if !defined(_GENERAL3_C_)
#define _GENERAL3_C_
#endif

#include "hercules.h"
#include "opcode.h"
#include "inline.h"


#if defined(FEATURE_GENERAL_INSTRUCTIONS_EXTENSION_FACILITY)

/*-------------------------------------------------------------------*/
/* EB6A ASI   - Add Immediate Storage                          [SIY] */
/*-------------------------------------------------------------------*/
DEF_INST(add_immediate_storage)
{
BYTE    i2;                             /* Immediate byte            */
int     b1;                             /* Base of effective addr    */
VADR    effective_addr1;                /* Effective address         */
U32     n;                              /* 32-bit operand value      */
int     cc;                             /* Condition Code            */

    SIY(inst, regs, i2, b1, effective_addr1);

    /* Load 32-bit operand from operand address */
    n = ARCH_DEP(vfetch4) ( effective_addr1, b1, regs );

    /* Add signed operands and set condition code */
    cc = add_signed (&n, n, (S32)(S8)i2);

    /* Store 32-bit operand at operand address */
    ARCH_DEP(vstore4) ( n, effective_addr1, b1, regs );

    /* Update Condition Code */
    regs->psw.cc = cc;

    /* Program check if fixed-point overflow */
    if ( regs->psw.cc == 3 && FOMASK(&regs->psw) )
        regs->program_interrupt (regs, PGM_FIXED_POINT_OVERFLOW_EXCEPTION);

} /* end DEF_INST(add_immediate_storage) */


/*-------------------------------------------------------------------*/
/* EB7A AGSI  - Add Immediate Long Storage                     [SIY] */
/*-------------------------------------------------------------------*/
DEF_INST(add_immediate_long_storage)
{
BYTE    i2;                             /* Immediate byte            */
int     b1;                             /* Base of effective addr    */
VADR    effective_addr1;                /* Effective address         */
U64     n;                              /* 64-bit operand value      */
int     cc;                             /* Condition Code            */

    SIY(inst, regs, i2, b1, effective_addr1);

    /* Load 64-bit operand from operand address */
    n = ARCH_DEP(vfetch8) ( effective_addr1, b1, regs );

    /* Add signed operands and set condition code */
    cc = add_signed_long (&n, n, (S64)(S8)i2);

    /* Store 64-bit value at operand address */
    ARCH_DEP(vstore8) ( n, effective_addr1, b1, regs );

    /* Update Condition Code */
    regs->psw.cc = cc;

    /* Program check if fixed-point overflow */
    if ( regs->psw.cc == 3 && FOMASK(&regs->psw) )
        regs->program_interrupt (regs, PGM_FIXED_POINT_OVERFLOW_EXCEPTION);

} /* end DEF_INST(add_immediate_long_storage) */


/*-------------------------------------------------------------------*/
/* EB6E ALSI  - Add Logical with Signed Immediate              [SIY] */
/*-------------------------------------------------------------------*/
DEF_INST(add_logical_with_signed_immediate)
{
BYTE    i2;                             /* Immediate byte            */
int     b1;                             /* Base of effective addr    */
VADR    effective_addr1;                /* Effective address         */
U32     n;                              /* 32-bit operand value      */
int     cc;                             /* Condition Code            */

    SIY(inst, regs, i2, b1, effective_addr1);

    /* Load 32-bit operand from operand address */
    n = ARCH_DEP(vfetch4) ( effective_addr1, b1, regs );

    /* Add operands and set condition code */
    cc = (S8)i2 < 0 ?
        sub_logical (&n, n, (S32)(-(S8)i2)) :
        add_logical (&n, n, (S32)(S8)i2);

    /* Store 32-bit operand at operand address */
    ARCH_DEP(vstore4) ( n, effective_addr1, b1, regs );

    /* Update Condition Code */
    regs->psw.cc = cc;

} /* end DEF_INST(add_logical_with_signed_immediate) */


/*-------------------------------------------------------------------*/
/* EB7E ALGSI - Add Logical with Signed Immediate Long         [SIY] */
/*-------------------------------------------------------------------*/
DEF_INST(add_logical_with_signed_immediate_long)
{
BYTE    i2;                             /* Immediate byte            */
int     b1;                             /* Base of effective addr    */
VADR    effective_addr1;                /* Effective address         */
U64     n;                              /* 64-bit operand value      */
int     cc;                             /* Condition Code            */

    SIY(inst, regs, i2, b1, effective_addr1);

    /* Load 64-bit operand from operand address */
    n = ARCH_DEP(vfetch8) ( effective_addr1, b1, regs );

    /* Add operands and set condition code */
    cc = (S8)i2 < 0 ?
        sub_logical_long (&n, n, (S64)(-(S8)i2)) :
        add_logical_long (&n, n, (S64)(S8)i2);

    /* Store 64-bit value at operand address */
    ARCH_DEP(vstore8) ( n, effective_addr1, b1, regs );

    /* Update Condition Code */
    regs->psw.cc = cc;

} /* end DEF_INST(add_logical_with_signed_immediate_long) */


/*-------------------------------------------------------------------*/
/* ECF6 CRB   - Compare and Branch Register                    [RRS] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_and_branch_register)
{
int     r1, r2;                         /* Register numbers          */
int     m3;                             /* Mask bits                 */
int     b4;                             /* Base of effective addr    */
VADR    effective_addr4;                /* Effective address         */
int     cc;                             /* Comparison result         */

    RRS_B(inst, regs, r1, r2, m3, b4, effective_addr4);

    /* Compare signed operands and set comparison result */
    cc = (S32)regs->GR_L(r1) < (S32)regs->GR_L(r2) ? 1 :
         (S32)regs->GR_L(r1) > (S32)regs->GR_L(r2) ? 2 : 0;

    /* Branch to operand address if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
        SUCCESSFUL_BRANCH(regs, effective_addr4, 6);
    else
        INST_UPDATE_PSW(regs, 6, 0);

} /* end DEF_INST(compare_and_branch_register) */


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* ECE4 CGRB  - Compare and Branch Long Register               [RRS] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_and_branch_long_register)
{
int     r1, r2;                         /* Register numbers          */
int     m3;                             /* Mask bits                 */
int     b4;                             /* Base of effective addr    */
VADR    effective_addr4;                /* Effective address         */
int     cc;                             /* Comparison result         */

    RRS_B(inst, regs, r1, r2, m3, b4, effective_addr4);

    /* Compare signed operands and set comparison result */
    cc = (S64)regs->GR_G(r1) < (S64)regs->GR_G(r2) ? 1 :
         (S64)regs->GR_G(r1) > (S64)regs->GR_G(r2) ? 2 : 0;

    /* Branch to operand address if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
        SUCCESSFUL_BRANCH(regs, effective_addr4, 6);
    else
        INST_UPDATE_PSW(regs, 6, 0);

} /* end DEF_INST(compare_and_branch_long_register) */
#endif /*defined(FEATURE_ESAME)*/


/*-------------------------------------------------------------------*/
/* EC76 CRJ   - Compare and Branch Relative Register           [RIE] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_and_branch_relative_register)
{
int     r1, r2;                         /* Register numbers          */
int     m3;                             /* Mask bits                 */
S16     i4;                             /* 16-bit immediate offset   */
int     cc;                             /* Comparison result         */

    RIE_RRIM_B(inst, regs, r1, r2, i4, m3);

    /* Compare signed operands and set comparison result */
    cc = (S32)regs->GR_L(r1) < (S32)regs->GR_L(r2) ? 1 :
         (S32)regs->GR_L(r1) > (S32)regs->GR_L(r2) ? 2 : 0;

    /* Branch to immediate offset if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
        SUCCESSFUL_RELATIVE_BRANCH(regs, 2*i4, 6);
    else
        INST_UPDATE_PSW(regs, 6, 0);

} /* end DEF_INST(compare_and_branch_relative_register) */


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* EC64 CGRJ  - Compare and Branch Relative Long Register      [RIE] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_and_branch_relative_long_register)
{
int     r1, r2;                         /* Register numbers          */
int     m3;                             /* Mask bits                 */
S16     i4;                             /* 16-bit immediate offset   */
int     cc;                             /* Comparison result         */

    RIE_RRIM_B(inst, regs, r1, r2, i4, m3);

    /* Compare signed operands and set comparison result */
    cc = (S64)regs->GR_G(r1) < (S64)regs->GR_G(r2) ? 1 :
         (S64)regs->GR_G(r1) > (S64)regs->GR_G(r2) ? 2 : 0;

    /* Branch to immediate offset if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
        SUCCESSFUL_RELATIVE_BRANCH(regs, 2*i4, 6);
    else
        INST_UPDATE_PSW(regs, 6, 0);

} /* end DEF_INST(compare_and_branch_relative_long_register) */
#endif /*defined(FEATURE_ESAME)*/


/*-------------------------------------------------------------------*/
/* B972 CRT   - Compare and Trap Register                      [RRF] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_and_trap_register)
{
int     r1, r2;                         /* Register numbers          */
int     m3;                             /* Mask bits                 */
int     cc;                             /* Comparison result         */

    RRF_M(inst, regs, r1, r2, m3);

    /* Compare signed operands and set comparison result */
    cc = (S32)regs->GR_L(r1) < (S32)regs->GR_L(r2) ? 1 :
         (S32)regs->GR_L(r1) > (S32)regs->GR_L(r2) ? 2 : 0;

    /* Raise data exception if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
    {
        regs->dxc = DXC_COMPARE_AND_TRAP;
        ARCH_DEP(program_interrupt) (regs, PGM_DATA_EXCEPTION);
    }

} /* end DEF_INST(compare_and_trap_register) */


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* B960 CGRT  - Compare and Trap Long Register                 [RRF] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_and_trap_long_register)
{
int     r1, r2;                         /* Register numbers          */
int     m3;                             /* Mask bits                 */
int     cc;                             /* Comparison result         */

    RRF_M(inst, regs, r1, r2, m3);

    /* Compare signed operands and set comparison result */
    cc = (S64)regs->GR_G(r1) < (S64)regs->GR_G(r2) ? 1 :
         (S64)regs->GR_G(r1) > (S64)regs->GR_G(r2) ? 2 : 0;

    /* Raise data exception if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
    {
        regs->dxc = DXC_COMPARE_AND_TRAP;
        ARCH_DEP(program_interrupt) (regs, PGM_DATA_EXCEPTION);
    }

} /* end DEF_INST(compare_and_trap_long_register) */
#endif /*defined(FEATURE_ESAME)*/


/*-------------------------------------------------------------------*/
/* E554 CHHSI - Compare Halfword Immediate Halfword Storage    [SIL] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_halfword_immediate_halfword_storage)
{
int     b1;                             /* Base of effective addr    */
VADR    effective_addr1;                /* Effective address         */
S16     i2;                             /* 16-bit immediate value    */
S16     n;                              /* 16-bit storage value      */

    SIL(inst, regs, i2, b1, effective_addr1);

    /* Load 16-bit value from first operand address */
    n = (S16)ARCH_DEP(vfetch2) ( effective_addr1, b1, regs );

    /* Compare signed operands and set condition code */
    regs->psw.cc = n < i2 ? 1 : n > i2 ? 2 : 0;

} /* end DEF_INST(compare_halfword_immediate_halfword_storage) */


/*-------------------------------------------------------------------*/
/* E558 CGHSI - Compare Halfword Immediate Long Storage        [SIL] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_halfword_immediate_long_storage)
{
int     b1;                             /* Base of effective addr    */
VADR    effective_addr1;                /* Effective address         */
S16     i2;                             /* 16-bit immediate value    */
S64     n;                              /* 64-bit storage value      */

    SIL(inst, regs, i2, b1, effective_addr1);

    /* Load 64-bit value from first operand address */
    n = (S64)ARCH_DEP(vfetch8) ( effective_addr1, b1, regs );

    /* Compare signed operands and set condition code */
    regs->psw.cc = n < i2 ? 1 : n > i2 ? 2 : 0;

} /* end DEF_INST(compare_halfword_immediate_long_storage) */


/*-------------------------------------------------------------------*/
/* E55C CHSI  - Compare Halfword Immediate Storage             [SIL] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_halfword_immediate_storage)
{
int     b1;                             /* Base of effective addr    */
VADR    effective_addr1;                /* Effective address         */
S16     i2;                             /* 16-bit immediate value    */
S32     n;                              /* 32-bit storage value      */

    SIL(inst, regs, i2, b1, effective_addr1);

    /* Load 32-bit value from first operand address */
    n = (S32)ARCH_DEP(vfetch4) ( effective_addr1, b1, regs );

    /* Compare signed operands and set condition code */
    regs->psw.cc = n < i2 ? 1 : n > i2 ? 2 : 0;

} /* end DEF_INST(compare_halfword_immediate_storage) */


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* E334 CGH   - Compare Halfword Long                          [RXY] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_halfword_long)
{
int     r1;                             /* Values of R fields        */
int     b2;                             /* Base of effective addr    */
VADR    effective_addr2;                /* Effective address         */
S64     n;                              /* 64-bit operand value      */

    RXY(inst, regs, r1, b2, effective_addr2);

    /* Load rightmost 2 bytes of comparand from operand address */
    n = (S16)ARCH_DEP(vfetch2) ( effective_addr2, b2, regs );

    /* Compare signed operands and set condition code */
    regs->psw.cc =
            (S64)regs->GR_G(r1) < n ? 1 :
            (S64)regs->GR_G(r1) > n ? 2 : 0;

} /* end DEF_INST(compare_halfword_long) */
#endif /*defined(FEATURE_ESAME)*/


/*-------------------------------------------------------------------*/
/* C6x5 CHRL  - Compare Halfword Relative Long                 [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_halfword_relative_long)
{
int     r1;                             /* Register number           */
VADR    addr2;                          /* Relative operand address  */
U16     n;                              /* Relative operand value    */

    RIL_A(inst, regs, r1, addr2);

    /* Load relative operand from instruction address space */
    n = ARCH_DEP(vfetch2) ( addr2, USE_INST_SPACE, regs );

    /* Compare signed operands and set condition code */
    regs->psw.cc =
            (S32)regs->GR_L(r1) < (S16)n ? 1 :
            (S32)regs->GR_L(r1) > (S16)n ? 2 : 0;

} /* end DEF_INST(compare_halfword_relative_long) */


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* C6x4 CGHRL - Compare Halfword Relative Long Long            [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_halfword_relative_long_long)
{
int     r1;                             /* Register number           */
VADR    addr2;                          /* Relative operand address  */
U16     n;                              /* Relative operand value    */

    RIL_A(inst, regs, r1, addr2);

    /* Load relative operand from instruction address space */
    n = ARCH_DEP(vfetch2) ( addr2, USE_INST_SPACE, regs );

    /* Compare signed operands and set condition code */
    regs->psw.cc =
            (S64)regs->GR_G(r1) < (S16)n ? 1 :
            (S64)regs->GR_G(r1) > (S16)n ? 2 : 0;

} /* end DEF_INST(compare_halfword_relative_long_long) */
#endif /*defined(FEATURE_ESAME)*/


/*-------------------------------------------------------------------*/
/* ECFE CIB   - Compare Immediate and Branch                   [RIS] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_immediate_and_branch)
{
int     r1;                             /* Register number           */
int     m3;                             /* Mask bits                 */
int     b4;                             /* Base of effective addr    */
VADR    effective_addr4;                /* Effective address         */
int     cc;                             /* Comparison result         */
BYTE    i2;                             /* Immediate value           */

    RIS_B(inst, regs, r1, i2, m3, b4, effective_addr4);

    /* Compare signed operands and set comparison result */
    cc = (S32)regs->GR_L(r1) < (S32)(S8)i2 ? 1 :
         (S32)regs->GR_L(r1) > (S32)(S8)i2 ? 2 : 0;

    /* Branch to operand address if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
        SUCCESSFUL_BRANCH(regs, effective_addr4, 6);
    else
        INST_UPDATE_PSW(regs, 6, 0);

} /* end DEF_INST(compare_immediate_and_branch) */


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* ECFC CGIB  - Compare Immediate and Branch Long              [RIS] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_immediate_and_branch_long)
{
int     r1;                             /* Register number           */
int     m3;                             /* Mask bits                 */
int     b4;                             /* Base of effective addr    */
VADR    effective_addr4;                /* Effective address         */
int     cc;                             /* Comparison result         */
BYTE    i2;                             /* Immediate value           */

    RIS_B(inst, regs, r1, i2, m3, b4, effective_addr4);

    /* Compare signed operands and set comparison result */
    cc = (S64)regs->GR_G(r1) < (S64)(S8)i2 ? 1 :
         (S64)regs->GR_G(r1) > (S64)(S8)i2 ? 2 : 0;

    /* Branch to operand address if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
        SUCCESSFUL_BRANCH(regs, effective_addr4, 6);
    else
        INST_UPDATE_PSW(regs, 6, 0);

} /* end DEF_INST(compare_immediate_and_branch_long) */
#endif /*defined(FEATURE_ESAME)*/


/*-------------------------------------------------------------------*/
/* EC7E CIJ   - Compare Immediate and Branch Relative          [RIE] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_immediate_and_branch_relative)
{
int     r1;                             /* Register numbers          */
int     m3;                             /* Mask bits                 */
BYTE    i2;                             /* Immediate operand value   */
S16     i4;                             /* 16-bit immediate offset   */
int     cc;                             /* Comparison result         */

    RIE_RMII_B(inst, regs, r1, i2, m3, i4);

    /* Compare signed operands and set comparison result */
    cc = (S32)regs->GR_L(r1) < (S32)(S8)i2 ? 1 :
         (S32)regs->GR_L(r1) > (S32)(S8)i2 ? 2 : 0;

    /* Branch to immediate offset if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
        SUCCESSFUL_RELATIVE_BRANCH(regs, 2*i4, 6);
    else
        INST_UPDATE_PSW(regs, 6, 0);

} /* end DEF_INST(compare_immediate_and_branch_relative) */


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* EC7C CGIJ  - Compare Immediate and Branch Relative Long     [RIE] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_immediate_and_branch_relative_long)
{
int     r1;                             /* Register numbers          */
int     m3;                             /* Mask bits                 */
BYTE    i2;                             /* Immediate operand value   */
S16     i4;                             /* 16-bit immediate offset   */
int     cc;                             /* Comparison result         */

    RIE_RMII_B(inst, regs, r1, i2, m3, i4);

    /* Compare signed operands and set comparison result */
    cc = (S64)regs->GR_G(r1) < (S64)(S8)i2 ? 1 :
         (S64)regs->GR_G(r1) > (S64)(S8)i2 ? 2 : 0;

    /* Branch to immediate offset if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
        SUCCESSFUL_RELATIVE_BRANCH(regs, 2*i4, 6);
    else
        INST_UPDATE_PSW(regs, 6, 0);

} /* end DEF_INST(compare_immediate_and_branch_relative_long) */
#endif /*defined(FEATURE_ESAME)*/


/*-------------------------------------------------------------------*/
/* EC72 CIT   - Compare Immediate and Trap                     [RIE] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_immediate_and_trap)
{
int     r1;                             /* Register number           */
int     m3;                             /* Mask bits                 */
int     cc;                             /* Comparison result         */
U16     i2;                             /* 16-bit immediate value    */

    RIE_RIM(inst, regs, r1, i2, m3);

    /* Compare signed operands and set comparison result */
    cc = (S32)regs->GR_L(r1) < (S32)(S16)i2 ? 1 :
         (S32)regs->GR_L(r1) > (S32)(S16)i2 ? 2 : 0;

    /* Raise data exception if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
    {
        regs->dxc = DXC_COMPARE_AND_TRAP;
        ARCH_DEP(program_interrupt) (regs, PGM_DATA_EXCEPTION);
    }

} /* end DEF_INST(compare_immediate_and_trap) */


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* EC70 CGIT  - Compare Immediate and Trap Long                [RIE] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_immediate_and_trap_long)
{
int     r1;                             /* Register number           */
int     m3;                             /* Mask bits                 */
int     cc;                             /* Comparison result         */
U16     i2;                             /* 16-bit immediate value    */

    RIE_RIM(inst, regs, r1, i2, m3);

    /* Compare signed operands and set comparison result */
    cc = (S64)regs->GR_G(r1) < (S64)(S16)i2 ? 1 :
         (S64)regs->GR_G(r1) > (S64)(S16)i2 ? 2 : 0;

    /* Raise data exception if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
    {
        regs->dxc = DXC_COMPARE_AND_TRAP;
        ARCH_DEP(program_interrupt) (regs, PGM_DATA_EXCEPTION);
    }

} /* end DEF_INST(compare_immediate_and_trap_long) */
#endif /*defined(FEATURE_ESAME)*/


/*-------------------------------------------------------------------*/
/* ECF7 CLRB  - Compare Logical and Branch Register            [RRS] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_logical_and_branch_register)
{
int     r1, r2;                         /* Register numbers          */
int     m3;                             /* Mask bits                 */
int     b4;                             /* Base of effective addr    */
VADR    effective_addr4;                /* Effective address         */
int     cc;                             /* Comparison result         */

    RRS_B(inst, regs, r1, r2, m3, b4, effective_addr4);

    /* Compare unsigned operands and set comparison result */
    cc = regs->GR_L(r1) < regs->GR_L(r2) ? 1 :
         regs->GR_L(r1) > regs->GR_L(r2) ? 2 : 0;

    /* Branch to operand address if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
        SUCCESSFUL_BRANCH(regs, effective_addr4, 6);
    else
        INST_UPDATE_PSW(regs, 6, 0);

} /* end DEF_INST(compare_logical_and_branch_register) */


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* ECE5 CLGRB - Compare Logical and Branch Long Register       [RRS] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_logical_and_branch_long_register)
{
int     r1, r2;                         /* Register numbers          */
int     m3;                             /* Mask bits                 */
int     b4;                             /* Base of effective addr    */
VADR    effective_addr4;                /* Effective address         */
int     cc;                             /* Comparison result         */

    RRS_B(inst, regs, r1, r2, m3, b4, effective_addr4);

    /* Compare unsigned operands and set comparison result */
    cc = regs->GR_G(r1) < regs->GR_G(r2) ? 1 :
         regs->GR_G(r1) > regs->GR_G(r2) ? 2 : 0;

    /* Branch to operand address if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
        SUCCESSFUL_BRANCH(regs, effective_addr4, 6);
    else
        INST_UPDATE_PSW(regs, 6, 0);

} /* end DEF_INST(compare_logical_and_branch_long_register) */
#endif /*defined(FEATURE_ESAME)*/


/*-------------------------------------------------------------------*/
/* EC77 CLRJ  - Compare Logical and Branch Relative Register   [RIE] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_logical_and_branch_relative_register)
{
int     r1, r2;                         /* Register numbers          */
int     m3;                             /* Mask bits                 */
S16     i4;                             /* 16-bit immediate offset   */
int     cc;                             /* Comparison result         */

    RIE_RRIM_B(inst, regs, r1, r2, i4, m3);

    /* Compare unsigned operands and set comparison result */
    cc = regs->GR_L(r1) < regs->GR_L(r2) ? 1 :
         regs->GR_L(r1) > regs->GR_L(r2) ? 2 : 0;

    /* Branch to immediate offset if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
        SUCCESSFUL_RELATIVE_BRANCH(regs, 2*i4, 6);
    else
        INST_UPDATE_PSW(regs, 6, 0);

} /* end DEF_INST(compare_logical_and_branch_relative_register) */


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* EC65 CLGRJ - Compare Logical and Branch Relative Long Reg   [RIE] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_logical_and_branch_relative_long_register)
{
int     r1, r2;                         /* Register numbers          */
int     m3;                             /* Mask bits                 */
S16     i4;                             /* 16-bit immediate offset   */
int     cc;                             /* Comparison result         */

    RIE_RRIM_B(inst, regs, r1, r2, i4, m3);

    /* Compare unsigned operands and set comparison result */
    cc = regs->GR_G(r1) < regs->GR_G(r2) ? 1 :
         regs->GR_G(r1) > regs->GR_G(r2) ? 2 : 0;

    /* Branch to immediate offset if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
        SUCCESSFUL_RELATIVE_BRANCH(regs, 2*i4, 6);
    else
        INST_UPDATE_PSW(regs, 6, 0);

} /* end DEF_INST(compare_logical_and_branch_relative_long_register) */
#endif /*defined(FEATURE_ESAME)*/


/*-------------------------------------------------------------------*/
/* B973 CLRT  - Compare Logical and Trap Register              [RRF] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_logical_and_trap_register)
{
int     r1, r2;                         /* Register numbers          */
int     m3;                             /* Mask bits                 */
int     cc;                             /* Comparison result         */

    RRF_M(inst, regs, r1, r2, m3);

    /* Compare unsigned operands and set comparison result */
    cc = regs->GR_L(r1) < regs->GR_L(r2) ? 1 :
         regs->GR_L(r1) > regs->GR_L(r2) ? 2 : 0;

    /* Raise data exception if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
    {
        regs->dxc = DXC_COMPARE_AND_TRAP;
        ARCH_DEP(program_interrupt) (regs, PGM_DATA_EXCEPTION);
    }

} /* end DEF_INST(compare_logical_and_trap_register) */


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* B961 CLGRT - Compare Logical and Trap Long Register         [RRF] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_logical_and_trap_long_register)
{
int     r1, r2;                         /* Register numbers          */
int     m3;                             /* Mask bits                 */
int     cc;                             /* Comparison result         */

    RRF_M(inst, regs, r1, r2, m3);

    /* Compare unsigned operands and set comparison result */
    cc = regs->GR_G(r1) < regs->GR_G(r2) ? 1 :
         regs->GR_G(r1) > regs->GR_G(r2) ? 2 : 0;

    /* Raise data exception if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
    {
        regs->dxc = DXC_COMPARE_AND_TRAP;
        ARCH_DEP(program_interrupt) (regs, PGM_DATA_EXCEPTION);
    }

} /* end DEF_INST(compare_logical_and_trap_long_register) */
#endif /*defined(FEATURE_ESAME)*/


/*-------------------------------------------------------------------*/
/* ECFF CLIB  - Compare Logical Immediate and Branch           [RIS] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_logical_immediate_and_branch)
{
int     r1;                             /* Register number           */
int     m3;                             /* Mask bits                 */
int     b4;                             /* Base of effective addr    */
VADR    effective_addr4;                /* Effective address         */
int     cc;                             /* Comparison result         */
BYTE    i2;                             /* Immediate value           */

    RIS_B(inst, regs, r1, i2, m3, b4, effective_addr4);

    /* Compare unsigned operands and set comparison result */
    cc = regs->GR_L(r1) < i2 ? 1 :
         regs->GR_L(r1) > i2 ? 2 : 0;

    /* Branch to operand address if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
        SUCCESSFUL_BRANCH(regs, effective_addr4, 6);
    else
        INST_UPDATE_PSW(regs, 6, 0);

} /* end DEF_INST(compare_logical_immediate_and_branch) */


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* ECFD CLGIB - Compare Logical Immediate and Branch Long      [RIS] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_logical_immediate_and_branch_long)
{
int     r1;                             /* Register number           */
int     m3;                             /* Mask bits                 */
int     b4;                             /* Base of effective addr    */
VADR    effective_addr4;                /* Effective address         */
int     cc;                             /* Comparison result         */
BYTE    i2;                             /* Immediate value           */

    RIS_B(inst, regs, r1, i2, m3, b4, effective_addr4);

    /* Compare unsigned operands and set comparison result */
    cc = regs->GR_G(r1) < i2 ? 1 :
         regs->GR_G(r1) > i2 ? 2 : 0;

    /* Branch to operand address if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
        SUCCESSFUL_BRANCH(regs, effective_addr4, 6);
    else
        INST_UPDATE_PSW(regs, 6, 0);

} /* end DEF_INST(compare_logical_immediate_and_branch_long) */
#endif /*defined(FEATURE_ESAME)*/


/*-------------------------------------------------------------------*/
/* EC7F CLIJ  - Compare Logical Immediate and Branch Relative  [RIE] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_logical_immediate_and_branch_relative)
{
int     r1;                             /* Register number           */
int     m3;                             /* Mask bits                 */
BYTE    i2;                             /* Immediate operand value   */
S16     i4;                             /* 16-bit immediate offset   */
int     cc;                             /* Comparison result         */

    RIE_RMII_B(inst, regs, r1, i2, m3, i4);

    /* Compare unsigned operands and set comparison result */
    cc = regs->GR_L(r1) < i2 ? 1 :
         regs->GR_L(r1) > i2 ? 2 : 0;

    /* Branch to immediate offset if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
        SUCCESSFUL_RELATIVE_BRANCH(regs, 2*i4, 6);
    else
        INST_UPDATE_PSW(regs, 6, 0);

} /* end DEF_INST(compare_logical_immediate_and_branch_relative) */


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* EC7D CLGIJ - Compare Logical Immed and Branch Relative Long [RIE] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_logical_immediate_and_branch_relative_long)
{
int     r1;                             /* Register number           */
int     m3;                             /* Mask bits                 */
BYTE    i2;                             /* Immediate operand value   */
S16     i4;                             /* 16-bit immediate offset   */
int     cc;                             /* Comparison result         */

    RIE_RMII_B(inst, regs, r1, i2, m3, i4);

    /* Compare unsigned operands and set comparison result */
    cc = regs->GR_G(r1) < i2 ? 1 :
         regs->GR_G(r1) > i2 ? 2 : 0;

    /* Branch to immediate offset if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
        SUCCESSFUL_RELATIVE_BRANCH(regs, 2*i4, 6);
    else
        INST_UPDATE_PSW(regs, 6, 0);

} /* end DEF_INST(compare_logical_immediate_and_branch_relative_long) */
#endif /*defined(FEATURE_ESAME)*/


/*-------------------------------------------------------------------*/
/* EC73 CLFIT - Compare Logical Immediate and Trap Fullword    [RIE] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_logical_immediate_and_trap_fullword)
{
int     r1;                             /* Register number           */
int     m3;                             /* Mask bits                 */
int     cc;                             /* Comparison result         */
U16     i2;                             /* 16-bit immediate value    */

    RIE_RIM(inst, regs, r1, i2, m3);

    /* Compare unsigned operands and set comparison result */
    cc = regs->GR_L(r1) < i2 ? 1 :
         regs->GR_L(r1) > i2 ? 2 : 0;

    /* Raise data exception if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
    {
        regs->dxc = DXC_COMPARE_AND_TRAP;
        ARCH_DEP(program_interrupt) (regs, PGM_DATA_EXCEPTION);
    }

} /* end DEF_INST(compare_logical_immediate_and_trap_fullword) */


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* EC71 CLGIT - Compare Logical Immediate and Trap Long        [RIE] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_logical_immediate_and_trap_long)
{
int     r1;                             /* Register number           */
int     m3;                             /* Mask bits                 */
int     cc;                             /* Comparison result         */
U16     i2;                             /* 16-bit immediate value    */

    RIE_RIM(inst, regs, r1, i2, m3);

    /* Compare unsigned operands and set comparison result */
    cc = regs->GR_G(r1) < i2 ? 1 :
         regs->GR_G(r1) > i2 ? 2 : 0;

    /* Raise data exception if m3 mask bit is set */
    if ((0x8 >> cc) & m3)
    {
        regs->dxc = DXC_COMPARE_AND_TRAP;
        ARCH_DEP(program_interrupt) (regs, PGM_DATA_EXCEPTION);
    }

} /* end DEF_INST(compare_logical_immediate_and_trap_long) */
#endif /*defined(FEATURE_ESAME)*/


/*-------------------------------------------------------------------*/
/* E55D CLFHSI - Compare Logical Immediate Fullword Storage    [SIL] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_logical_immediate_fullword_storage)
{
int     b1;                             /* Base of effective addr    */
VADR    effective_addr1;                /* Effective address         */
U16     i2;                             /* 16-bit immediate value    */
U32     n;                              /* 32-bit storage value      */

    SIL(inst, regs, i2, b1, effective_addr1);

    /* Load 32-bit value from first operand address */
    n = ARCH_DEP(vfetch4) ( effective_addr1, b1, regs );

    /* Compare unsigned operands and set condition code */
    regs->psw.cc = n < i2 ? 1 : n > i2 ? 2 : 0;

} /* end DEF_INST(compare_logical_immediate_fullword_storage) */


/*-------------------------------------------------------------------*/
/* E555 CLHHSI - Compare Logical Immediate Halfword Storage    [SIL] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_logical_immediate_halfword_storage)
{
int     b1;                             /* Base of effective addr    */
VADR    effective_addr1;                /* Effective address         */
U16     i2;                             /* 16-bit immediate value    */
U16     n;                              /* 16-bit storage value      */

    SIL(inst, regs, i2, b1, effective_addr1);

    /* Load 16-bit value from first operand address */
    n = ARCH_DEP(vfetch2) ( effective_addr1, b1, regs );

    /* Compare unsigned operands and set condition code */
    regs->psw.cc = n < i2 ? 1 : n > i2 ? 2 : 0;

} /* end DEF_INST(compare_logical_immediate_halfword_storage) */


/*-------------------------------------------------------------------*/
/* E559 CLGHSI - Compare Logical Immediate Long Storage        [SIL] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_logical_immediate_long_storage)
{
int     b1;                             /* Base of effective addr    */
VADR    effective_addr1;                /* Effective address         */
U16     i2;                             /* 16-bit immediate value    */
U64     n;                              /* 64-bit storage value      */

    SIL(inst, regs, i2, b1, effective_addr1);

    /* Load 64-bit value from first operand address */
    n = ARCH_DEP(vfetch8) ( effective_addr1, b1, regs );

    /* Compare unsigned operands and set condition code */
    regs->psw.cc = n < i2 ? 1 : n > i2 ? 2 : 0;

} /* end DEF_INST(compare_logical_immediate_long_storage) */


/*-------------------------------------------------------------------*/
/* C6xF CLRL  - Compare Logical Relative Long                  [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_logical_relative_long)
{
int     r1;                             /* Register number           */
VADR    addr2;                          /* Relative operand address  */
U32     n;                              /* Relative operand value    */

    RIL_A(inst, regs, r1, addr2);

    /* Program check if operand not on fullword boundary */
    FW_CHECK(addr2, regs);

    /* Load relative operand from instruction address space */
    n = ARCH_DEP(vfetch4) ( addr2, USE_INST_SPACE, regs );

    /* Compare signed operands and set condition code */
    regs->psw.cc =
            regs->GR_L(r1) < n ? 1 :
            regs->GR_L(r1) > n ? 2 : 0;

} /* end DEF_INST(compare_logical_relative_long) */


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* C6xA CLGRL - Compare Logical Relative Long Long             [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_logical_relative_long_long)
{
int     r1;                             /* Register number           */
VADR    addr2;                          /* Relative operand address  */
U64     n;                              /* Relative operand value    */

    RIL_A(inst, regs, r1, addr2);

    /* Program check if operand not on doubleword boundary */
    DW_CHECK(addr2, regs);

    /* Load relative operand from instruction address space */
    n = ARCH_DEP(vfetch8) ( addr2, USE_INST_SPACE, regs );

    /* Compare signed operands and set condition code */
    regs->psw.cc =
            regs->GR_G(r1) < n ? 1 :
            regs->GR_G(r1) > n ? 2 : 0;

} /* end DEF_INST(compare_logical_relative_long_long) */
#endif /*defined(FEATURE_ESAME)*/


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* C6xE CLGFRL - Compare Logical Relative Long Long Fullword   [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_logical_relative_long_long_fullword)
{
int     r1;                             /* Register number           */
VADR    addr2;                          /* Relative operand address  */
U32     n;                              /* Relative operand value    */

    RIL_A(inst, regs, r1, addr2);

    /* Program check if operand not on fullword boundary */
    FW_CHECK(addr2, regs);

    /* Load relative operand from instruction address space */
    n = ARCH_DEP(vfetch4) ( addr2, USE_INST_SPACE, regs );

    /* Compare signed operands and set condition code */
    regs->psw.cc =
            regs->GR_G(r1) < n ? 1 :
            regs->GR_G(r1) > n ? 2 : 0;

} /* end DEF_INST(compare_logical_relative_long_long_fullword) */
#endif /*defined(FEATURE_ESAME)*/


/*-------------------------------------------------------------------*/
/* C6x7 CLHRL - Compare Logical Halfword Relative Long         [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_logical_relative_long_halfword)
{
int     r1;                             /* Register number           */
VADR    addr2;                          /* Relative operand address  */
U16     n;                              /* Relative operand value    */

    RIL_A(inst, regs, r1, addr2);

    /* Load relative operand from instruction address space */
    n = ARCH_DEP(vfetch2) ( addr2, USE_INST_SPACE, regs );

    /* Compare signed operands and set condition code */
    regs->psw.cc =
            regs->GR_L(r1) < n ? 1 :
            regs->GR_L(r1) > n ? 2 : 0;

} /* end DEF_INST(compare_logical_relative_long_halfword) */


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* C6x6 CLGHRL - Compare Logical Halfword Relative Long Long   [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_logical_relative_long_long_halfword)
{
int     r1;                             /* Register number           */
VADR    addr2;                          /* Relative operand address  */
U16     n;                              /* Relative operand value    */

    RIL_A(inst, regs, r1, addr2);

    /* Load relative operand from instruction address space */
    n = ARCH_DEP(vfetch2) ( addr2, USE_INST_SPACE, regs );

    /* Compare signed operands and set condition code */
    regs->psw.cc =
            regs->GR_G(r1) < n ? 1 :
            regs->GR_G(r1) > n ? 2 : 0;

} /* end DEF_INST(compare_logical_relative_long_long_halfword) */
#endif /*defined(FEATURE_ESAME)*/


/*-------------------------------------------------------------------*/
/* C6xD CRL   - Compare Relative Long                          [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_relative_long)
{
int     r1;                             /* Register number           */
VADR    addr2;                          /* Relative operand address  */
U32     n;                              /* Relative operand value    */

    RIL_A(inst, regs, r1, addr2);

    /* Program check if operand not on fullword boundary */
    FW_CHECK(addr2, regs);

    /* Load relative operand from instruction address space */
    n = ARCH_DEP(vfetch4) ( addr2, USE_INST_SPACE, regs );

    /* Compare signed operands and set condition code */
    regs->psw.cc =
            (S32)regs->GR_L(r1) < (S32)n ? 1 :
            (S32)regs->GR_L(r1) > (S32)n ? 2 : 0;

} /* end DEF_INST(compare_relative_long) */


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* C6x8 CGRL  - Compare Relative Long Long                     [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_relative_long_long)
{
int     r1;                             /* Register number           */
VADR    addr2;                          /* Relative operand address  */
U64     n;                              /* Relative operand value    */

    RIL_A(inst, regs, r1, addr2);

    /* Program check if operand not on doubleword boundary */
    DW_CHECK(addr2, regs);

    /* Load relative operand from instruction address space */
    n = ARCH_DEP(vfetch8) ( addr2, USE_INST_SPACE, regs );

    /* Compare signed operands and set condition code */
    regs->psw.cc =
            (S64)regs->GR_G(r1) < (S64)n ? 1 :
            (S64)regs->GR_G(r1) > (S64)n ? 2 : 0;

} /* end DEF_INST(compare_relative_long_long) */
#endif /*defined(FEATURE_ESAME)*/


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* C6xC CGFRL - Compare Relative Long Long Fullword            [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(compare_relative_long_long_fullword)
{
int     r1;                             /* Register number           */
VADR    addr2;                          /* Relative operand address  */
U32     n;                              /* Relative operand value    */

    RIL_A(inst, regs, r1, addr2);

    /* Program check if operand not on fullword boundary */
    FW_CHECK(addr2, regs);

    /* Load relative operand from instruction address space */
    n = ARCH_DEP(vfetch4) ( addr2, USE_INST_SPACE, regs );

    /* Compare signed operands and set condition code */
    regs->psw.cc =
            (S64)regs->GR_G(r1) < (S32)n ? 1 :
            (S64)regs->GR_G(r1) > (S32)n ? 2 : 0;

} /* end DEF_INST(compare_relative_long_long_fullword) */
#endif /*defined(FEATURE_ESAME)*/


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* EB4C ECAG  - Extract Cache Attribute                        [RSY] */
/*-------------------------------------------------------------------*/
DEF_INST(extract_cache_attribute)
{
int     r1, r3;                         /* Register numbers          */
int     b2;                             /* Base of effective addr    */
VADR    effective_addr2;                /* Effective address         */
int     ai, li, ti;                     /* Operand address subfields */

    RSY(inst, regs, r1, r3, b2, effective_addr2);

    /* Address bit 63 contains the Type Indication (TI) */
    ti = effective_addr2 & 0x1;

    /* Address bits 60-62 contain the Level Indication (LI) */
    li = (effective_addr2 >> 1) & 0x7;

    /* Address bits 56-59 contain the Attribute Indication (AI) */
    ai = (effective_addr2 >> 4) & 0xF;

    //logmsg ("ECAG ai=%d li=%d ti=%d\n", ai, li, ti);

    /* If reserved bits 40-55 are not zero then set r1 to all ones */
    if ((effective_addr2 & 0xFFFF00) != 0)
    {
        regs->GR(r1) = 0xFFFFFFFFFFFFFFFFULL;
        return;
    }

    /* If AI=0 (topology summary) is requested, set register r1 to
       indicate that cache level 0 is private to this CPU and that
       cache levels 1-7 are not implemented */
    if (ai == 0)
    {
        regs->GR_H(r1) = 0x04000000;
        regs->GR_L(r1) = 0x00000000;
        return;
    }

    /* If cache level is not 0, set register r1 to all ones which
       indicates that the requested cache level is not implemented */
    if (li > 0)
    {
        regs->GR(r1) = 0xFFFFFFFFFFFFFFFFULL;
        return;
    }

    /* If AI=1 (cache line size) is requested for cache level 0
       set register r1 to indicate a fictitious cache line size */
    if (ai == 1 && li == 0)
    {
        regs->GR(r1) = 256;
        return;
    }

    /* If AI=2 (total cache size) is requested for cache level 0 
       set register r1 to indicate a fictitious total cache size */
    if (ai == 2 && li == 0)
    {
        regs->GR(r1) = 256 * 2048;
        return;
    }

    /* Set register r1 to all ones indicating that the requested
       attribute indication is reserved */
    regs->GR(r1) = 0xFFFFFFFFFFFFFFFFULL;

} /* end DEF_INST(extract_cache_attribute) */
#endif /*defined(FEATURE_ESAME)*/


#if defined(FEATURE_ACCESS_REGISTERS)
/*-------------------------------------------------------------------*/
/* E375 LAEY  - Load Address Extended (Long Displacement)      [RXY] */
/*-------------------------------------------------------------------*/
DEF_INST(load_address_extended_y)
{
int     r1;                             /* Value of R field          */
int     b2;                             /* Base of effective addr    */
VADR    effective_addr2;                /* Effective address         */

    RXY0(inst, regs, r1, b2, effective_addr2);

    /* Load operand address into register */
    SET_GR_A(r1, regs,effective_addr2);

    /* Load corresponding value into access register */
    if ( PRIMARY_SPACE_MODE(&(regs->psw)) )
        regs->AR(r1) = ALET_PRIMARY;
    else if ( SECONDARY_SPACE_MODE(&(regs->psw)) )
        regs->AR(r1) = ALET_SECONDARY;
    else if ( HOME_SPACE_MODE(&(regs->psw)) )
        regs->AR(r1) = ALET_HOME;
    else /* ACCESS_REGISTER_MODE(&(regs->psw)) */
        regs->AR(r1) = (b2 == 0) ? 0 : regs->AR(b2);
    SET_AEA_AR(regs, r1);

} /* end DEF_INST(load_address_extended_y) */
#endif /*defined(FEATURE_ACCESS_REGISTERS)*/


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* E332 LTGF  - Load and Test Long Fullword                    [RXY] */
/*-------------------------------------------------------------------*/
DEF_INST(load_and_test_long_fullword)
{
int     r1;                             /* Value of R field          */
int     b2;                             /* Base of effective addr    */
VADR    effective_addr2;                /* Effective address         */
U32     n;                              /* Second operand value      */

    RXY(inst, regs, r1, b2, effective_addr2);

    /* Load R1 register from sign-extended second operand */
    n = ARCH_DEP(vfetch4) ( effective_addr2, b2, regs );
    regs->GR_G(r1) = (S64)(S32)n;

    /* Set condition code according to value loaded */
    regs->psw.cc = (S64)regs->GR_G(r1) < 0 ? 1 :
                   (S64)regs->GR_G(r1) > 0 ? 2 : 0;

} /* end DEF_INST(load_and_test_long_fullword) */
#endif /*defined(FEATURE_ESAME)*/


/*-------------------------------------------------------------------*/
/* C4x5 LHRL  - Load Halfword Relative Long                    [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(load_halfword_relative_long)
{
int     r1;                             /* Register number           */
VADR    addr2;                          /* Relative operand address  */
U16     n;                              /* Relative operand value    */

    RIL_A(inst, regs, r1, addr2);

    /* Load relative operand from instruction address space */
    n = ARCH_DEP(vfetch2) ( addr2, USE_INST_SPACE, regs );

    /* Sign-extend operand value and load into R1 register */
    regs->GR_L(r1) = (S32)(S16)n;

} /* end DEF_INST(load_halfword_relative_long) */


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* C4x4 LGHRL - Load Halfword Relative Long Long               [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(load_halfword_relative_long_long)
{
int     r1;                             /* Register number           */
VADR    addr2;                          /* Relative operand address  */
U16     n;                              /* Relative operand value    */

    RIL_A(inst, regs, r1, addr2);

    /* Load relative operand from instruction address space */
    n = ARCH_DEP(vfetch2) ( addr2, USE_INST_SPACE, regs );

    /* Sign-extend operand value and load into R1 register */
    regs->GR_G(r1) = (S64)(S16)n;

} /* end DEF_INST(load_halfword_relative_long_long) */
#endif /*defined(FEATURE_ESAME)*/


/*-------------------------------------------------------------------*/
/* C4x2 LLHRL - Load Logical Halfword Relative Long            [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(load_logical_halfword_relative_long)
{
int     r1;                             /* Register number           */
VADR    addr2;                          /* Relative operand address  */
U16     n;                              /* Relative operand value    */

    RIL_A(inst, regs, r1, addr2);

    /* Load relative operand from instruction address space */
    n = ARCH_DEP(vfetch2) ( addr2, USE_INST_SPACE, regs );

    /* Zero-extend operand value and load into R1 register */
    regs->GR_L(r1) = n;

} /* end DEF_INST(load_logical_halfword_relative_long) */


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* C4x6 LLGHRL - Load Logical Halfword Relative Long Long      [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(load_logical_halfword_relative_long_long)
{
int     r1;                             /* Register number           */
VADR    addr2;                          /* Relative operand address  */
U16     n;                              /* Relative operand value    */

    RIL_A(inst, regs, r1, addr2);

    /* Load relative operand from instruction address space */
    n = ARCH_DEP(vfetch2) ( addr2, USE_INST_SPACE, regs );

    /* Zero-extend operand value and load into R1 register */
    regs->GR_G(r1) = n;

} /* end DEF_INST(load_logical_halfword_relative_long_long) */
#endif /*defined(FEATURE_ESAME)*/


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* C4xE LLGFRL - Load Logical Relative Long Long Fullword      [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(load_logical_relative_long_long_fullword)
{
int     r1;                             /* Register number           */
VADR    addr2;                          /* Relative operand address  */
U32     n;                              /* Relative operand value    */

    RIL_A(inst, regs, r1, addr2);

    /* Program check if operand not on fullword boundary */
    FW_CHECK(addr2, regs);

    /* Load relative operand from instruction address space */
    n = ARCH_DEP(vfetch4) ( addr2, USE_INST_SPACE, regs );

    /* Zero-extend operand value and load into R1 register */
    regs->GR_G(r1) = n;

} /* end DEF_INST(load_logical_relative_long_long_fullword) */
#endif /*defined(FEATURE_ESAME)*/


/*-------------------------------------------------------------------*/
/* C4xD LRL   - Load Relative Long                             [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(load_relative_long)
{
int     r1;                             /* Register number           */
VADR    addr2;                          /* Relative operand address  */
U32     n;                              /* Relative operand value    */

    RIL_A(inst, regs, r1, addr2);

    /* Program check if operand not on fullword boundary */
    FW_CHECK(addr2, regs);

    /* Load relative operand from instruction address space */
    n = ARCH_DEP(vfetch4) ( addr2, USE_INST_SPACE, regs );

    /* Load operand value into R1 register */
    regs->GR_L(r1) = n;

} /* end DEF_INST(load_relative_long) */


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* C4x8 LGRL  - Load Relative Long Long                        [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(load_relative_long_long)
{
int     r1;                             /* Register number           */
VADR    addr2;                          /* Relative operand address  */
U64     n;                              /* Relative operand value    */

    RIL_A(inst, regs, r1, addr2);

    /* Program check if operand not on doubleword boundary */
    DW_CHECK(addr2, regs);

    /* Load relative operand from instruction address space */
    n = ARCH_DEP(vfetch8) ( addr2, USE_INST_SPACE, regs );

    /* Load operand value into R1 register */
    regs->GR_G(r1) = n;

} /* end DEF_INST(load_relative_long_long) */
#endif /*defined(FEATURE_ESAME)*/


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* C4xC LGFRL - Load Relative Long Long Fullword               [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(load_relative_long_long_fullword)
{
int     r1;                             /* Register number           */
VADR    addr2;                          /* Relative operand address  */
U32     n;                              /* Relative operand value    */

    RIL_A(inst, regs, r1, addr2);

    /* Program check if operand not on fullword boundary */
    FW_CHECK(addr2, regs);

    /* Load relative operand from instruction address space */
    n = ARCH_DEP(vfetch4) ( addr2, USE_INST_SPACE, regs );

    /* Sign-extend operand value and load into R1 register */
    regs->GR_G(r1) = (S64)(S32)n;

} /* end DEF_INST(load_relative_long_long_fullword) */
#endif /*defined(FEATURE_ESAME)*/


/*-------------------------------------------------------------------*/
/* E54C MVHI  - Move Fullword from Halfword Immediate          [SIL] */
/*-------------------------------------------------------------------*/
DEF_INST(move_fullword_from_halfword_immediate)
{
int     b1;                             /* Base of effective addr    */
VADR    effective_addr1;                /* Effective address         */
S16     i2;                             /* 16-bit immediate value    */
S32     n;                              /* Sign-extended value of i2 */

    SIL(inst, regs, i2, b1, effective_addr1);

    /* Sign-extend 16-bit immediate value to 32 bits */
    n = i2;

    /* Store 4-byte value at operand address */
    ARCH_DEP(vstore4) ( n, effective_addr1, b1, regs );

} /* end DEF_INST(move_fullword_from_halfword_immediate) */


/*-------------------------------------------------------------------*/
/* E544 MVHHI - Move Halfword from Halfword Immediate          [SIL] */
/*-------------------------------------------------------------------*/
DEF_INST(move_halfword_from_halfword_immediate)
{
int     b1;                             /* Base of effective addr    */
VADR    effective_addr1;                /* Effective address         */
S16     i2;                             /* 16-bit immediate value    */

    SIL(inst, regs, i2, b1, effective_addr1);

    /* Store 16-bit immediate value at operand address */
    ARCH_DEP(vstore2) ( i2, effective_addr1, b1, regs );

} /* end DEF_INST(move_halfword_from_halfword_immediate) */


/*-------------------------------------------------------------------*/
/* E548 MVGHI - Move Long from Halfword Immediate              [SIL] */
/*-------------------------------------------------------------------*/
DEF_INST(move_long_from_halfword_immediate)
{
int     b1;                             /* Base of effective addr    */
VADR    effective_addr1;                /* Effective address         */
S16     i2;                             /* 16-bit immediate value    */
S64     n;                              /* Sign-extended value of i2 */

    SIL(inst, regs, i2, b1, effective_addr1);

    /* Sign-extend 16-bit immediate value to 64 bits */
    n = i2;

    /* Store 8-byte value at operand address */
    ARCH_DEP(vstore8) ( n, effective_addr1, b1, regs );

} /* end DEF_INST(move_long_from_halfword_immediate) */


/*-------------------------------------------------------------------*/
/* E37C MHY   - Multiply Halfword (Long Displacement)          [RXY] */
/*-------------------------------------------------------------------*/
DEF_INST(multiply_halfword_y)
{
int     r1;                             /* Value of R field          */
int     b2;                             /* Base of effective addr    */
VADR    effective_addr2;                /* Effective address         */
S32     n;                              /* 32-bit operand values     */

    RXY(inst, regs, r1, b2, effective_addr2);

    /* Load 2 bytes from operand address */
    n = (S16)ARCH_DEP(vfetch2) ( effective_addr2, b2, regs );

    /* Multiply R1 register by n, ignore leftmost 32 bits of
       result, and place rightmost 32 bits in R1 register */
    mul_signed ((U32 *)&n, &(regs->GR_L(r1)), regs->GR_L(r1), n);

} /* end DEF_INST(multiply_halfword_y) */


/*-------------------------------------------------------------------*/
/* C2x1 MSFI  - Multiply Single Immediate Fullword             [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(multiply_single_immediate_fullword)
{
int     r1;                             /* Register number           */
int     opcd;                           /* Opcode                    */
U32     i2;                             /* 32-bit operand value      */

    RIL(inst, regs, r1, opcd, i2);

    /* Multiply signed operands ignoring overflow */
    regs->GR_L(r1) = (S32)regs->GR_L(r1) * (S32)i2;

} /* end DEF_INST(multiply_single_immediate_fullword) */


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* C2x0 MSGFI - Multiply Single Immediate Long Fullword        [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(multiply_single_immediate_long_fullword)
{
int     r1;                             /* Register number           */
int     opcd;                           /* Opcode                    */
U32     i2;                             /* 32-bit operand value      */

    RIL(inst, regs, r1, opcd, i2);

    /* Multiply signed operands ignoring overflow */
    regs->GR_G(r1) = (S64)regs->GR_G(r1) * (S32)i2;

} /* end DEF_INST(multiply_single_immediate_long_fullword) */
#endif /*defined(FEATURE_ESAME)*/


/*-------------------------------------------------------------------*/
/* E35C MFY   - Multiply (Long Displacement)                   [RXY] */
/*-------------------------------------------------------------------*/
DEF_INST(multiply_y)
{
int     r1;                             /* Value of R field          */
int     b2;                             /* Base of effective addr    */
VADR    effective_addr2;                /* Effective address         */
U32     n;                              /* 32-bit operand values     */

    RXY(inst, regs, r1, b2, effective_addr2);

    ODD_CHECK(r1, regs);

    /* Load second operand from operand address */
    n = ARCH_DEP(vfetch4) ( effective_addr2, b2, regs );

    /* Multiply r1+1 by n and place result in r1 and r1+1 */
    mul_signed (&(regs->GR_L(r1)), &(regs->GR_L(r1+1)),
                    regs->GR_L(r1+1),
                    n);

} /* end DEF_INST(multiply_y) */


/*-------------------------------------------------------------------*/
/* E336 PFD   - Prefetch Data                                  [RXY] */
/*-------------------------------------------------------------------*/
DEF_INST(prefetch_data)
{
int     m1;                             /* Mask value                */
int     b2;                             /* Base of effective addr    */
VADR    effective_addr2;                /* Effective address         */

    RXY(inst, regs, m1, b2, effective_addr2);

    /* The Prefetch Data instruction acts as a no-op */

} /* end DEF_INST(prefetch_data) */


/*-------------------------------------------------------------------*/
/* C6x2 PFDRL - Prefetch Data Relative Long                    [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(prefetch_data_relative_long)
{
int     m1;                             /* Mask value                */
VADR    addr2;                          /* Relative operand address  */

    RIL_A(inst, regs, m1, addr2);

    /* The Prefetch Data instruction acts as a no-op */

} /* end DEF_INST(prefetch_data_relative_long) */


#if defined(FEATURE_ESAME)
/*-------------------------------------------------------------------*/
/* Rotate Then Perform Operation On Selected Bits Long Register      */
/* Subroutine is called by RNSBG,RISBG,ROSBG,RXSBG instructions      */
/*-------------------------------------------------------------------*/
DEF_INST(rotate_then_xxx_selected_bits_long_reg)
{
int     r1, r2;                         /* Register numbers          */
int     start, end;                     /* Start and end bit number  */
U64     mask, rota, resu;               /* 64-bit work areas         */
int     n;                              /* Number of bits to shift   */
int     t_bit = 0;                      /* Test-results indicator    */
int     z_bit = 0;                      /* Zero-remaining indicator  */
int     i;                              /* Loop counter              */
BYTE    i3, i4, i5;                     /* Immediate values          */
BYTE    opcode;                         /* 2nd byte of opcode        */

    RIE_RRIII(inst, regs, r1, r2, i3, i4, i5);

    /* Extract second byte of instruction opcode */
    opcode = inst[5];

    /* Extract parameters from immediate fields */
    start = i3 & 0x3F;
    end = i4 & 0x3F;
    n = i5 & 0x3F;
    if (opcode == 0x55)
        z_bit = i4 >> 7;
    else
        t_bit = i3 >> 7;

    /* Copy value from R2 register and rotate left n bits */
    rota = (regs->GR_G(r2) << n)
            | ((n == 0) ? 0 : (regs->GR_G(r2) >> (64 - n)));

    /* Construct mask for selected bits */
    for (i=0, mask=0; i < 64; i++)
    {
        mask <<= 1;
        if (start <= end) {
            if (i >= start && i <= end) mask |= 1;
        } else {
            if (i <= end || i >= start) mask |= 1;
        }
    } /* end for(i) */

    /* Isolate selected bits of rotated second operand */
    rota &= mask;

    /* Isolate selected bits of first operand */
    resu = regs->GR_G(r1) & mask;

    /* Perform operation on selected bits */
    switch (opcode) {
    case 0x54: /* And */
        resu &= rota;
        break;
    case 0x55: /* Insert */
        resu = rota;
        break;
    case 0x56: /* Or */
        resu |= rota;
        break;
    case 0x57: /* Exclusive Or */
        resu ^= rota;
        break;
    } /* end switch(opcode) */

    /* Except RISBG set condition code according to result bits */
    if (opcode != 0x55)
        regs->psw.cc = (resu == 0) ? 0 : 1;

    /* Insert result bits into R1 register */
    if (t_bit == 0)
    {
        if (z_bit == 0)
            regs->GR_G(r1) = (regs->GR_G(r1) & ~mask) | resu;
        else
            regs->GR_G(r1) = resu;
    } /* end if(t_bit==0) */

    /* For RISBG set condition code according to signed result */
    if (opcode == 0x55)
        regs->psw.cc =
                (S64)regs->GR_G(r1) < 0 ? 1 :
                (S64)regs->GR_G(r1) > 0 ? 2 : 0;

} /* end DEF_INST(rotate_then_xxx_selected_bits_long_reg) */


/*-------------------------------------------------------------------*/
/* EC54 RNSBG - Rotate Then And Selected Bits                  [RIE] */
/*-------------------------------------------------------------------*/
DEF_INST(rotate_then_and_selected_bits_long_reg)
{
    ARCH_DEP(rotate_then_xxx_selected_bits_long_reg) (inst, regs);
} /* end DEF_INST(rotate_then_and_selected_bits_long_reg) */


/*-------------------------------------------------------------------*/
/* EC55 RISBG - Rotate Then Insert Selected Bits               [RIE] */
/*-------------------------------------------------------------------*/
DEF_INST(rotate_then_insert_selected_bits_long_reg)
{
    ARCH_DEP(rotate_then_xxx_selected_bits_long_reg) (inst, regs);
} /* end DEF_INST(rotate_then_insert_selected_bits_long_reg) */


/*-------------------------------------------------------------------*/
/* EC56 ROSBG - Rotate Then Or Selected Bits                   [RIE] */
/*-------------------------------------------------------------------*/
DEF_INST(rotate_then_or_selected_bits_long_reg)
{
    ARCH_DEP(rotate_then_xxx_selected_bits_long_reg) (inst, regs);
} /* end DEF_INST(rotate_then_or_selected_bits_long_reg) */


/*-------------------------------------------------------------------*/
/* EC57 RXSBG - Rotate Then Exclusive Or Selected Bits         [RIE] */
/*-------------------------------------------------------------------*/
DEF_INST(rotate_then_exclusive_or_selected_bits_long_reg)
{
    ARCH_DEP(rotate_then_xxx_selected_bits_long_reg) (inst, regs);
} /* end DEF_INST(rotate_then_exclusive_or_selected_bits_long_reg) */
#endif /*defined(FEATURE_ESAME)*/


/*-------------------------------------------------------------------*/
/* C4x7 STHRL - Store Halfword Relative Long                   [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(store_halfword_relative_long)
{
int     r1;                             /* Register number           */
VADR    addr2;                          /* Relative operand address  */

    RIL_A(inst, regs, r1, addr2);

    /* Store low 2 bytes of R1 register in instruction address space */
    ARCH_DEP(vstore2) ( regs->GR_LHL(r1), addr2, USE_INST_SPACE, regs );

} /* end DEF_INST(store_halfword_relative_long) */


/*-------------------------------------------------------------------*/
/* C4xF STRL  - Store Relative Long                            [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(store_relative_long)
{
int     r1;                             /* Register number           */
VADR    addr2;                          /* Relative operand address  */

    RIL_A(inst, regs, r1, addr2);

    /* Program check if operand not on fullword boundary */
    FW_CHECK(addr2, regs);

    /* Store low 4 bytes of R1 register in instruction address space */
    ARCH_DEP(vstore4) ( regs->GR_L(r1), addr2, USE_INST_SPACE, regs );

} /* end DEF_INST(store_relative_long) */


/*-------------------------------------------------------------------*/
/* C4xB STGRL - Store Relative Long Long                       [RIL] */
/*-------------------------------------------------------------------*/
DEF_INST(store_relative_long_long)
{
int     r1;                             /* Register number           */
VADR    addr2;                          /* Relative operand address  */

    RIL_A(inst, regs, r1, addr2);

    /* Program check if operand not on doubleword boundary */
    DW_CHECK(addr2, regs);

    /* Store R1 register in instruction address space */
    ARCH_DEP(vstore8) ( regs->GR_G(r1), addr2, USE_INST_SPACE, regs );

} /* end DEF_INST(store_relative_long_long) */


#endif /*defined(FEATURE_GENERAL_INSTRUCTIONS_EXTENSION_FACILITY)*/


#if !defined(_GEN_ARCH)

#if defined(_ARCHMODE2)
 #define  _GEN_ARCH _ARCHMODE2
 #include "general3.c"
#endif

#if defined(_ARCHMODE3)
 #undef   _GEN_ARCH
 #define  _GEN_ARCH _ARCHMODE3
 #include "general3.c"
#endif

#endif /*!defined(_GEN_ARCH)*/
