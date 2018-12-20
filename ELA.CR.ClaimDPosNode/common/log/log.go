package log

import (
	"bytes"
	"fmt"
	"io"
	"log"
	"os"
	"path/filepath"
	"runtime"
	"strconv"

	"github.com/elastos/Elastos.ELA.Utility/elalog"
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
	OutputPath            = "./Logs/" // The log files output path
	calldepth             = 2
	KBSize                = int64(1024)
	MBSize                = KBSize * 1024
	GBSize                = MBSize * 1024
	defaultPerLogFileSize = 20 * MBSize
	defaultLogsFolderSize = 5 * GBSize
)

func GetGID() uint64 {
	var buf [64]byte
	b := buf[:runtime.Stack(buf[:], false)]
	b = bytes.TrimPrefix(b, []byte("goroutine "))
	b = b[:bytes.IndexByte(b, ' ')]
	n, _ := strconv.ParseUint(string(b), 10, 64)
	return n
}

var logger *Logger

func levelName(level uint8) string {
	if int(level) >= len(levels) {
		return fmt.Sprintf("LEVEL%d", level)
	}
	return levels[int(level)]
}

type Logger struct {
	level  uint8 // The log print level
	logger *log.Logger
}

func NewLogger(outputPath string, level uint8, maxPerLogSizeMb, maxLogsSizeMb int64) *Logger {
	var perLogFileSize = defaultPerLogFileSize
	var logsFolderSize = defaultLogsFolderSize

	if maxPerLogSizeMb != 0 {
		perLogFileSize = maxPerLogSizeMb * MBSize
	}
	if maxLogsSizeMb != 0 {
		logsFolderSize = maxLogsSizeMb * MBSize
	}

	writer := elalog.NewFileWriter(outputPath, perLogFileSize, logsFolderSize)

	return &Logger{
		level: level,
		logger: log.New(io.MultiWriter(os.Stdout, writer), "",
			log.Ldate|log.Lmicroseconds),
	}
}

func Init(level uint8, maxPerLogSizeMb, maxLogsSizeMb int64) {
	logger = NewLogger(OutputPath, level, maxPerLogSizeMb, maxLogsSizeMb)
}

func (l *Logger) SetPrintLevel(level uint8) {
	l.level = level
}

func (l *Logger) Output(level uint8, a ...interface{}) {
	if l.level <= level {
		gidStr := strconv.FormatUint(GetGID(), 10)
		a = append([]interface{}{levelName(level), "GID", gidStr + ","}, a...)
		l.logger.Output(calldepth, fmt.Sprintln(a...))
	}
}

func (l *Logger) Outputf(level uint8, format string, v ...interface{}) {
	if l.level <= level {
		v = append([]interface{}{levelName(level), "GID", GetGID()}, v...)
		l.logger.Output(calldepth, fmt.Sprintf("%s %s %d, "+format+"\n", v...))
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

	l.Outputf(debugLog, format, a...)
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
	logger.Debugf("%s %s:%d "+format, a...)
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
	logger.SetPrintLevel(level)
}
