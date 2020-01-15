package blockchain

import (
	"bytes"
	"crypto/rand"
	"encoding/hex"
	"encoding/json"
	mathrand "math/rand"
	"os"
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA.SideChain.ID/params"
	"github.com/elastos/Elastos.ELA.SideChain.ID/types"
	"github.com/elastos/Elastos.ELA.SideChain.ID/types/base64url"
	stype "github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/stretchr/testify/assert"
)

const (
	ID1 = "did:elastos:iXZck7tte4V1F7THQ9Z4z7xuah6j4U33zo"
	ID2 = "did:elastos:imfFhB5zNwTAMHh2RMpgRrZeSo7C5WtkzS"
	ID3 = "did:elastos:ifawgWFmZRLXN1JVmqpXcNRurhB1zyHNcf"
)

var didPayloadBytes = []byte(
	`{
        "id" : "did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN",
        "publicKey":[{ "id": "did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default",
                       "type":"ECDSAsecp256r1",
                       "controller":"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN",
                       "publicKeyBase58":"27bqfhMew6TjL4NMz2u8b2cFCvGovaELqr19Xytt1rDmd"
                      }
                    ],
        "authentication":["did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default",
                          {
                               "id": "did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default",
                               "type":"ECDSAsecp256r1",
                               "controller":"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN",
                               "publicKeyBase58":"zNxoZaZLdackZQNMas7sCkPRHZsJ3BtdjEvM2y5gNvKJ"
                           }
                         ],
        "authorization":["did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default"],
        "expires" : "2023-02-10T17:00:00Z"
	}`)

func isOperationEqual(operation1, operation2 types.Operation) bool {
	buf1 := new(bytes.Buffer)
	operation1.Serialize(buf1, types.DIDInfoVersion)
	buf2 := new(bytes.Buffer)
	operation2.Serialize(buf2, types.DIDInfoVersion)
	return bytes.Equal(buf2.Bytes(), buf1.Bytes())
}

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
	regPayload1 := getRandomPayloadDid(ID1)
	var txData1, txData2, txData3 types.TranasactionData
	txData1.Operation = *regPayload1
	buf1 := new(bytes.Buffer)
	regPayload1.Serialize(buf1, types.DIDInfoVersion)

	id1 := []byte(idChainStore.GetDIDFromUri(regPayload1.PayloadInfo.ID))
	tx1 := &stype.Transaction{
		Payload: regPayload1,
	}

	regPayload2 := getRandomPayloadDid(ID2)
	txData2.Operation = *regPayload2

	buf2 := new(bytes.Buffer)
	regPayload2.Serialize(buf2, types.DIDInfoVersion)
	id2 := []byte(idChainStore.GetDIDFromUri(regPayload2.PayloadInfo.ID))
	tx2 := &stype.Transaction{
		Payload: regPayload2,
	}

	regPayload3 := getRandomPayloadDid(ID3)
	txData3.Operation = *regPayload3

	buf3 := new(bytes.Buffer)
	regPayload3.Serialize(buf3, types.DIDInfoVersion)
	regPayload3.PayloadInfo.ID = regPayload2.PayloadInfo.ID
	tx3 := &stype.Transaction{
		Payload: regPayload3,
	}

	//check chain store not exist did tx
	_, err = idChainStore.GetAllDIDTxTxData(id1)
	assert.True(t, err != nil)

	// persist register DID transaction
	batch := idChainStore.NewBatch()
	err = idChainStore.persistRegisterDIDTx(batch, id1, tx1, blockHeight1,
		blockTimeStamp1)
	assert.True(t, err == nil)
	batch.Commit()

	// get DID transaction from chain store
	txs, err := idChainStore.GetAllDIDTxTxData(id1)
	assert.True(t, err == nil)
	assert.True(t, len(txs) == 1)
	assert.True(t, isOperationEqual(txs[0].Operation, txData1.Operation))

	height, err := idChainStore.GetExpiresHeight(id1)
	targetExpHeight, _ := idChainStore.TryGetExpiresHeight(tx1, blockHeight1, blockTimeStamp1)
	assert.True(t, err == nil)
	assert.Equal(t, targetExpHeight, height)

	p1, err := idChainStore.GetLastDIDTxData(id1)
	assert.True(t, err == nil)
	assert.True(t, isOperationEqual(p1.Operation, txData1.Operation))

	// persist register DID transaction
	err = idChainStore.persistRegisterDIDTx(batch, id2, tx2, blockHeight2,
		blockTimeStamp2)
	assert.True(t, err == nil)
	batch.Commit()

	p2, err := idChainStore.GetLastDIDTxData(id2)
	assert.True(t, err == nil)
	assert.True(t, isOperationEqual(p2.Operation, txData2.Operation))

	height2, err := idChainStore.GetExpiresHeight(id2)
	targetExpHeight2, _ := idChainStore.TryGetExpiresHeight(tx2, blockHeight2, blockTimeStamp2)
	assert.True(t, err == nil)
	assert.Equal(t, targetExpHeight2, height2)

	// persist register DID transaction
	err = idChainStore.persistRegisterDIDTx(batch, id2, tx3, blockHeight3,
		blockTimeStamp3)
	assert.True(t, err == nil)
	batch.Commit()

	height3, err := idChainStore.GetExpiresHeight(id2)
	targetExpHeight3, _ := idChainStore.TryGetExpiresHeight(tx2, blockHeight3, blockTimeStamp3)
	assert.True(t, err == nil)
	assert.Equal(t, targetExpHeight3, height3)

	// get DID transaction from chain store
	txs3, err := idChainStore.GetAllDIDTxTxData(id2)
	assert.True(t, err == nil)
	assert.True(t, len(txs3) == 2)
	assert.True(t, isOperationEqual(txs3[0].Operation, txData3.Operation))
	assert.True(t, isOperationEqual(txs3[1].Operation, txData2.Operation))

	p3, err := idChainStore.GetLastDIDTxData(id2)
	assert.True(t, err == nil)
	assert.True(t, isOperationEqual(p3.Operation, txData3.Operation))

	// rollback tx2 will return error, tx2 is not the last one
	err = idChainStore.rollbackRegisterDIDTx(batch, id2, tx2)
	assert.True(t, err != nil)
	batch.Commit()

	p4, err := idChainStore.GetLastDIDTxData(id2)
	assert.True(t, err == nil)
	assert.True(t, isOperationEqual(p4.Operation, txData3.Operation))

	// rollback tx3
	err = idChainStore.rollbackRegisterDIDTx(batch, id2, tx3)
	assert.True(t, err == nil)
	batch.Commit()
	height2Roll, err := idChainStore.GetExpiresHeight(id2)
	assert.True(t, err == nil)
	assert.Equal(t, height2, height2Roll)

	txs4, err := idChainStore.GetAllDIDTxTxData(id2)
	assert.True(t, err == nil)
	assert.True(t, len(txs4) == 1)
	assert.True(t, assert.True(t, isOperationEqual(txs4[0].Operation, txData2.Operation)))

	p5, err := idChainStore.GetLastDIDTxData(id2)
	assert.True(t, err == nil)
	assert.True(t, isOperationEqual(p5.Operation, txData2.Operation))

	// rollback tx2
	err = idChainStore.rollbackRegisterDIDTx(batch, id2, tx2)
	assert.True(t, err == nil)
	batch.Commit()
	height1Roll, err := idChainStore.GetExpiresHeight(id1)
	assert.True(t, err == nil)
	assert.Equal(t, height, height1Roll)

	_, err = idChainStore.GetAllDIDTxTxData(id2)
	assert.True(t, err != nil)
	_, err = idChainStore.GetLastDIDTxData(id2)
	assert.True(t, err != nil)

	// rollback tx1
	err = idChainStore.rollbackRegisterDIDTx(batch, id1, tx1)
	assert.True(t, err == nil)
	batch.Commit()
	_, err = idChainStore.GetAllDIDTxTxData(id1)
	assert.True(t, err != nil)
	_, err = idChainStore.GetLastDIDTxData(id1)
	assert.True(t, err != nil)

	_, err = idChainStore.GetExpiresHeight(id1)
	assert.True(t, err != nil)

}

