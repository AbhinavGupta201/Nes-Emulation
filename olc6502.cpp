                                              /*  ---addressing modes--- */

// Address Mode: Absolute
// load full 16 bit address
uint8_t olc6502::ABS()
{
         uint16_t lo=read(pc);
         pc++;
         uint16_t hi=read(pc);
         pc++;
         addr_abs=(hi<<8)|lo;
         return 0;
}

// Address Mode: Absolute with X Offset
// content of x is added to the absolute address .if the page boundary is crossed then add a clock cycle
uint8_t olc6502::ABX()
{
      uint16_t lo=read(pc);
      pc++;
      uint16_t hi=read(pc);
      pc++;
      addr_abs=(hi<<8)|lo;
      addr_abs+=x;
      if((addr_abs&0xFF00) !=(hi<<8))
         return 1;
    return 0;
}

// Address Mode: Absolute with Y Offset
// same as the ABX except the y register
uint8_t olc6502::ABY()
{
    uint16_t lo=read(pc);
    pc++;
    uint16_t hi=read(pc);
    pc++;
    addr_abs=(hi<<8)|lo;
      addr_abs+=y;
    if(addr_abs&0xFF00 !=(hi<<8))
     return 1;
     return 0;
}

// Address Mode: Indirect
// The supplied 16-bit address is read to get the actual 16-bit address. This is
// instruction is unusual in that it has a bug in the hardware! To emulate its
// function accurately, we also need to emulate this bug. If the low byte of the
// supplied address is 0xFF, then to read the high byte of the actual address
// we need to cross a page boundary. This doesnt actually work on the chip as
// designed, instead it wraps back around in the same page, yielding an
// invalid actual address
uint8_t olc6502::IND()
{
    uint16_t ptr_low=read(pc);
    pc++;
    uint16_t ptr_high=read(pc);
    pc++;
    uint16_t ptr=(ptr_high<<8)|ptr_low ;
    if(ptr_low==0x00FF)  /// page boundry hardware bug
        addr_abs=( read(ptr&0x00FF)<<8 | read(ptr_low));
    else
        addr_abs=read(ptr+1)<<8 | read(ptr_low);
    return 0;

}

//address mode :Indirect X
// The supplied 8-bit address is offset by X Register to index
// a location in page 0x00. The actual 16-bit address is read
// from this location
uint8_t olc6502:: IZX()
{
    uint16_t t=read(pc);
    pc++;
    uint16_t low=read( (uint16_t )(t+(uint16_t)x)&0x00FF );
    uint16_t high =read( ( uint16_t)(t+(uint16_t)(x+1))&0x00FF) ;
    addr_abs=high<<8 | low;
    return 0;
}

// Address Mode: Indirect Y
// The supplied 8-bit address indexes a location in page 0x00. From
// here the actual 16-bit address is read, and the contents of
// Y Register is added to it to offset it. If the offset causes a
// change in page then an additional clock cycle is required.
uint8_t olc6502::IZY()
{
    uint16_t t=read(pc);
    pc++;
    uint16_t low=read( t & 0x00FF );
    uint16_t high=read( (t+1)& 0x00FF);
    addr_abs =high<<8 | low ;
    addr_abs+=y;
    if(addr_abs &0xFF00 != high<<8)
        return 1;
    return 0;
}



                                                        /* ------Instructions------*/


// Instruction: Increment Y Register
// Function:    Y = Y + 1
// Flags Out:   N, Z
uint8_t olc6502::INY()
{
    y=y+1;
    SetFlag(N,y=0x00);
    SetFlag(Z,y&0x80);
    return 0;

}

// Instruction: Jump To Location
// Function:    pc = address
uint8_t olc6502::JMP()
{
    pc=addr_abs;
    return 0;
}

// Instruction: Jump To Sub-Routine
// Function:    Push current pc to stack, pc = address
uint8_t olc6502::JSR()
{
    pc--;
    write( 0x0100 + stkp ,(pc>>8)& 0x00FF ) ;
    stkp--;
    write( 0x0100 + stkp ,pc&0x00FF);
    stkp--;

    pc=addr_abs;
    return 0;
}

