package store

type DBOperator interface {
	InitConnection(connParams ...interface{}) error
	Connect() error
	Disconnect() error

	Create(table *DBTable) error
	Insert(table *DBTable, fields []*Field) (uint64, error)
	Select(table *DBTable, inputFields []*Field) ([][]*Field, error)
	Update(table *DBTable, inputFields []*Field, updateFields []*Field) ([]uint64, error)

	SelectID(table *DBTable, inputFields []*Field) ([]uint64, error)
}
