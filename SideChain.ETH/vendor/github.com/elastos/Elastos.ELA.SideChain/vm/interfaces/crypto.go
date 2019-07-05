package interfaces

type ICrypto interface {
	Hash168(data []byte) []byte

	Hash256(data []byte) []byte

	VerifySignature(data []byte, signature []byte, pubkey []byte) error
}
