package store

// DataEntryPrefix
type DataEntryPrefix byte

const (
	// DPOS
	DPOSCheckPointHeights  DataEntryPrefix = 0x10
	DPOSSingleCheckPoint   DataEntryPrefix = 0x11
)
