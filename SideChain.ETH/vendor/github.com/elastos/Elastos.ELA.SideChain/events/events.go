package events

import (
	"fmt"
	"sync"
)

// EventType represents the type of a event message.
type EventType int

// EventCallback is used for a caller to provide a callback for
// notifications about various chain events.
type EventCallback func(*Event)

// Constants for the type of a notification message.
const (
	// ETBlockAccepted indicates the associated block was accepted into
	// the block chain.  Note that this does not necessarily mean it was
	// added to the main chain.  For that, use ETBlockConnected.
	ETBlockAccepted EventType = iota

	// ETBlockConnected indicates the associated block was connected to the
	// main chain.
	ETBlockConnected

	// ETBlockDisconnected indicates the associated block was disconnected
	// from the main chain.
	ETBlockDisconnected

	// ETTransactionAccepted indicates the associated transaction was accepted
	// into transaction mem pool.
	ETTransactionAccepted
)

// notificationTypeStrings is a map of notification types back to their constant
// names for pretty printing.
var notificationTypeStrings = map[EventType]string{
	ETBlockAccepted:       "ETBlockAccepted",
	ETBlockConnected:      "ETBlockConnected",
	ETBlockDisconnected:   "ETBlockDisconnected",
	ETTransactionAccepted: "ETTransactionAccepted",
}

// String returns the EventType in human-readable form.
func (n EventType) String() string {
	if s, ok := notificationTypeStrings[n]; ok {
		return s
	}
	return fmt.Sprintf("Unknown Event Type (%d)", int(n))
}

// Event defines notification that is sent to the caller via the callback
// function provided during the call to New and consists of a notification type
// as well as associated data that depends on the type as follows:
// 	- ETBlockAccepted:     *types.Block
// 	- ETBlockConnected:    *types.Block
// 	- ETBlockDisconnected: *types.Block
// 	- ETTransactionAccepted: *types.Transaction
type Event struct {
	Type EventType
	Data interface{}
}

var events struct {
	mtx       sync.Mutex
	callbacks []EventCallback
}

// Subscribe to block chain notifications. Registers a callback to be executed
// when various events take place. See the documentation on Event and
// EventType for details on the types and contents of notifications.
func Subscribe(callback EventCallback) {
	events.mtx.Lock()
	events.callbacks = append(events.callbacks, callback)
	events.mtx.Unlock()
}

// Notify sends a notification with the passed type and data if the
// caller requested notifications by providing a callback function in the call
// to New.
func Notify(typ EventType, data interface{}) {
	// Generate and send the notification.
	n := Event{Type: typ, Data: data}
	events.mtx.Lock()
	for _, callback := range events.callbacks {
		callback(&n)
	}
	events.mtx.Unlock()
}
