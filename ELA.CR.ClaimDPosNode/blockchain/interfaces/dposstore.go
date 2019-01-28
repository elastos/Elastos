package interfaces

type IDBBasic interface {
	Close() error
}

type IDBOperator interface {
	IDBBasic

	Create(table *DBTable) error
	Insert(table *DBTable, fields []*Field) (uint64, error)
	Select(table *DBTable, inputFields []*Field) ([][]*Field, error)
	Update(table *DBTable, inputFields []*Field, updateFields []*Field) ([]uint64, error)

	SelectID(table *DBTable, inputFields []*Field) ([]uint64, error)
}

type DirectPeers struct {
	PublicKey []byte
	Address   string
	Sequence  uint32
}

type IEventRecord interface {
	StartEventRecord()
	AddProposalEvent(event interface{}) error
	UpdateProposalEvent(event interface{}) error
	AddVoteEvent(event interface{}) error
	AddViewEvent(event interface{}) error
	AddConsensusEvent(event interface{}) error
	UpdateConsensusEvent(event interface{}) error
}

type IArbitratorsRecord interface {
	StartArbitratorsRecord()
	GetArbitrators(a Arbitrators) error
	SaveDposDutyChangedCount(count uint32)
	SaveEmergencyData(started bool, startHeight uint32)
	SaveCurrentArbitrators(a Arbitrators)
	SaveNextArbitrators(a Arbitrators)

	GetDirectPeers() ([]*DirectPeers, error)
	SaveDirectPeers(peers []*DirectPeers)
}

// IDposStore provides func for dpos
type IDposStore interface {
	IDBOperator
	IEventRecord
	IArbitratorsRecord
}
