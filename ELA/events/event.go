package events

import (
	"errors"
	"sync"
)

type EventFunc func(v interface{})

type Subscriber chan interface{}

type EventType int16

const (
	EventSaveBlock               EventType = 0
	EventReplyTx                 EventType = 1
	EventBlockPersistCompleted   EventType = 2
	EventNewInventory            EventType = 3
	EventNodeDisconnect          EventType = 4
	EventRollbackTransaction     EventType = 5
	EventNewTransactionPutInPool EventType = 6
)

type Event struct {
	m           sync.RWMutex
	subscribers map[EventType]map[Subscriber]EventFunc
}

func NewEvent() *Event {
	return &Event{
		subscribers: make(map[EventType]map[Subscriber]EventFunc),
	}
}

//  adds a new subscriber to Event.
func (e *Event) Subscribe(eventType EventType, eventFunc EventFunc) Subscriber {
	e.m.Lock()
	defer e.m.Unlock()

	sub := make(chan interface{})
	_, ok := e.subscribers[eventType]
	if !ok {
		e.subscribers[eventType] = make(map[Subscriber]EventFunc)
	}
	e.subscribers[eventType][sub] = eventFunc

	return sub
}

//Notify subscribers that Subscribe specified event
func (e *Event) Notify(eventType EventType, value interface{}) (err error) {
	e.m.RLock()
	defer e.m.RUnlock()

	subs, ok := e.subscribers[eventType]
	if !ok {
		err = errors.New("No event type.")
		return
	}

	for _, event := range subs {
		go func(eventFunc EventFunc, value interface{}) { eventFunc(value) }(event, value)
	}
	return
}
