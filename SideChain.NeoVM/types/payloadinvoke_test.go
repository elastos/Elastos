package types

import (
	"testing"
	"github.com/elastos/Elastos.ELA/common"
	"bytes"
	"github.com/stretchr/testify/assert"
)

func TestPayloadInvoke_Serialize(t *testing.T) {
	payload := PayloadInvoke{}
	payload.CodeHash = common.Uint168{1,2,3}
	payload.Code = []byte{1, 2, 3, 4, 3, 1, 2, 3, 4, 3, 1, 2, 3, 4, 3, 1, 2, 3, 4, 3, 3}
	payload.ProgramHash = common.Uint168{3,4,1}

	buf := new(bytes.Buffer)
	err := payload.Serialize(buf, 1)
	assert.NoError(t, err)

	r := bytes.NewReader(buf.Bytes())

	payload2 := PayloadInvoke{}
	err = payload2.Deserialize(r, 1)

	assert.NoError(t, err)
	assert.True(t, payload.CodeHash == payload2.CodeHash)

	assert.True(t, len(payload.Code) == len(payload2.Code))
	for i := 0; i < len(payload.Code); i++ {
		assert.True(t, payload2.Code[i] == payload.Code[i])
	}

	assert.True(t, payload.ProgramHash == payload2.ProgramHash)
}