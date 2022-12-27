package datatype

import (
	"testing"

	"github.com/stretchr/testify/assert"

	"github.com/elastos/Elastos.ELA/common"
)

func hexToBytes(s string) []byte {
	b, err := common.HexStringToBytes(s)
	if err != nil {
		panic("invalid hex in source file: " + s)
	}
	return b
}

func TestByteArray_GetBigInteger(t *testing.T) {
	tests := []struct {
		valueByte []byte
		valueData int64
	}{
		{[]byte{}, 0},
		{[]byte{1}, 1},
		{[]byte{255}, -1},
		{[]byte{255, 254}, -257},
		{[]byte{202, 255, 254}, -65590},
		{[]byte{255, 254, 32}, 2162431},
		{[]byte{255, 254, 32, 254}, -31392001},
		{[]byte{255, 254, 32, 254, 56}, 244781743871},
		{[]byte{255, 254, 32, 254, 253}, -8621326593},
		{[]byte{255, 254, 32, 254, 253, 28}, 31877215878911},
		{[]byte{202, 255, 254, 32, 123, 242, 43}, 12370034647039946},
		{[]byte{140, 140, 15, 124, 240, 250, 2, 67}, 4828697661275081868},

		{hexToBytes("01"), 1},
		{hexToBytes("ff"), -1},
		{hexToBytes("7f"), 127},
		{hexToBytes("81"), -127},
		{hexToBytes("8000"), 128},
		{hexToBytes("80"), -128},
		{hexToBytes("8100"), 129},
		{hexToBytes("7fff"), -129},
		{hexToBytes("0001"), 256},
		{hexToBytes("ff7f"), 32767},
		{hexToBytes("0180"), -32767},
		{hexToBytes("ffff00"), 65535},
		{hexToBytes("0100ff"), -65535},
		{hexToBytes("ffffff7f"), 2147483647},
		{hexToBytes("01000080"), -2147483647},
		{hexToBytes("0000008000"), 2147483648},
		{hexToBytes("00000080"), -2147483648},
		{hexToBytes("ffffffff00"), 4294967295},
		{hexToBytes("01000000ff"), -4294967295},
		{hexToBytes("000000000100"), 4294967296},
		{hexToBytes("00000000ff"), -4294967296},
		{hexToBytes("ffffffffffffff00"), 72057594037927935},
		{hexToBytes("01000000000000ff"), -72057594037927935},
		{hexToBytes("ffffffffffffff7f"), 9223372036854775807},
		{hexToBytes("0100000000000080"), -9223372036854775807},
	}

	for _, test := range tests {
		byteArray := NewByteArray(test.valueByte)
		x1 := byteArray.GetBigInteger().Int64()
		assert.True(t, x1 == test.valueData)
	}
}
