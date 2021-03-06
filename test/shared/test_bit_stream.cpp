#include "../shared.hpp"

namespace test_bit_stream {

////////////////////////////////////////////////////////////////////////////////

using WriteStream  = hfsm2::detail::BitWriteStream<46>;
using ReadStream   = hfsm2::detail::BitReadStream <46>;
using StreamBuffer = typename WriteStream::Buffer;

//------------------------------------------------------------------------------

TEST_CASE("Shared.BitStream<>", "[shared]") {
	StreamBuffer buffer;

	WriteStream writeStream{buffer};
	writeStream.write< 5>((uint8_t)       27);
	writeStream.write< 4>((uint8_t)       11);
	writeStream.write< 3>((uint8_t)        5);
	writeStream.write<12>((uint16_t)    1472);
	writeStream.write<21>((uint32_t) 1000000);
	REQUIRE(buffer.bitSize == 45);

	ReadStream readStream{buffer};
	REQUIRE(readStream.read< 5>() ==      27);
	REQUIRE(readStream.read< 4>() ==      11);
	REQUIRE(readStream.read< 3>() ==       5);
	REQUIRE(readStream.read<12>() ==    1472);
	REQUIRE(readStream.read<21>() == 1000000);
	REQUIRE(readStream.cursor() == 45);
}

////////////////////////////////////////////////////////////////////////////////

}
