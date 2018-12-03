package store

type DBOperator interface {
	InitConnection(connParams ...interface{}) error
	Connect() error
	Disconnect() error
	Execute(script string, params ...interface{}) (uint64, error)
	Query(script string, params ...interface{}) (uint64, error)
}
