package store

type DBOperator interface {
	InitConnection(connParams ...interface{})
	Connect() error
	Disconnect() error
	Execute(script string, params ...interface{}) (uint64, error)
	Query(script string, params ...interface{}) (uint64, error)
}