// Instruction: Load The Accumulator
// Function:    A = M
// Flags Out:   N, Z
uint8_t olc6502::LDA( )
{
    fetch( ) ;

    a=fetched;
    SetFlag(N, a&0x80 );
    SetFlag( Z, a==0x00) ;
    return 0;
}

// Instruction: Load The X Register
// Function:    X = M
// Flags Out:   N, Z
uint8_t olc6502:: LDX( )
{
      fetch();
      x=fetched;
      SetFlag ( N , x&0x80 );
      SetFlag ( Z ,x==0x00);
      return 0;
}

// Instruction: Load The Y Register
// Function:    Y = M
// Flags Out:   N, Z
uint8_t olc6502 :: LDY( )
{
    fetch( );
    y=fetched ;
    SetFlag(N , y&0x80);
    SetFlag(Z, y==0x00);
    return 0;
}

// instruction: logical shift right
// LSR shifts all bits right one position. 0 is shifted into bit 7 and the original bit 0 is shifted into the Carry.
// flages out : Z,N ,C
uint8_t olc6502::LSR()
{
    fetch( );
    SetFlag( C , fetched & 0x0001) ;
    temp=fetched>>1;
    SetFlag( Z, ( temp&&0x00FF)==0x0000) ;
    SetFlag( N , temp&0x0080);
    if (lookup[opcode].addrmode == &olc6502::IMP)
		a = temp & 0x00FF;
    else
        write(addr_abs, temp & 0x00FF);
    return 0;


}

//NOP is a mnemonic that stands for “No Operation”. This instruction does nothing during execution
uint8_t olc6502::NOP()
{
    switch(opcode)
    {
    case 0x1C:
    case 0x3C:
    case 0x5C:
    case 0x7C:
    case 0xDC:
    case 0xFC:
       return 1;
    }
    return 0;

}

// Instruction: Bitwise Logic OR
// Function:    A = A | M
// Flags Out:   N, Z
uint8_t olc6502::ORA()
{
    fetch( );
    a=a|fetched ;
    SetFlag( Z , a==0x00);
    SetFlag( N , a&0x80);
    return 0;
}

// Instruction: Push Accumulator to Stack
// Function:    A -> stack
uint8_t olc6502::PHA()
{
     write(0x0100+stkp ,a);
     stkp--;
     return 0;
}

// Instruction: Push Status Register to Stack
// Function:    status -> stack
// Note:        Break flag is set to 1 before push
uint8_t olc6502::PHP()
{
    write(0x0100+stkp ,status | B | U);
    stkp--;
    SetFlag( B,0 );
    SetFlag(U,0 );
    return 0;
}

// Instruction: Pop Accumulator off Stack
// Function:    A <- stack
// Flags Out:   N, Z
uint8_t olc6502::PLA()
{
    stkp++;
    a=read(0x0100+stkp);
    SetFlag(N, a&0x80);
    SetFlag(Z,a==0x00);
    return 0;

}

// Instruction: Pop Status Register off Stack
// Function:    Status <- stack
uint8_t olc6502::PLP()
{
    stkp++;
    status=read(0x0100+stkp);
    SetFlag(U ,1);
    return 0;
}

/* remiaing rol and other three from the rol */
//The rotate left (ROL) and rotate through carry left (RCL) instructions shift all the bits toward more-significant bit positions,
//except for the most-significant bit, which is rotated to the least significant bit location
// set flage : N,Z,C;
uint8_t olc6502::ROL()
{
    fetch();
    temp=(uint16_t)(fetched<<1)| GetFlag(C);
    SetFlag(C,temp&0xFF00);
    SetFlag(Z , (temp&0x00FF)==0x0000);
    SetFlag(N , temp&0x0080);
    if(lookup[opcode].addrmode == &olc6502::IMP)
        a=temp&0x00FF;
    else
        write(addr_abs,temp&0x00FF);
    return 0;
}

