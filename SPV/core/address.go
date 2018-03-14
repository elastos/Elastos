package core

type Address struct {
	Address      string
	ProgramHash  *Uint168
	RedeemScript []byte
}
