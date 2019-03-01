package elalog

// Logger is an interface which describes a level-based logger.  A default
// implementation of Logger is implemented by this package and can be created
// by calling (*Backend).Logger.
type Logger interface {
	// Debugf formats message according to format specifier and writes to
	// log with LevelDebug.
	Debugf(format string, params ...interface{})

	// Infof formats message according to format specifier and writes to
	// log with LevelInfo.
	Infof(format string, params ...interface{})

	// Warnf formats message according to format specifier and writes to
	// to log with LevelWarn.
	Warnf(format string, params ...interface{})

	// Errorf formats message according to format specifier and writes to
	// to log with LevelError.
	Errorf(format string, params ...interface{})

	// Fatalf formats message according to format specifier and writes to
	// log with LevelFatal.
	Fatalf(format string, params ...interface{})

	// Debug formats message using the default formats for its operands
	// and writes to log with LevelDebug.
	Debug(v ...interface{})

	// Info formats message using the default formats for its operands
	// and writes to log with LevelInfo.
	Info(v ...interface{})

	// Warn formats message using the default formats for its operands
	// and writes to log with LevelWarn.
	Warn(v ...interface{})

	// Error formats message using the default formats for its operands
	// and writes to log with LevelError.
	Error(v ...interface{})

	// Fatal formats message using the default formats for its operands
	// and writes to log with LevelFatal.
	Fatal(v ...interface{})

	// Level returns the current logging level.
	Level() Level

	// SetLevel changes the logging level to the passed level.
	SetLevel(level Level)
}
