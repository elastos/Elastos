package store

// DataEntryPrefix
type DataEntryPrefix byte

const (
	// DPOS
	DPOSDutyChangedCount   DataEntryPrefix = 0x11
	DPOSCurrentArbitrators DataEntryPrefix = 0x12
	DPOSCurrentCandidates  DataEntryPrefix = 0x13
	DPOSNextArbitrators    DataEntryPrefix = 0x14
	DPOSNextCandidates     DataEntryPrefix = 0x15
	DPOSDirectPeers        DataEntryPrefix = 0x16
)
