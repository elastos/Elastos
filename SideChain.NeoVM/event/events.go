package event

import (
	"github.com/elastos/Elastos.ELA.SideChain/events"
)

// Constants for the type of a notification message.
const (
	ETRunTimeNotify events.EventType = 0x50
	ETRunTimeLog    events.EventType = 051
)

var notificationStrings = map[events.EventType]string{
	ETRunTimeNotify: "ETRunTimeNotify",
	ETRunTimeLog:    "ETRunTimeLog",
}