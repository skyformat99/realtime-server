// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include <cstdint>
#include <cstdlib>
#include <string>


inline uint32_t ConvertToFixed( float inNumber, float inMin, float inPrecision )
{
	return static_cast< int > ( ( inNumber - inMin ) / inPrecision );
}

inline float ConvertFromFixed( uint32_t inNumber, float inMin, float inPrecision )
{
	return static_cast< float >( inNumber ) * inPrecision + inMin;
}


class OutputBitStream
{
public:

	OutputBitStream() :
		mBitHead(0),
		mBuffer(nullptr)
	{
		ReallocBuffer( 1500 * 8 );
	}

	~OutputBitStream()	{ std::free( mBuffer ); }

	void		WriteBits( uint8_t inData, uint32_t inBitCount );
	void		WriteBits( const void* inData, uint32_t inBitCount );

	const 	char*	GetBufferPtr()		const	{ return mBuffer; }
	uint32_t		GetBitLength()		const	{ return mBitHead; }
	uint32_t		GetByteLength()		const	{ return ( mBitHead + 7 ) >> 3; }

	void WriteBytes( const void* inData, uint32_t inByteCount )	{ WriteBits( inData, inByteCount << 3 ); }

	/*
	void Write( uint32_t inData, uint32_t inBitCount = 32 )	{ WriteBits( &inData, inBitCount ); }
	void Write( int inData, uint32_t inBitCount = 32 )		{ WriteBits( &inData, inBitCount ); }
	void Write( float inData )								{ WriteBits( &inData, 32 ); }

	void Write( uint16_t inData, uint32_t inBitCount = 16 )	{ WriteBits( &inData, inBitCount ); }
	void Write( int16_t inData, uint32_t inBitCount = 16 )	{ WriteBits( &inData, inBitCount ); }

	void Write( uint8_t inData, uint32_t inBitCount = 8 )	{ WriteBits( &inData, inBitCount ); }
	*/

	template< typename T >
	void Write( T inData, uint32_t inBitCount = sizeof( T ) * 8 )
	{
		static_assert( std::is_arithmetic< T >::value ||
			std::is_enum< T >::value,
			"Generic Write only supports primitive data types" );
		WriteBits( &inData, inBitCount );
	}

	void 		Write( bool inData )								{ WriteBits( &inData, 1 ); }


	void Write( const std::string& inString )
	{
		uint32_t elementCount = static_cast< uint32_t >( inString.size() );
		Write( elementCount );
		for( const auto& element : inString )
		{
			Write( element );
		}
	}

private:
	void		ReallocBuffer( uint32_t inNewBitCapacity );

	char*		mBuffer;
	uint32_t	mBitHead;
	uint32_t	mBitCapacity;
};

class InputBitStream
{
public:

	InputBitStream( char* inBuffer, uint32_t inBitCount ) :
		mBuffer( inBuffer ),
		mBitCapacity( inBitCount ),
		mBitHead( 0 ),
		mIsBufferOwner( false ) {}

	InputBitStream( const InputBitStream& inOther ) :
		mBitCapacity( inOther.mBitCapacity ),
		mBitHead( inOther.mBitHead ),
		mIsBufferOwner( true )
	{
		//allocate buffer of right size
		int byteCount = mBitCapacity / 8;
		mBuffer = static_cast< char* >( malloc( byteCount ) );
		//copy
		memcpy( mBuffer, inOther.mBuffer, byteCount );
	}

	~InputBitStream()	{ if( mIsBufferOwner ) { free( mBuffer ); }; }

	const 	char*	GetBufferPtr()		const	{ return mBuffer; }
	uint32_t	GetRemainingBitCount() 	const { return mBitCapacity - mBitHead; }

	void		ReadBits( uint8_t& outData, uint32_t inBitCount );
	void		ReadBits( void* outData, uint32_t inBitCount );

	void		ReadBytes( void* outData, uint32_t inByteCount )		{ ReadBits( outData, inByteCount << 3 ); }

	template< typename T >
	void Read( T& inData, uint32_t inBitCount = sizeof( T ) * 8 )
	{
		static_assert( std::is_arithmetic< T >::value ||
			std::is_enum< T >::value,
			"Generic Read only supports primitive data types" );
		ReadBits( &inData, inBitCount );
	}

	void		Read( uint32_t& outData, uint32_t inBitCount = 32 )		{ ReadBits( &outData, inBitCount ); }
	void		Read( int& outData, uint32_t inBitCount = 32 )			{ ReadBits( &outData, inBitCount ); }
	void		Read( float& outData )									{ ReadBits( &outData, 32 ); }

	void		Read( uint16_t& outData, uint32_t inBitCount = 16 )		{ ReadBits( &outData, inBitCount ); }
	void		Read( int16_t& outData, uint32_t inBitCount = 16 )		{ ReadBits( &outData, inBitCount ); }

	void		Read( uint8_t& outData, uint32_t inBitCount = 8 )		{ ReadBits( &outData, inBitCount ); }
	void		Read( bool& outData )									{ ReadBits( &outData, 1 ); }


	void		ResetToCapacity( uint32_t inByteCapacity )				{ mBitCapacity = inByteCapacity << 3; mBitHead = 0; }


	void Read( std::string& inString )
	{
		uint32_t elementCount;
		Read( elementCount );
		inString.resize( elementCount );
		for( auto& element : inString )
		{
			Read( element );
		}
	}


private:
	char*		mBuffer;
	uint32_t	mBitHead;
	uint32_t	mBitCapacity;
	bool		mIsBufferOwner;

};
