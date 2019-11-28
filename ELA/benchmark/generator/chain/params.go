package chain

type GenerationMod byte

const (
	Fast    GenerationMod = 0x00
	Normal  GenerationMod = 0x01
	Minimal GenerationMod = 0x02
)

type GenerationParams struct {
	Mod GenerationMod

	PrepareStartHeight uint32
	RandomStartHeight  uint32

	InputsPerBlock uint32
	MaxRefersCount uint32
	MinRefersCount uint32
	AddressCount   uint32
}
