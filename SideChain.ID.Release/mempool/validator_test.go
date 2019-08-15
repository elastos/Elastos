package mempool

import (
	"crypto/rand"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"math/big"
	"testing"

	"github.com/btcsuite/btcutil/base58"
	"github.com/elastos/Elastos.ELA.SideChain.ID/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain.ID/params"
	"github.com/elastos/Elastos.ELA.SideChain.ID/types"
	types2 "github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/stretchr/testify/assert"
)

var validator_test validator

func init() {

	idChainStore, err := blockchain.NewChainStore(params.GenesisBlock,
		"Chain_UnitTest")
	if err != nil {
		return
	}
	validator_test.Store = idChainStore
}

//03bfd8bd2b10e887ec785360f9b329c2ae567975c784daca2f223cb19840b51914
const (
	PayloadPrivateKey = "5fe87de21fa55d751583bd0d74532c3cc679caf67919261e0c9b2a56f547c38d"
	//my 5fe87de21fa55d751583bd0d74532c3cc679caf67919261e0c9b2a56f547c38d
	//other 7638c2a799d93185279a4a6ae84a5b76bd89e41fa9f465d9ae9b2120533983a1
	TxPrivateKey  = "5fe87de21fa55d751583bd0d74532c3cc679caf67919261e0c9b2a56f547c38d"
	publicKeyStr1 = "031e12374bae471aa09ad479f66c2306f4bcc4ca5b754609a82a1839b94b4721b9"
	publicKeyStr2 = "03bfd8bd2b10e887ec785360f9b329c2ae567975c784daca2f223cb19840b51914"
	publicKeyStr3 = "035d3adebb69db5fbd8005c37d225cd2fd9ec50ec7fcb38ff7c4fcf9b90455cf5f"
)

var didPayloadBytes = []byte(
	"{" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN\"," +
		"\"publicKey\": [{" +
		"\"id\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default\"," +
		"\"type\": \"ECDSAsecp256r1\"," +
		"\"controller\": \"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN\"," +
		"\"publicKeyBase58\": \"27bqfhMew6TjL4NMz2u8b2cFCvGovaELqr19Xytt1rDmd\"" +
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
		"\"expires\": \"2014-02-10T17:00:00Z\"" +
		"}",
)

func TestIDChainStore_CreateDIDTx(t *testing.T) {
	tx := &types2.Transaction{
		TxType:         0x0a,
		PayloadVersion: 0,
		Payload:        getPayloadCreateDID(),
		Inputs:         nil,
		Outputs:        nil,
		LockTime:       0,
		Programs:       nil,
		Fee:            0,
		FeePerKB:       0,
	}
	fmt.Println(tx)

	data, _ := hex.DecodeString(publicKeyStr1)
	fmt.Println(len(data))
	i, _ := getDIDByPublicKey(data)
	didAddress, _ := i.ToAddress()
	fmt.Println("didAddress", didAddress)

	bi := new(big.Int).SetBytes(data).String()
	base58PK := base58.Encode([]byte(bi))

	fmt.Println("base58PK", string(base58PK))
	fmt.Println("len(base58PK）", len(base58PK))

	err2 := validator_test.checkRegisterDID(tx)
	assert.NoError(t, err2)

}

func TestCheckDIDOperation(t *testing.T) {

	//no create ------>update
	payloadUpdateDIDInfo := getPayloadUpdateDID()
	err := validator_test.checkDIDOperation(&payloadUpdateDIDInfo.Header,
		payloadUpdateDIDInfo.PayloadInfo.ID)
	assert.Equal(t, err.Error(), "DID WRONG OPERATION NOT EXIST")

	//doubale create
	payloadCreate := getPayloadCreateDID()
	err = validator_test.checkDIDOperation(&payloadCreate.Header,
		payloadCreate.PayloadInfo.ID)
	assert.NoError(t, err)
	//todo process tx
	//err = validator_test.checkDIDOperation(&payloadCreate.Header,
	//	payloadCreate.PayloadInfo.ID)
	//assert.Equal(t, err.Error(), "DID WRONG OPERATION ALREADY EXIST")

}

func TestGetIDFromUri(t *testing.T) {
	validUriFormat := "did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN"
	id := validator_test.Store.GetIDFromUri(validUriFormat)
	assert.Equal(t, id, "icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN")

	InvalidUriFormat := "icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN"
	id = validator_test.Store.GetIDFromUri(InvalidUriFormat)
	assert.Equal(t, id, "")
}

func getDIDByPublicKey(publicKey []byte) (*common.Uint168, error) {
	pk, _ := crypto.DecodePoint(publicKey)
	redeemScript, err := contract.CreateStandardRedeemScript(pk)
	if err != nil {
		return nil, err
	}
	return getDIDByCode(redeemScript)
}

func getDIDByCode(code []byte) (*common.Uint168, error) {
	ct1, error := contract.CreateCRDIDContractByCode(code)
	if error != nil {
		return nil, error
	}
	return ct1.ToProgramHash(), error
}

func getPayloadCreateDID() *types.PayloadDIDInfo {
	info := new(types.DIDPayloadInfo)
	json.Unmarshal(didPayloadBytes, info)

	p := &types.PayloadDIDInfo{
		Header: types.DIDHeaderInfo{
			Specification: "elastos/did/1.0",
			Operation:     "create",
		},
		Payload: hex.EncodeToString(didPayloadBytes),
		Proof: types.DIDProofInfo{
			Type:               "ECDSAsecp256r1",
			VerificationMethod: "did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default",
		},
		PayloadInfo: info,
	}

	privateKey1, _ := common.HexStringToBytes(PayloadPrivateKey)

	sign, _ := crypto.Sign(privateKey1, p.Data(types.DIDInfoVersion))
	p.Proof.Signature = hex.EncodeToString(sign)

	return p
}

func getPayloadUpdateDID() *types.PayloadDIDInfo {
	info := new(types.DIDPayloadInfo)
	json.Unmarshal(didPayloadBytes, info)

	return &types.PayloadDIDInfo{
		Header: types.DIDHeaderInfo{
			Specification: "elastos/did/1.0",
			Operation:     "update",
		},
		Payload: hex.EncodeToString(didPayloadBytes),
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
