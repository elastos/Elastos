package blockchain

// DataEntryPrefix
type DataEntryPrefix byte

const (
	// DATA
	DATABlockHash   DataEntryPrefix = 0x00
	DATAHeader      DataEntryPrefix = 0x01
	DATATransaction DataEntryPrefix = 0x02
	DATAConfirm     DataEntryPrefix = 0x03

	// DPOS
	DPOSDutyChangedCount DataEntryPrefix = 0x10

	// INDEX
	IXHeaderHashList DataEntryPrefix = 0x80
	IXUnspent        DataEntryPrefix = 0x90
	IXUnspentUTXO    DataEntryPrefix = 0x91
	IXSideChainTx    DataEntryPrefix = 0x92

	// VOTE
	VOTERegisterProducer DataEntryPrefix = 0xc1

	// ASSET
	STInfo DataEntryPrefix = 0xc0

	//SYSTEM
	SYSCurrentBlock      DataEntryPrefix = 0x40
	SYSCurrentBookKeeper DataEntryPrefix = 0x42

	//CONFIG
	CFGVersion DataEntryPrefix = 0xf0
)
