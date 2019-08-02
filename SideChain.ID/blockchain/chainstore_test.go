package blockchain

import (
	"bytes"
	"crypto/rand"
	"encoding/hex"
	"os"
	"testing"

	"github.com/elastos/Elastos.ELA.SideChain.ID/params"
	"github.com/elastos/Elastos.ELA.SideChain.ID/types"

	stype "github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/stretchr/testify/assert"
)

func TestIDChainStore_PersistDIDTx(t *testing.T) {
	idChainStore, err := NewChainStore(params.GenesisBlock, "Chain_UnitTest")
	if err != nil {
		os.Exit(1)
	}

	// prepare data for test
	regPayload1 := randomPayloadDID()
	buf1 := new(bytes.Buffer)
	regPayload1.Serialize(buf1, types.DIDInfoVersion)
	id1, _ := hex.DecodeString(regPayload1.PayloadInfo.ID)
	tx1 := &stype.Transaction{
		Payload: regPayload1,
	}

	regPayload2 := randomPayloadDID()
	buf2 := new(bytes.Buffer)
	regPayload2.Serialize(buf2, types.DIDInfoVersion)
	id2, _ := hex.DecodeString(regPayload2.PayloadInfo.ID)
	tx2 := &stype.Transaction{
		Payload: regPayload2,
	}

	regPayload3 := randomPayloadDID()
	buf3 := new(bytes.Buffer)
	regPayload3.Serialize(buf3, types.DIDInfoVersion)
	regPayload3.PayloadInfo.ID = regPayload2.PayloadInfo.ID
	tx3 := &stype.Transaction{
		Payload: regPayload3,
	}

	// check chain store not exist did tx
	_, err = idChainStore.GetDIDTxPayload(id1)
	assert.True(t, err != nil)

	// persist register DID transaction
	batch := idChainStore.NewBatch()
	err = idChainStore.persistRegisterDIDTx(batch, id1, tx1)
	assert.True(t, err == nil)
	batch.Commit()

	// get DID transaction from chain store
	txs, err := idChainStore.GetDIDTxPayload(id1)
	assert.True(t, err == nil)
	assert.True(t, len(txs) == 1)
	assert.True(t, bytes.Equal(txs[0], buf1.Bytes()))

	p1, err := idChainStore.GetLastDIDTxPayload(id1)
	assert.True(t, err == nil)
	assert.True(t, bytes.Equal(p1, buf1.Bytes()))

	// persist register DID transaction
	err = idChainStore.persistRegisterDIDTx(batch, id2, tx2)
	assert.True(t, err == nil)
	batch.Commit()

	// get DID transaction from chain store
	txs2, err := idChainStore.GetDIDTxPayload(id2)
	assert.True(t, err == nil)
	assert.True(t, len(txs2) == 1)
	assert.True(t, bytes.Equal(txs2[0], buf2.Bytes()))

	p2, err := idChainStore.GetLastDIDTxPayload(id2)
	assert.True(t, err == nil)
	assert.True(t, bytes.Equal(p2, buf2.Bytes()))

	// persist register DID transaction
	err = idChainStore.persistRegisterDIDTx(batch, id2, tx3)
	assert.True(t, err == nil)
	batch.Commit()

	// get DID transaction from chain store
	txs3, err := idChainStore.GetDIDTxPayload(id2)
	assert.True(t, err == nil)
	assert.True(t, len(txs3) == 2)
	assert.True(t, bytes.Equal(txs3[0], buf3.Bytes()))
	assert.True(t, bytes.Equal(txs3[1], buf2.Bytes()))

	p3, err := idChainStore.GetLastDIDTxPayload(id2)
	assert.True(t, err == nil)
	assert.True(t, bytes.Equal(p3, buf3.Bytes()))

	// rollback tx2 will return error, tx2 is not the last one
	err = idChainStore.rollbackRegisterDIDTx(batch, id2, tx2)
	assert.True(t, err != nil)
	batch.Commit()

	p4, err := idChainStore.GetLastDIDTxPayload(id2)
	assert.True(t, err == nil)
	assert.True(t, bytes.Equal(p4, buf3.Bytes()))

	// rollback tx3
	err = idChainStore.rollbackRegisterDIDTx(batch, id2, tx3)
	assert.True(t, err == nil)
	batch.Commit()

	txs4, err := idChainStore.GetDIDTxPayload(id2)
	assert.True(t, err == nil)
	assert.True(t, len(txs4) == 1)
	assert.True(t, bytes.Equal(txs4[0], buf2.Bytes()))

	p5, err := idChainStore.GetLastDIDTxPayload(id2)
	assert.True(t, err == nil)
	assert.True(t, bytes.Equal(p5, buf2.Bytes()))

	// rollback tx2
	err = idChainStore.rollbackRegisterDIDTx(batch, id2, tx2)
	assert.True(t, err == nil)
	batch.Commit()
	_, err = idChainStore.GetDIDTxPayload(id2)
	assert.True(t, err != nil)
	_, err = idChainStore.GetLastDIDTxPayload(id2)
	assert.True(t, err != nil)

	// rollback tx1
	err = idChainStore.rollbackRegisterDIDTx(batch, id1, tx1)
	assert.True(t, err == nil)
	batch.Commit()
	_, err = idChainStore.GetDIDTxPayload(id1)
	assert.True(t, err != nil)
	_, err = idChainStore.GetLastDIDTxPayload(id1)
	assert.True(t, err != nil)
}

func randomPayloadDID() *types.PayloadDIDInfo {
	info := &types.DIDPayloadInfo{
		ID: randomString(),
		PublicKey: []types.DIDPublicKeyInfo{
			{
				ID:              randomString(),
				Type:            randomString(),
				Controller:      randomString(),
				PublicKeyBase58: randomString(),
			},
		},
		Authentication: []interface{}{
			randomString(),
			randomString(),
		},
		Authorization: []interface{}{
			randomString(),
			randomString(),
		},
	}
	return &types.PayloadDIDInfo{
		Header: types.DIDHeaderInfo{
			Specification: randomString(),
			Operation:     randomString(),
		},
		Payload: randomString(),
		Proof: types.DIDProofInfo{
			Type:               randomString(),
			VerificationMethod: randomString(),
			Signature:          randomString(),
		},
		PayloadInfo: info,
	}
}

func randomString() string {
	a := make([]byte, 20)
	rand.Read(a)
	return common.BytesToHexString(a)
}
