package p2p

func Init(walletId uint64) {
	// Initiate PeerManager
	instance = new(PeerManager)
	instance.Peers = newPeers(walletId)
	instance.SyncManager = new(SyncManager)
	instance.addrManager = newAddrManager()
	instance.connManager = newConnManager(instance.OnDiscardAddr)

	// Init listeners
	listeners = &Listeners{
		OnVersion:     instance.OnVersion,
		OnVerAck:      instance.OnVerAck,
		OnPing:        instance.OnPing,
		OnPong:        instance.OnPong,
		OnAddrs:       instance.OnAddrs,
		OnAddrsReq:    instance.OnAddrsReq,
		OnInventory:   instance.OnInventory,
		OnMerkleBlock: instance.OnMerkleBlock,
		OnTxn:         instance.OnTxn,
		OnNotFound:    instance.OnNotFound,
	}
}

func Start() {
	instance.Start()
}
