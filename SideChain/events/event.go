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
func (e *Event) Subscribe(eventtype EventType, eventfunc EventFunc) Subscriber {
	e.m.Lock()
	defer e.m.Unlock()

	sub := make(chan interface{})
	_, ok := e.subscribers[eventtype]
	if !ok {
		e.subscribers[eventtype] = make(map[Subscriber]EventFunc)
	}
	e.subscribers[eventtype][sub] = eventfunc

	return sub
}

//Notify subscribers that Subscribe specified event
func (e *Event) Notify(eventtype EventType, value interface{}) (err error) {
	e.m.RLock()
	defer e.m.RUnlock()

	subs, ok := e.subscribers[eventtype]
	if !ok {
		err = errors.New("No event type.")
		return
	}

	for _, event := range subs {
		go func(eventfunc EventFunc, value interface{}) { eventfunc(value) }(event, value)
	}
	return
}
