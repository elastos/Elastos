package outputpayload

import (
	"bytes"
	"crypto/rand"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestMapping_Serialize(t *testing.T) {
	m := Mapping{}
	m.OwnerPublicKey = make([]byte, 33)
	m.SideProducerID = make([]byte, 33)
	buf := new(bytes.Buffer)
	err := m.Serialize(buf)
	assert.NoError(t, err)

	// 1 byte(Version)
	// 1 byte(len) + 33 bytes(OwnerPublicKey)
	// 1 byte(len) + 33 bytes(SideProducerID)
	// = 69 bytes
	assert.Equal(t, 69, buf.Len())
}

func TestMapping_Deserialize(t *testing.T) {
	m := Mapping{}
	// OwnerPublicKey over size.
	m.OwnerPublicKey = make([]byte, 34)
	m.SideProducerID = make([]byte, 33)
	buf := new(bytes.Buffer)
	err := m.Serialize(buf)
	assert.NoError(t, err)
	err = m.Deserialize(buf)
	assert.Error(t, err)

	// SideProducerID over size.
	m.OwnerPublicKey = make([]byte, 33)
	m.SideProducerID = make([]byte, 257)
	buf = new(bytes.Buffer)
	err = m.Serialize(buf)
	assert.NoError(t, err)
	err = m.Deserialize(buf)
	assert.Error(t, err)

	// Equal test.
	m1, m2 := Mapping{}, Mapping{}
	m1.Version = 2
	m1.OwnerPublicKey = make([]byte, 33)
	m1.SideProducerID = make([]byte, 33)
	rand.Read(m1.OwnerPublicKey)
	rand.Read(m1.SideProducerID)

	buf = new(bytes.Buffer)
	err = m1.Serialize(buf)
	assert.NoError(t, err)

	err = m2.Deserialize(buf)
	assert.NoError(t, err)

	assert.Equal(t, m1, m2)
}

func TestMapping_Validate(t *testing.T) {
	m := Mapping{}
	m.OwnerPublicKey = make([]byte, 33)
	m.SideProducerID = make([]byte, 33)
	err := m.Validate()
	assert.Error(t, err)
}
