package txs

import (
	"testing"

	"github.com/stretchr/testify/suite"
)

type txVersionV0TestSuite struct {
	suite.Suite

	Version TxVersion
}

func (s *txVersionV0TestSuite) SetupTest() {
	s.Version = &txV0{}
}

func TestTxVersionV0Suit(t *testing.T) {
	suite.Run(t, new(txVersionV0TestSuite))
}
