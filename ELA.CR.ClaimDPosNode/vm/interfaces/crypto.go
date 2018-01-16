package interfaces

type ICrypto interface {
	Hash160(dataContent []byte) ([]byte)

	Hash256(dateContent []byte) ([]byte)

	VerifySignature(dataContent []byte, signature []byte, pubkey []byte) (bool, error)
}
