package common

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestUint256FromHexString(t *testing.T) {
	a := "123456"
	aConverted, err := Uint256FromHexString(a)
	assert.EqualError(t, err, "[Common]: Uint256ParseFromString err, len != 64")
	if aConverted != nil {
		t.Error("result should be nil here")
	}

	b := "9dcad6d4ec2851bf522ddd301c7567caf98554a82a0bcce866de80b503909642"
	cBytes := [32]uint8{157, 202, 214, 212, 236, 40, 81, 191, 82, 45, 221, 48,
		28, 117, 103, 202, 249, 133, 84, 168, 42, 11, 204, 232, 102, 222, 128,
		181, 3, 144, 150, 66}

	c, err := Uint256FromHexString(b)

	if err != nil {
		t.Error("should not have error:", err)
	}

	if c == nil {
		t.Error("should not be null here:", err)
	}

	if *c != Uint256(cBytes) {
		t.Error("did not convert correctly:", c.Bytes())
	}
}

func TestUint256_IsEqual(t *testing.T) {
	u1 := Uint256{}
	u2 := Uint256{}

	for i := 0; i < 32; i++ {
		u1[i] = byte(i)
		u2[i] = byte(i)
	}

	assert.Equal(t, true, u1.IsEqual(u2))
	assert.Equal(t, true, (&u1).IsEqual(u2))
}
