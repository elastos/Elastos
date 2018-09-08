package database

// DB is the common interface to all database implementations.
type DB interface {
	// Clear delete all data in database.
	Clear() error

	// Close database.
	Close() error
}
