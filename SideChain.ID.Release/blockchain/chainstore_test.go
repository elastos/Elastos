package blockchain

import (
	"bytes"
	"crypto/rand"
	"encoding/hex"
	"encoding/json"
	"os"
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA.SideChain.ID/params"
	"github.com/elastos/Elastos.ELA.SideChain.ID/types"

	stype "github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/stretchr/testify/assert"
)

var didPayloadBytes = []byte(
	"{" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN\"," +
		"\"publicKey\": [{" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default\"," +
		"\"type\": \"ECDSAsecp256r1\"," +
		"\"controller\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN\"," +
		"\"publicKeyBase58\": \"zNxoZaZLdackZQNMas7sCkPRHZsJ3BtdjEvM2y5gNvKJ\"" +
		"}, {" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#key2\"," +
		"\"type\": \"ECDSAsecp256r1\"," +
		"\"controller\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN\"," +
		"\"publicKeyBase58\": \"273j8fQ1ZZVM6U6d5XE3X8SyULuJwjyYXbxNopXVuftBe\"" +
		"}, {" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#recovery\"," +
		"\"type\": \"ECDSAsecp256r1\"," +
		"\"controller\": \"did:elastos:ip7ntDo2metGnU8wGP4FnyKCUdbHm4BPDh\"," +
		"\"publicKeyBase58\": \"zppy33i2r3uC1LT3RFcLqJJPFpYuZPDuKMeKZ5TdAskM\"" +
		"}]," +
		"\"authentication\": [" +
		"\"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default\"," +
		"\"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#key2\"," +
		"{" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#keys3\"," +
		"\"type\": \"ECDSAsecp256r1\"," +
		"\"controller\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN\"," +
		"\"publicKeyBase58\": \"H3C2AVvLMv6gmMNam3uVAjZpfkcJCwDwnZn6z3wXmqPV\"" +
		"}]," +
		"\"authorization\": [" +
		"\"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default\"," +
		"\"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#key2\"," +
		"{" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#keys3\"," +
		"\"type\": \"ECDSAsecp256r1\"," +
		"\"controller\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN\"," +
		"\"publicKeyBase58\": \"H3C2AVvLMv6gmMNam3uVAjZpfkcJCwDwnZn6z3wXmqPV\"" +
		"}]," +
		"\"expires\": \"2024-02-10T17:00:00Z\"" +
		"}",
)
var didPayloadBytes1 = []byte(
	"{" +
		"\"id\": \"did:elastos:iXZck7tte4V1F7THQ9Z4z7xuah6j4U33zo\"," +
		"\"publicKey\": [{" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default\"," +
		"\"type\": \"ECDSAsecp256r1\"," +
		"\"controller\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN\"," +
		"\"publicKeyBase58\": \"zNxoZaZLdackZQNMas7sCkPRHZsJ3BtdjEvM2y5gNvKJ\"" +
		"}, {" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#key2\"," +
		"\"type\": \"ECDSAsecp256r1\"," +
		"\"controller\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN\"," +
		"\"publicKeyBase58\": \"273j8fQ1ZZVM6U6d5XE3X8SyULuJwjyYXbxNopXVuftBe\"" +
		"}, {" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#recovery\"," +
		"\"type\": \"ECDSAsecp256r1\"," +
		"\"controller\": \"did:elastos:ip7ntDo2metGnU8wGP4FnyKCUdbHm4BPDh\"," +
		"\"publicKeyBase58\": \"zppy33i2r3uC1LT3RFcLqJJPFpYuZPDuKMeKZ5TdAskM\"" +
		"}]," +
		"\"authentication\": [" +
		"\"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default\"," +
		"\"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#key2\"," +
		"{" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#keys3\"," +
		"\"type\": \"ECDSAsecp256r1\"," +
		"\"controller\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN\"," +
		"\"publicKeyBase58\": \"H3C2AVvLMv6gmMNam3uVAjZpfkcJCwDwnZn6z3wXmqPV\"" +
		"}]," +
		"\"authorization\": [" +
		"\"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default\"," +
		"\"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#key2\"," +
		"{" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#keys3\"," +
		"\"type\": \"ECDSAsecp256r1\"," +
		"\"controller\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN\"," +
		"\"publicKeyBase58\": \"H3C2AVvLMv6gmMNam3uVAjZpfkcJCwDwnZn6z3wXmqPV\"" +
		"}]," +
		"\"expires\": \"2024-02-10T17:00:00Z\"" +
		"}",
)
var didPayloadBytes2 = []byte(
	"{" +
		"\"id\": \"did:elastos:imfFhB5zNwTAMHh2RMpgRrZeSo7C5WtkzS\"," +
		"\"publicKey\": [{" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default\"," +
		"\"type\": \"ECDSAsecp256r1\"," +
		"\"controller\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN\"," +
		"\"publicKeyBase58\": \"zNxoZaZLdackZQNMas7sCkPRHZsJ3BtdjEvM2y5gNvKJ\"" +
		"}, {" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#key2\"," +
		"\"type\": \"ECDSAsecp256r1\"," +
		"\"controller\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN\"," +
		"\"publicKeyBase58\": \"273j8fQ1ZZVM6U6d5XE3X8SyULuJwjyYXbxNopXVuftBe\"" +
		"}, {" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#recovery\"," +
		"\"type\": \"ECDSAsecp256r1\"," +
		"\"controller\": \"did:elastos:ip7ntDo2metGnU8wGP4FnyKCUdbHm4BPDh\"," +
		"\"publicKeyBase58\": \"zppy33i2r3uC1LT3RFcLqJJPFpYuZPDuKMeKZ5TdAskM\"" +
		"}]," +
		"\"authentication\": [" +
		"\"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default\"," +
		"\"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#key2\"," +
		"{" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#keys3\"," +
		"\"type\": \"ECDSAsecp256r1\"," +
		"\"controller\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN\"," +
		"\"publicKeyBase58\": \"H3C2AVvLMv6gmMNam3uVAjZpfkcJCwDwnZn6z3wXmqPV\"" +
		"}]," +
		"\"authorization\": [" +
		"\"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default\"," +
		"\"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#key2\"," +
		"{" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#keys3\"," +
		"\"type\": \"ECDSAsecp256r1\"," +
		"\"controller\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN\"," +
		"\"publicKeyBase58\": \"H3C2AVvLMv6gmMNam3uVAjZpfkcJCwDwnZn6z3wXmqPV\"" +
		"}]," +
		"\"expires\": \"2024-02-10T17:00:00Z\"" +
		"}",
)
var didPayloadBytes3 = []byte(
	"{" +
		"\"id\": \"did:elastos:ifawgWFmZRLXN1JVmqpXcNRurhB1zyHNcf\"," +
		"\"publicKey\": [{" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default\"," +
		"\"type\": \"ECDSAsecp256r1\"," +
		"\"controller\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN\"," +
		"\"publicKeyBase58\": \"zNxoZaZLdackZQNMas7sCkPRHZsJ3BtdjEvM2y5gNvKJ\"" +
		"}, {" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#key2\"," +
		"\"type\": \"ECDSAsecp256r1\"," +
		"\"controller\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN\"," +
		"\"publicKeyBase58\": \"273j8fQ1ZZVM6U6d5XE3X8SyULuJwjyYXbxNopXVuftBe\"" +
		"}, {" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#recovery\"," +
		"\"type\": \"ECDSAsecp256r1\"," +
		"\"controller\": \"did:elastos:ip7ntDo2metGnU8wGP4FnyKCUdbHm4BPDh\"," +
		"\"publicKeyBase58\": \"zppy33i2r3uC1LT3RFcLqJJPFpYuZPDuKMeKZ5TdAskM\"" +
		"}]," +
		"\"authentication\": [" +
		"\"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default\"," +
		"\"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#key2\"," +
		"{" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#keys3\"," +
		"\"type\": \"ECDSAsecp256r1\"," +
		"\"controller\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN\"," +
		"\"publicKeyBase58\": \"H3C2AVvLMv6gmMNam3uVAjZpfkcJCwDwnZn6z3wXmqPV\"" +
		"}]," +
		"\"authorization\": [" +
		"\"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default\"," +
		"\"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#key2\"," +
		"{" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#keys3\"," +
		"\"type\": \"ECDSAsecp256r1\"," +
		"\"controller\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN\"," +
		"\"publicKeyBase58\": \"H3C2AVvLMv6gmMNam3uVAjZpfkcJCwDwnZn6z3wXmqPV\"" +
		"}]," +
		"\"expires\": \"2024-02-10T17:00:00Z\"" +
		"}",
)

