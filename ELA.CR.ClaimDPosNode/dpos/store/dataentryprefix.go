package store

// DataEntryPrefix
type DataEntryPrefix byte

const (
	// DPOS
	DPOSCheckPoints        DataEntryPrefix = 0x10
	DPOSDutyIndex          DataEntryPrefix = 0x11
	DPOSCurrentArbitrators DataEntryPrefix = 0x12
	DPOSCurrentCandidates  DataEntryPrefix = 0x13
	DPOSNextArbitrators    DataEntryPrefix = 0x14
	DPOSNextCandidates     DataEntryPrefix = 0x15
	DPOSCurrentReward      DataEntryPrefix = 0x16
	DPOSNextReward         DataEntryPrefix = 0x17
	DPOSState              DataEntryPrefix = 0x18
)