func randomString() string {
	a := make([]byte, 20)
	rand.Read(a)
	return common.BytesToHexString(a)
}

func getRandomPayloadDid(id string) *types.Operation {
	info := new(types.DIDPayloadInfo)
	json.Unmarshal(didPayloadBytes, info)
	info.ID = id
	info.Expires = getFourYearAfterUTCString()
	data, _ := json.Marshal(info)

	return &types.Operation{
		Header: types.DIDHeaderInfo{
			Specification: "elastos/did/1.0",
			Operation:     getRandomOperation(),
		},
		Payload: base64url.EncodeToString(data),
		Proof: types.DIDProofInfo{
			Type:               randomString(),
			VerificationMethod: randomString(),
			Signature:          randomString(),
		},
		PayloadInfo: info,
	}
}

func getFourYearAfterUTCString() string {
	timeUtc := time.Now().UTC()
	fourYearLater := timeUtc.AddDate(4, 0, 0)
	return fourYearLater.Format(time.RFC3339)
}

func getRandomOperation() string {
	operations := []string{"create", "update"}
	index := mathrand.Int() % 2
	return operations[index]
}

func getDIDPayloadBytes(id string) []byte {
	return []byte(
		"{" +
			"\"id\": \"did:elastos:" + id + "\"," +
			"\"publicKey\": [{" +
			"\"id\": \"did:elastos:" + id + "\"," +
			"\"type\": \"ECDSAsecp256r1\"," +
			"\"controller\": \"did:elastos:" + id + "\"," +
			"\"publicKeyBase58\": \"zxt6NyoorFUFMXA8mDBULjnuH3v6iNdZm42PyG4c1YdC\"" +
			"}]," +
			"\"authentication\": [" +
			"\"did:elastos:" + id + "\"" +
			"]," +
			"\"authorization\": [" +
			"\"did:elastos:" + id + "\"" +
			"]," +
			"\"expires\": \"2020-08-15T17:00:00Z\"" +
			"}",
	)
}

func getPayloadDIDInfo(id string, didOperation string) *types.Operation {
	pBytes := getDIDPayloadBytes(id)
	info := new(types.DIDPayloadInfo)
	json.Unmarshal(pBytes, info)
	p := &types.Operation{
		Header: types.DIDHeaderInfo{
			Specification: "elastos/did/1.0",
			Operation:     didOperation,
		},
		Payload: hex.EncodeToString(pBytes),
		Proof: types.DIDProofInfo{
			Type:               "ECDSAsecp256r1",
			VerificationMethod: "did:elastos:" + id,
		},
		PayloadInfo: info,
	}
	return p
}

//didOperation must be create or update
func getDIDTx(id, didOperation string) *stype.Transaction {
	payloadDidInfo := getPayloadDIDInfo(id, didOperation)
	txn := new(stype.Transaction)
	txn.TxType = types.RegisterDID
	txn.Payload = payloadDidInfo
	return txn
}

func didPayloadEqual(first *types.DIDPayloadInfo, second *types.DIDPayloadInfo) bool {
	return first.ID == second.ID &&
		didPublicKeysEqual(first.PublicKey, second.PublicKey) &&
		didAuthEqual(first.Authentication, second.Authentication) &&
		didAuthEqual(first.Authorization, second.Authorization) &&
		first.Expires == second.Expires
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
