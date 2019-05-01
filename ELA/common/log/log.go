package log

import (
	"fmt"
	"io"
	"log"
	"os"
	"path/filepath"
	"runtime"
	"strconv"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/utils/elalog"
)

const (
	Red    = "1;31"
	Green  = "1;32"
	Yellow = "1;33"
	Pink   = "1;35"
	Cyan   = "1;36"
)

func Color(code, msg string) string {
	return fmt.Sprintf("\033[%sm%s\033[m", code, msg)
}

const (
	debugLog uint8 = iota
	infoLog
	warnLog
	errorLog
	fatalLog
	disableLog
)

var (
	levels = []string{
		debugLog:   Color(Green, "[DBG]"),
		infoLog:    Color(Pink, "[INF]"),
		warnLog:    Color(Yellow, "[WRN]"),
		errorLog:   Color(Red, "[ERR]"),
		fatalLog:   Color(Red, "[FAT]"),
		disableLog: "DISABLED",
	}
	Stdout = os.Stdout
)

const (
	calldepth = 2

	defaultPerLogFileSize int64 = 20 * elalog.MBSize
	defaultLogsFolderSize int64 = 5 * elalog.GBSize
)

var logger *Logger

func levelName(level uint8) string {
	if int(level) >= len(levels) {
		return fmt.Sprintf("LEVEL%d", level)
	}
	return levels[int(level)]
}

type Logger struct {
	level  uint8 // The log print level
	writer io.Writer
	logger *log.Logger
}

func NewLogger(outputPath string, level uint8, maxPerLogSizeMb, maxLogsSizeMb int64) *Logger {
	var perLogFileSize = defaultPerLogFileSize
	var logsFolderSize = defaultLogsFolderSize

	if maxPerLogSizeMb != 0 {
		perLogFileSize = maxPerLogSizeMb * elalog.MBSize
	}
	if maxLogsSizeMb != 0 {
		logsFolderSize = maxLogsSizeMb * elalog.MBSize
	}

	fileWriter := elalog.NewFileWriter(outputPath, perLogFileSize, logsFolderSize)
	logWriter := io.MultiWriter(os.Stdout, fileWriter)

	return &Logger{
		level:  level,
		writer: logWriter,
		logger: log.New(logWriter, "", log.Ldate|log.Lmicroseconds),
	}
}

func NewDefault(path string, level uint8, maxPerLogSizeMb, maxLogsSizeMb int64) *Logger {
	logger = NewLogger(path, level, maxPerLogSizeMb, maxLogsSizeMb)
	return logger
}

func (l *Logger) Writer() io.Writer {
	return l.writer
}

func (l *Logger) Output(level uint8, a ...interface{}) {
	if l.level <= level {
		a = append([]interface{}{levelName(level), "GID", common.Goid() + ","}, a...)
		l.logger.Output(calldepth, fmt.Sprintln(a...))
	}
}

func (l *Logger) Outputf(level uint8, format string, v ...interface{}) {
	if l.level <= level {
		v = append([]interface{}{levelName(level), "GID", common.Goid()}, v...)
		l.logger.Output(calldepth, fmt.Sprintf("%s %s %s, "+format+"\n", v...))
	}
}

func (l *Logger) Debug(a ...interface{}) {
	if l.level > debugLog {
		return
	}

	pc, file, line, ok := runtime.Caller(calldepth)
	if !ok {
		return
	}

	fn := runtime.FuncForPC(pc)
	a = append([]interface{}{fn.Name(), filepath.Base(file) + ":" + strconv.Itoa(line)}, a...)

	l.Output(debugLog, a...)
}

func (l *Logger) Debugf(format string, a ...interface{}) {
	if l.level > debugLog {
		return
	}

	pc, file, line, ok := runtime.Caller(calldepth)
	if !ok {
		return
	}

	fn := runtime.FuncForPC(pc)
	a = append([]interface{}{fn.Name(), filepath.Base(file), line}, a...)

	l.Outputf(debugLog, "%s %s:%d "+format, a...)
}

func (l *Logger) Info(a ...interface{}) {
	l.Output(infoLog, a...)
}

func (l *Logger) Infof(format string, a ...interface{}) {
	l.Outputf(infoLog, format, a...)
}

func (l *Logger) Warn(a ...interface{}) {
	l.Output(warnLog, a...)
}

func (l *Logger) Warnf(format string, a ...interface{}) {
	l.Outputf(warnLog, format, a...)
}

func (l *Logger) Error(a ...interface{}) {
	if l.level <= errorLog {
		l.Output(errorLog, a...)
	}
}

func (l *Logger) Errorf(format string, a ...interface{}) {
	l.Outputf(errorLog, format, a...)
}

func (l *Logger) Fatal(a ...interface{}) {
	l.Output(fatalLog, a...)
}

func (l *Logger) Fatalf(format string, a ...interface{}) {
	l.Outputf(fatalLog, format, a...)
}

// Level returns the current logging level.
func (l *Logger) Level() elalog.Level {
	return elalog.Level(l.level)
}

// SetLevel changes the logging level to the passed level.
func (l *Logger) SetLevel(level elalog.Level) {
	l.level = uint8(level)
}

func Debug(a ...interface{}) {
	logger.Debug(a...)
}

func Debugf(format string, a ...interface{}) {
	logger.Debugf(format, a...)
}

func Info(a ...interface{}) {
	logger.Info(a...)
}

func Warn(a ...interface{}) {
	logger.Warn(a...)
}

func Error(a ...interface{}) {
	logger.Error(a...)
}

func Fatal(a ...interface{}) {
	logger.Fatal(a...)
}

func Infof(format string, a ...interface{}) {
	logger.Infof(format, a...)
}

func Warnf(format string, a ...interface{}) {
	logger.Warnf(format, a...)
}

func Errorf(format string, a ...interface{}) {
	logger.Errorf(format, a...)
}

func Fatalf(format string, a ...interface{}) {
	logger.Fatalf(format, a...)
}

func SetPrintLevel(level uint8) {
	logger.SetLevel(elalog.Level(level))
}
