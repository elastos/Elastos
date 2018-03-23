package payload

import "io"

const IssueTokenPayloadVersion byte = 0x00

type IssueToken struct {
}

func (a *IssueToken) Data(version byte) []byte {
	//TODO: implement IssueToken.Data()
	return []byte{0}

}

func (a *IssueToken) Serialize(w io.Writer, version byte) error {
	return nil
}

func (a *IssueToken) Deserialize(r io.Reader, version byte) error {
	return nil
}
