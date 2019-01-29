package p2p

// NAFilter defines a NetAddress filter interface, it is used to filter inbound
// peer addresses and cached net addresses when responding to a getaddr message.
type NAFilter interface {
	// Filter takes a NetAddress and return if this address is ok to use.
	Filter(na *NetAddress) bool
}
