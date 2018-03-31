package sdk

const (
	MainNetMagic = 7630401
	TestNetMagic = 1234567

	ProtocolVersion = 1 // The min protocol version to support spv
	ServiveSPV    = 1 << 2
	SPVServerPort = 20866
	SPVClientPort = 20867
)
