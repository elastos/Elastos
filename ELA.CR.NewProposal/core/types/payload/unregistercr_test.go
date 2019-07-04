package payload

import (
	"bytes"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestUnregisterCR_Deserialize(t *testing.T) {
	unregisterCRPayload1 := randomUnregisterCRPayload()

	buf := new(bytes.Buffer)
	unregisterCRPayload1.Serialize(buf, UnregisterCRVersion)

	unregisterCRPayload2 := &UnregisterCR{}
	unregisterCRPayload2.Deserialize(buf, UnregisterCRVersion)

	assert.True(t, unregisterCRPayloadEqual(unregisterCRPayload1, unregisterCRPayload2))
}

func unregisterCRPayloadEqual(payload1 *UnregisterCR, payload2 *UnregisterCR) bool {
	if !bytes.Equal(payload1.Code, payload2.Code) ||
		!bytes.Equal(payload1.Signature, payload2.Signature) {
		return false
	}

	return true
}

func randomUnregisterCRPayload() *UnregisterCR {
	return &UnregisterCR{
		Code:      randomBytes(34),
		Signature: randomBytes(65),
	}
}
