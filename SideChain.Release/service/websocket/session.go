package websocket

import (
	"errors"
	"math/rand"
	"sync"
	"time"

	"github.com/gorilla/websocket"
)

const timeout int64 = 120 // 120 seconds

type Session struct {
	mtx        sync.Mutex
	id         string
	conn       *websocket.Conn
	lastActive int64
}

type SessionList struct {
	sync.RWMutex
	list map[string]*Session //key is session id
}

func (s *Session) Send(data []byte) error {
	s.mtx.Lock()
	defer s.mtx.Unlock()

	if s.conn == nil {
		return errors.New("WebSocket is null")
	}
	return s.conn.WriteMessage(websocket.TextMessage, data)
}

func (s *Session) IsTimeout() bool {
	return time.Now().Unix()-s.lastActive > timeout
}

func (sl *SessionList) NewSession(conn *websocket.Conn) *Session {
	sl.Lock()
	s := &Session{
		conn:       conn,
		lastActive: time.Now().Unix(),
		id:         randomId(),
	}
	sl.list[s.id] = s
	sl.Unlock()
	return s
}

func (sl *SessionList) CloseSession(session *Session) {
	sl.Lock()
	delete(sl.list, session.id)
	session.conn.Close()
	sl.Unlock()
}

func (sl *SessionList) ForEach(closure func(*Session)) {
	sl.RLock()
	for _, v := range sl.list {
		closure(v)
	}
	sl.RUnlock()
}

func newSessionList() *SessionList {
	return &SessionList{
		list: make(map[string]*Session),
	}
}

const chars = "abcdefghijklmnopqrstuvwxyz0123456789"

func randomId() string {
	rand.NewSource(time.Now().UnixNano())
	id := make([]byte, 32)
	for i := range id {
		id[i] = chars[rand.Intn(len(chars))]
	}
	return string(id)
}