//The rotate right (ROR) and rotate through carry right (RCR) instructions shift all the bits toward less significant bit positions
// except for the least-significant bit, which is rotated to the most-significant bit location
//set flage : C,N,Z
uint8_t olc6502::ROR()
{
    fetch();
    temp=(uint16_t)(fetched>>1)| (GetFlag(C)<<7);
    SetFlag(C ,temp&0xFF00);
    SetFlag(Z , temp&0x00FF ==0x0000);
    SetFlag(N ,temp&0x0080);
    if(lookup[opcode].addrmode == &olc6502::IMP)
        a=temp&0x00FF;
    else
        write(addr_abs ,temp&0x00FF);
    return 0;
}

//RTI:resume interrupted task
//
uint8_t olc6502::RTI()
{
    stkp++;
    status=read(0x0100+stkp);
    status&=~B;
    status &=~U; //can be left if U is not a flage
    stkp++;
    pc=(uint16_t)read(0x0100+stkp);
    stkp++;
    pc|=(uint16_t)read(0x0100+stkp)<<8;
    return 0;
}


//RTS    ---   RTS stands for Return from Subroutine
//RTS is one of the 6502 Subroutine Operations of 6502 instruction-set.
//The function of RTS is to pulls the top two bytes off the stack (low byte first)
// and transfers program control to that address+1 i.e.return (RTS)  from the calling subroutine
uint8_t olc6502::RTS()
{
    stkp++;
    pc=(uint16_t)read(0x0100+stkp);
    stkp++;
    pc|=(uint16_t)read(0x0100+stkp)<<8 ;
    pc++;
    return 0;
}

//Instruction: Set Carry Flag
// Function:    C = 1
uint8_t olc6502::SEC()
{
    SetFlag(C,true);
    return 0;
}

// Instruction: Set Decimal Flag
// Function:    D = 1
uint8_t olc6502::SED()
{
    SetFlag(D,true);
    return 0;
}

// Instruction: Set Interrupt Flag / Enable Interrupts
// Function:    I = 1
uint8_t olc6502::SEI()
{
    SetFlag(I ,true);
    return 0;
}

// Instruction: Store Accumulator at Address
// Function:    M = A
uint8_t olc6502::STA()
{
    write(addr_abs ,a);
    return 0;
}

// Instruction: Store X Register at Address
// Function:    M = X
uint8_t olc6502::STX()
{
    write(addr_abs ,x);
    return 0;
}

// Instruction: Store Y Register at Address
// Function:    M = Y
uint8_t olc6502::STY()
{
    write(addr_abs,y);
    return 0;
}

// Instruction: Transfer Accumulator to X Register
// Function:    X = A
// Flags Out:   N, Z
uint8_t olc6502::TAX()
{
    x=a;
    SetFlag(N ,a&0x80) ;
    SetFlag(Z , a==0x00);
    return 0;
}

// Instruction: Transfer Accumulator to Y Register
// Function:    Y = A
// Flags Out:   N, Z
uint8_t olc6502::TAY()
{
    y=a;
    SetFlag(N ,a&0x80);
    SetFlag(Z , a==0x00);
    return 0;
}

// Instruction: Transfer Stack Pointer to X Register
// Function:    X = stack pointer
// Flags Out:   N, Z
uint8_t olc6502::TSX()
{
    x=stkp;
    SetFlag(N , a&0x80);
    SetFlag(Z ,a==0x00) ;
    return 0;
}

// Instruction: Transfer X Register to Accumulator
// Function:    A = X
// Flags Out:   N, Z
uint8_t olc6502::TXA()
{
    a=x;
    SetFlag(N , a&0x80);
    SetFlag(Z , a==0x00);
    return 0;
}

// Instruction: Transfer X Register to Stack Pointer
// Function:    stack pointer = X
uint8_t olc6502::TXS()
{
    stkp=x;
    return 0 ;
}

// Instruction: Transfer Y Register to Accumulator
// Function:    A = Y
// Flags Out:   N, Z
uint8_t olc6502::TYA()
{
    a=y;
    SetFlag(Z , a==0x00);
    SetFlag(N , a&0x80) ;
    return 0;
}


// all illegal opcodes capturing
uint8_t olc6502 :: XXX()
{
    return 0;
}