func TestIDChainStore_PersistDIDTx(t *testing.T) {
	idChainStore, err := NewChainStore(params.GenesisBlock, "Chain_UnitTest")
	if err != nil {
		os.Exit(1)
	}
	var blockHeight1, blockTimeStamp1, blockHeight2, blockTimeStamp2,
		blockHeight3, blockTimeStamp3 uint32

	blockHeight1 = 1
	blockTimeStamp1 = uint32(time.Now().Unix())

	blockHeight2 = blockHeight1 + 1
	blockTimeStamp2 = blockTimeStamp1 + 1

	blockHeight3 = blockHeight2 + 1
	blockTimeStamp3 = blockTimeStamp2 + 1

	// prepare data for test
	regPayload1 := randomPayloadDIDNew1()
	buf1 := new(bytes.Buffer)
	regPayload1.Serialize(buf1, types.DIDInfoVersion)
	id1 := []byte(idChainStore.GetIDFromUri(regPayload1.PayloadInfo.ID))

	tx1 := &stype.Transaction{
		Payload: regPayload1,
	}

	regPayload2 := randomPayloadDIDNew2()
	buf2 := new(bytes.Buffer)
	regPayload2.Serialize(buf2, types.DIDInfoVersion)
	id2 := []byte(idChainStore.GetIDFromUri(regPayload2.PayloadInfo.ID))
	tx2 := &stype.Transaction{
		Payload: regPayload2,
	}

	regPayload3 := randomPayloadDIDNew3()
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
	err = idChainStore.persistRegisterDIDTx(batch, id1, tx1, blockHeight1,
		blockTimeStamp1)
	assert.True(t, err == nil)
	batch.Commit()

	// get DID transaction from chain store
	txs, err := idChainStore.GetDIDTxPayload(id1)
	assert.True(t, err == nil)
	assert.True(t, len(txs) == 1)
	assert.True(t, bytes.Equal(txs[0], buf1.Bytes()))

	height, err := idChainStore.GetExpiresHeight(id1)
	targetExpHeight, _ := getExpiresHeight(tx1, blockHeight1, blockTimeStamp1)
	assert.True(t, err == nil)
	assert.Equal(t, targetExpHeight, height)

	regPayload1New := new(types.PayloadDIDInfo)
	r := bytes.NewReader(txs[0])
	regPayload1New.Deserialize(r, types.DIDInfoVersion)
	assert.True(t, didPayloadEqual(regPayload1.PayloadInfo, regPayload1New.PayloadInfo))

	p1, err := idChainStore.GetLastDIDTxPayload(id1)
	assert.True(t, err == nil)
	assert.True(t, bytes.Equal(p1, buf1.Bytes()))

	// persist register DID transaction
	err = idChainStore.persistRegisterDIDTx(batch, id2, tx2, blockHeight2,
		blockTimeStamp2)
	assert.True(t, err == nil)
	batch.Commit()

	// get DID transaction from chain store
	txs2, err := idChainStore.GetDIDTxPayload(id2)
	assert.True(t, err == nil)
	assert.True(t, len(txs2) == 1)
	assert.True(t, bytes.Equal(txs2[0], buf2.Bytes()))
	regPayload2New := new(types.PayloadDIDInfo)
	r2 := bytes.NewReader(txs2[0])
	regPayload2New.Deserialize(r2, types.DIDInfoVersion)
	assert.True(t, didPayloadEqual(regPayload2.PayloadInfo, regPayload2New.PayloadInfo))

	p2, err := idChainStore.GetLastDIDTxPayload(id2)
	assert.True(t, err == nil)
	assert.True(t, bytes.Equal(p2, buf2.Bytes()))

	height2, err := idChainStore.GetExpiresHeight(id2)
	targetExpHeight2, _ := getExpiresHeight(tx2, blockHeight2, blockTimeStamp2)
	assert.True(t, err == nil)
	assert.Equal(t, targetExpHeight2, height2)

	// persist register DID transaction
	err = idChainStore.persistRegisterDIDTx(batch, id2, tx3, blockHeight3,
		blockTimeStamp3)
	assert.True(t, err == nil)
	batch.Commit()

	height3, err := idChainStore.GetExpiresHeight(id2)
	targetExpHeight3, _ := getExpiresHeight(tx2, blockHeight3, blockTimeStamp3)
	assert.True(t, err == nil)
	assert.Equal(t, targetExpHeight3, height3)

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
	height2Roll, err := idChainStore.GetExpiresHeight(id2)
	assert.True(t, err == nil)
	assert.Equal(t, height2, height2Roll)

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
	height1Roll, err := idChainStore.GetExpiresHeight(id1)
	assert.True(t, err == nil)
	assert.Equal(t, height, height1Roll)

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

	_, err = idChainStore.GetExpiresHeight(id1)
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
		Expires: "2020-02-10T17:00:00Z",
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

func randomPayloadDIDNew1() *types.PayloadDIDInfo {
	info := new(types.DIDPayloadInfo)
	json.Unmarshal(didPayloadBytes1, info)

	return &types.PayloadDIDInfo{
		Header: types.DIDHeaderInfo{
			Specification: randomString(),
			Operation:     randomString(),
		},
		Payload: hex.EncodeToString(didPayloadBytes1),
		Proof: types.DIDProofInfo{
			Type:               randomString(),
			VerificationMethod: randomString(),
			Signature:          randomString(),
		},
		PayloadInfo: info,
	}
}
func randomPayloadDIDNew2() *types.PayloadDIDInfo {
	info := new(types.DIDPayloadInfo)
	json.Unmarshal(didPayloadBytes2, info)

	return &types.PayloadDIDInfo{
		Header: types.DIDHeaderInfo{
			Specification: randomString(),
			Operation:     randomString(),
		},
		Payload: hex.EncodeToString(didPayloadBytes2),
		Proof: types.DIDProofInfo{
			Type:               randomString(),
			VerificationMethod: randomString(),
			Signature:          randomString(),
		},
		PayloadInfo: info,
	}
}
func randomPayloadDIDNew3() *types.PayloadDIDInfo {
	info := new(types.DIDPayloadInfo)
	json.Unmarshal(didPayloadBytes3, info)

	return &types.PayloadDIDInfo{
		Header: types.DIDHeaderInfo{
			Specification: randomString(),
			Operation:     randomString(),
		},
		Payload: hex.EncodeToString(didPayloadBytes3),
		Proof: types.DIDProofInfo{
			Type:               randomString(),
			VerificationMethod: randomString(),
			Signature:          randomString(),
		},
		PayloadInfo: info,
	}
}

func didPayloadEqual(first *types.DIDPayloadInfo, second *types.DIDPayloadInfo) bool {
	return first.ID == second.ID &&
		didPublicKeysEqual(first.PublicKey, second.PublicKey) &&
		didAuthEqual(first.Authentication, second.Authentication) &&
		didAuthEqual(first.Authorization, second.Authorization)
}

func didPublicKeysEqual(first []types.DIDPublicKeyInfo, second []types.DIDPublicKeyInfo) bool {
	if len(first) != len(second) {
		return false
	}
	for i := 0; i < len(first); i++ {
		if !didPublicKeyEqual(&first[i], &second[i]) {
			return false
		}
	}
	return true
}

func didPublicKeyEqual(first *types.DIDPublicKeyInfo, second *types.DIDPublicKeyInfo) bool {
	return first.ID == second.ID && first.Type == second.Type &&
		first.Controller == second.Controller &&
		first.PublicKeyBase58 == second.PublicKeyBase58
}

func didAuthEqual(first []interface{}, second []interface{}) bool {
	if len(first) != len(second) {
		return false
	}
	for i := 0; i < len(first); i++ {
		switch first[i].(type) {
		case string:
			if first[i] != second[i] {
				return false
			}
		case map[string]interface{}:
			data1, _ := json.Marshal(first[i])
			pk1 := new(types.DIDPublicKeyInfo)
			json.Unmarshal(data1, pk1)

			data2, _ := json.Marshal(second[i])
			pk2 := new(types.DIDPublicKeyInfo)
			json.Unmarshal(data2, pk2)

			if !didPublicKeyEqual(pk1, pk2) {
				return false
			}
		default:
			return false
		}

	}
	return true
}
