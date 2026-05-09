//
// Created by Richard Marks on 5/8/26.
//
#pragma once

#include <cstdint>

namespace m9
{
  enum class OpCode : uint32_t
  {
    //
    // system
    //

    // do nothing
    NO = 0x00,

    // execute system operation
    SO = 0xFD,

    // wait for current frame to be rendered
    WF = 0xFE,

    // halt execution
    HE = 0xFF,

    //
    // data management
    //

    // load memory into register
    LD = 0x02,
    LN = 0x0A,

    // store register into memory
    ST = 0x03,
    SN = 0x09,

    // move immediate into register
    MV = 0x04,

    // copy register to register
    CP = 0x05,

    // push register onto stack
    PU = 0x06,

    // pop stack into register
    PO = 0x07,

    // swap two registers
    XR = 0x08,

    //
    // arithmetic and logic
    //

    // logical AND
    ND = 0x30,

    // logical OR
    OR = 0x31,

    // logical exclusive OR
    EO = 0x32,

    // logical NOT
    NT = 0x33,

    // shift left
    SL = 0x34,

    // shift right zero-fill
    SR = 0x35,

    // shift right sign-fill
    AR = 0x36,

    // add two registers
    AD = 0x37,

    // add immediate to register
    AI = 0x38,

    // subtract two registers
    SB = 0x39,

    // subtract immediate from register
    SI = 0x3A,

    // multiply two registers
    MU = 0x3B,

    // multiply immediate by register
    MI = 0x3C,

    // divide two registers and get the quotient
    DV = 0x3D,

    // divide register by immediate and get the quotient
    DI = 0x3E,

    // divide two registers and get the remainder
    DQ = 0x3F,

    // divide register by immediate and get the remainder
    QI = 0x40,

    // increment register by one
    IR = 0x41,

    // decrement register by one
    DR = 0x42,

    // sign-extend register
    SX = 0x43,

    //
    // control flow
    //

    // unconditional jump
    UJ = 0x0E,

    // jump if zero
    ZJ = 0x0F,

    // jump if not zero
    NJ = 0x15,

    // jump if less signed
    LS = 0x10,

    // jump if less unsigned
    LU = 0x11,

    // jump if equal
    EJ = 0x12,

    // call subroutine (push-jump)
    PJ = 0x13,

    // return from subroutine (jump-previous)
    JP = 0x14,
  };

}
