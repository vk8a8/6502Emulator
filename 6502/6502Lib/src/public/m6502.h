#pragma once
#include <stdio.h>
#include <stdlib.h>

// http://www.obelisk.me.uk/6502/

namespace m6502
{
	using Byte = unsigned char;
	using Word = unsigned short;

	using u32 = unsigned int;
	using s32 = signed int;

	struct Mem;
	struct CPU;
}

struct m6502::Mem
{
	static constexpr u32 MAX_MEM = 1024 * 64;
	Byte Data[MAX_MEM];

	void Initialise()
	{
		for ( u32 i = 0; i < MAX_MEM; i++ )
		{
			Data[i] = 0;
		}
	}

	/** read 1 byte */
	Byte operator[]( u32 Address ) const
	{
		// assert here Address is < MAX_MEM
		return Data[Address];
	}

	/** write 1 byte */
	Byte& operator[]( u32 Address )
	{
		// assert here Address is < MAX_MEM
		return Data[Address];
	}

	/** write 2 bytes */
	void WriteWord( 
		Word Value, 
		u32 Address, 
		s32& Cycles )
	{
		Data[Address]		= Value & 0xFF;
		Data[Address + 1]   = (Value >> 8);
		Cycles -= 2;
	}
};

struct m6502::CPU
{
	Word PC;		//program counter
	Word SP;		//stack pointer

	Byte A, X, Y;	//registers

	Byte C : 1;	//status flag
	Byte Z : 1;	//status flag
	Byte I : 1;//status flag
	Byte D : 1;//status flag
	Byte B : 1;//status flag
	Byte V : 1;//status flag
	Byte N : 1;//status flag

	void Reset( Mem& memory )
	{
		Reset( 0xFFFC, memory );
	}

	void Reset( Word ResetVector, Mem& memory )
	{
		PC = ResetVector;
		SP = 0x0100;
		C = Z = I = D = B = V = N = 0;
		A = X = Y = 0;
		memory.Initialise();
	}

	Byte FetchByte( s32& Cycles, const Mem& memory )
	{
		Byte Data = memory[PC];
		PC++;
		Cycles--;
		return Data;
	}

	Word FetchWord( s32& Cycles, const Mem& memory )
	{
		// 6502 is little endian
		Word Data = memory[PC];
		PC++;
		
		Data |= (memory[PC] << 8 );
		PC++;

		Cycles -= 2;

		// if you wanted to handle endianness
		// you would have to swap bytes here
		// if ( PLATFORM_BIG_ENDIAN )
		//	SwapBytesInWord(Data)

		return Data;
	}

	Byte ReadByte(
		s32& Cycles,
		Word Address,
		const Mem& memory )
	{
		Byte Data = memory[Address];
		Cycles--;
		return Data;
	}

	Word ReadWord(
		s32& Cycles,
		Word Address,
		const Mem& memory )
	{
		Byte LoByte = ReadByte( Cycles, Address, memory );
		Byte HiByte = ReadByte( Cycles, Address + 1, memory );
		return LoByte | (HiByte << 8);
	}

	void WriteByte( Byte Value, s32& Cycles, Word Address, Mem& memory )
	{
		memory[Address] = Value;
		Cycles--;
	}

	// opcodes
	static constexpr Byte
		//LDA
		INS_LDA_IM = 0xA9,
		INS_LDA_ZP = 0xA5,
		INS_LDA_ZPX = 0xB5,
		INS_LDA_ABS = 0xAD,
		INS_LDA_ABSX = 0xBD,
		INS_LDA_ABSY = 0xB9,
		INS_LDA_INDX = 0xA1,
		INS_LDA_INDY = 0xB1,
		//LDX
		INS_LDX_IM = 0xA2,
		INS_LDX_ZP = 0xA6,
		INS_LDX_ZPY = 0xB6,
		INS_LDX_ABS = 0xAE,
		INS_LDX_ABSY = 0xBE,
		//LDY
		INS_LDY_IM = 0xA0,
		INS_LDY_ZP = 0xA4,
		INS_LDY_ZPX = 0xB4,
		INS_LDY_ABS = 0xAC,
		INS_LDY_ABSX = 0xBC,
		//STA
		INS_STA_ZP = 0x85,
		INS_STA_ZPX = 0x95,
		INS_STA_ABS = 0x8D,
		INS_STA_ABSX = 0x9D,
		INS_STA_ABSY = 0x99,
		INS_STA_INDX = 0x81,
		INS_STA_INDY = 0x91,
		//STX
		INS_STX_ZP = 0x86,
		INS_STX_ABS = 0x8E,
		//STY
		INS_STY_ZP = 0x84,
		INS_STY_ZPX = 0x94,
		INS_STY_ABS = 0x8C,
		
		INS_JSR = 0x20;

	/** Sets the correct Process status after a load register instruction
	*	- LDA, LDX, LDY
	*	@Register The A,X or Y Register */
	void LoadRegisterSetStatus( Byte Register )
	{
		Z = (Register == 0);
		N = (Register & 0b10000000) > 0;
	}

	/** @return the number of cycles that were used */
	s32 Execute( s32 Cycles, Mem& memory );

	/** Addressing mode - Zero page */
	Word AddrZeroPage( s32& Cycles, const Mem& memory );

	/** Addressing mode - Zero page with X offset */
	Word AddrZeroPageX( s32& Cycles, const Mem& memory );

	/** Addressing mode - Zero page with Y offset */
	Word AddrZeroPageY( s32& Cycles, const Mem& memory );

	/** Addressing mode - Absolute */
	Word AddrAbsolute( s32& Cycles, const Mem& memory );

	/** Addressing mode - Absolute with X offset */
	Word AddrAbsoluteX( s32& Cycles, const Mem& memory );

	/** Addressing mode - Absolute with Y offset */
	Word AddrAbsoluteY( s32& Cycles, const Mem& memory );

	/** Addressing mode - Indirect X | Indexed Indirect */
	Word AddrIndirectX( s32& Cycles, const Mem& memory );

	/** Addressing mode - Indirect Y | Indirect Indexed */
	Word AddrIndirectY( s32& Cycles, const Mem& memory );
};