package types

import (
	"bytes"
	"crypto/rand"
	"encoding/hex"
	"encoding/json"
	"testing"

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
		"}]" +
		"}",
)

func TestDIDPayloadInfo(t *testing.T) {
	// test for unmarshal did payload from bytes
	info := new(DIDPayloadInfo)
	err := json.Unmarshal(didPayloadBytes, info)
	assert.True(t, err == nil)
}

func TestPayloadDID_Serialize(t *testing.T) {
	// test for payloadDIDInfo serialize and deserialize
	payload1 := randomPayloadDID()

	buf := new(bytes.Buffer)
	payload1.Serialize(buf, DIDInfoVersion)

	payload2 := &PayloadDIDInfo{}
	payload2.Deserialize(buf, DIDInfoVersion)

	assert.True(t, paylaodDIDInfoEqual(payload1, payload2))
}

func paylaodDIDInfoEqual(first *PayloadDIDInfo, second *PayloadDIDInfo) bool {
	return didHeaderEqual(&first.Header, &second.Header) &&
		first.Payload == second.Payload &&
		didProofEqual(&first.Proof, &second.Proof) &&
		didPayloadEqual(first.PayloadInfo, second.PayloadInfo)
}

func didHeaderEqual(first *DIDHeaderInfo, second *DIDHeaderInfo) bool {
	return first.Specification == second.Specification &&
		first.Operation == second.Operation
}

func didProofEqual(first *DIDProofInfo, second *DIDProofInfo) bool {
	return first.Type == second.Type &&
		first.VerificationMethod == second.VerificationMethod &&
		first.Signature == second.Signature
}

func didPayloadEqual(first *DIDPayloadInfo, second *DIDPayloadInfo) bool {
	return first.ID == second.ID &&
		didPublicKeysEqual(first.PublicKey, second.PublicKey) &&
		didAuthEqual(first.Authentication, second.Authentication) &&
		didAuthEqual(first.Authorization, second.Authorization)
}

func didPublicKeysEqual(first []DIDPublicKeyInfo, second []DIDPublicKeyInfo) bool {
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

func didPublicKeyEqual(first *DIDPublicKeyInfo, second *DIDPublicKeyInfo) bool {
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
			pk1 := new(DIDPublicKeyInfo)
			json.Unmarshal(data1, pk1)

			data2, _ := json.Marshal(second[i])
			pk2 := new(DIDPublicKeyInfo)
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

func randomPayloadDID() *PayloadDIDInfo {
	info := new(DIDPayloadInfo)
	json.Unmarshal(didPayloadBytes, info)

	return &PayloadDIDInfo{
		Header: DIDHeaderInfo{
			Specification: randomString(),
			Operation:     randomString(),
		},
		Payload: hex.EncodeToString(didPayloadBytes),
		Proof: DIDProofInfo{
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
