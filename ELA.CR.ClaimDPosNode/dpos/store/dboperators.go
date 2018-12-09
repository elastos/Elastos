package store

type DBOperator interface {
	InitConnection(connParams ...interface{}) error
	Connect() error
	Disconnect() error

	Create(table *DposTable) error
	Insert(table *DposTable, fields []*Field) (uint64, error)
	Select(table *DposTable, inputFields []*Field) ([][]*Field, error)
	Update(table *DposTable, inputFields []*Field, updateFields []*Field) ([]uint64, error)

	SelectID(table *DposTable, inputFields []*Field) ([]uint64, error)
}
