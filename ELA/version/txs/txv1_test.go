package txs

import (
	"testing"

	"github.com/stretchr/testify/suite"
)

type txVersionV1TestSuite struct {
	suite.Suite

	Version TxVersion
}

func (s *txVersionV1TestSuite) SetupTest() {
	s.Version = &txV1{}
}

func TestTxVersionV1Suit(t *testing.T) {
	suite.Run(t, new(txVersionV1TestSuite))
}
