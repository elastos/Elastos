// Copyright 2014 The Elastos.ELA.SideChain.ETH Authors
// This file is part of the Elastos.ELA.SideChain.ETH library.
//
// The Elastos.ELA.SideChain.ETH library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The Elastos.ELA.SideChain.ETH library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with the Elastos.ELA.SideChain.ETH library. If not, see <http://www.gnu.org/licenses/>.

package core

import (
	"bytes"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"math/big"
	"strings"

	"github.com/elastos/Elastos.ELA.SideChain.ETH/common"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/common/hexutil"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/common/math"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/core/rawdb"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/core/state"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/core/types"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/ethdb"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/log"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/params"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/rlp"
)

//go:generate gencodec -type Genesis -field-override genesisSpecMarshaling -out gen_genesis.go
//go:generate gencodec -type GenesisAccount -field-override genesisAccountMarshaling -out gen_genesis_account.go

var errGenesisNoConfig = errors.New("genesis has no chain configuration")

// Genesis specifies the header fields, state of a genesis block. It also defines hard
// fork switch-over blocks through the chain configuration.
type Genesis struct {
	Config     *params.ChainConfig `json:"config"`
	Nonce      uint64              `json:"nonce"`
	Timestamp  uint64              `json:"timestamp"`
	ExtraData  []byte              `json:"extraData"`
	GasLimit   uint64              `json:"gasLimit"   gencodec:"required"`
	Difficulty *big.Int            `json:"difficulty" gencodec:"required"`
	Mixhash    common.Hash         `json:"mixHash"`
	Coinbase   common.Address      `json:"coinbase"`
	Alloc      GenesisAlloc        `json:"alloc"      gencodec:"required"`

	// These fields are used for consensus tests. Please don't use them
	// in actual genesis blocks.
	Number     uint64      `json:"number"`
	GasUsed    uint64      `json:"gasUsed"`
	ParentHash common.Hash `json:"parentHash"`
}

// GenesisAlloc specifies the initial state that is part of the genesis block.
type GenesisAlloc map[common.Address]GenesisAccount

func (ga *GenesisAlloc) UnmarshalJSON(data []byte) error {
	m := make(map[common.UnprefixedAddress]GenesisAccount)
	if err := json.Unmarshal(data, &m); err != nil {
		return err
	}
	*ga = make(GenesisAlloc)
	for addr, a := range m {
		(*ga)[common.Address(addr)] = a
	}
	return nil
}

// GenesisAccount is an account in the state of the genesis block.
type GenesisAccount struct {
	Code       []byte                      `json:"code,omitempty"`
	Storage    map[common.Hash]common.Hash `json:"storage,omitempty"`
	Balance    *big.Int                    `json:"balance" gencodec:"required"`
	Nonce      uint64                      `json:"nonce,omitempty"`
	PrivateKey []byte                      `json:"secretKey,omitempty"` // for tests
}

// field type overrides for gencodec
type genesisSpecMarshaling struct {
	Nonce      math.HexOrDecimal64
	Timestamp  math.HexOrDecimal64
	ExtraData  hexutil.Bytes
	GasLimit   math.HexOrDecimal64
	GasUsed    math.HexOrDecimal64
	Number     math.HexOrDecimal64
	Difficulty *math.HexOrDecimal256
	Alloc      map[common.UnprefixedAddress]GenesisAccount
}

type genesisAccountMarshaling struct {
	Code       hexutil.Bytes
	Balance    *math.HexOrDecimal256
	Nonce      math.HexOrDecimal64
	Storage    map[storageJSON]storageJSON
	PrivateKey hexutil.Bytes
}

// storageJSON represents a 256 bit byte array, but allows less than 256 bits when
// unmarshaling from hex.
type storageJSON common.Hash

func (h *storageJSON) UnmarshalText(text []byte) error {
	text = bytes.TrimPrefix(text, []byte("0x"))
	if len(text) > 64 {
		return fmt.Errorf("too many hex characters in storage key/value %q", text)
	}
	offset := len(h) - len(text)/2 // pad on the left
	if _, err := hex.Decode(h[offset:], text); err != nil {
		fmt.Println(err)
		return fmt.Errorf("invalid hex storage key/value %q", text)
	}
	return nil
}

func (h storageJSON) MarshalText() ([]byte, error) {
	return hexutil.Bytes(h[:]).MarshalText()
}

// GenesisMismatchError is raised when trying to overwrite an existing
// genesis block with an incompatible one.
type GenesisMismatchError struct {
	Stored, New common.Hash
}

func (e *GenesisMismatchError) Error() string {
	return fmt.Sprintf("database already contains an incompatible genesis block (have %x, new %x)", e.Stored[:8], e.New[:8])
}

// SetupGenesisBlock writes or updates the genesis block in db.
// The block that will be used is:
//
//                          genesis == nil       genesis != nil
//                       +------------------------------------------
//     db has no genesis |  main-net default  |  genesis
//     db has genesis    |  from DB           |  genesis (if compatible)
//
// The stored chain configuration will be updated if it is compatible (i.e. does not
// specify a fork block below the local head block). In case of a conflict, the
// error is a *params.ConfigCompatError and the new, unwritten config is returned.
//
// The returned chain configuration is never nil.
func SetupGenesisBlock(db ethdb.Database, genesis *Genesis) (*params.ChainConfig, common.Hash, error) {
	if genesis != nil && genesis.Config == nil {
		return params.AllEthashProtocolChanges, common.Hash{}, errGenesisNoConfig
	}

	// Just commit the new block if there is no stored genesis block.
	stored := rawdb.ReadCanonicalHash(db, 0)
	if (stored == common.Hash{}) {
		if genesis == nil {
			log.Info("Writing default main net genesis block")
			genesis = DefaultGenesisBlock()

		} else {
			log.Info("Writing custom genesis block")
		}
		block, err := genesis.Commit(db)
		return genesis.Config, block.Hash(), err
	}

	// Check whether the genesis block is already written.
	if genesis != nil {
		hash := genesis.ToBlock(nil).Hash()
		if hash != stored {
			return genesis.Config, hash, &GenesisMismatchError{stored, hash}
		}
	}

	// Get the existing chain configuration.
	newcfg := genesis.configOrDefault(stored)
	storedcfg := rawdb.ReadChainConfig(db, stored)

	if storedcfg == nil {
		log.Warn("Found genesis block without chain config")
		rawdb.WriteChainConfig(db, stored, newcfg)
		return newcfg, stored, nil
	}
	// Special case: don't change the existing config of a non-mainnet chain if no new
	// config is supplied. These chains would get AllProtocolChanges (and a compat error)
	// if we just continued here.
	if genesis == nil && stored != params.MainnetGenesisHash {
		return storedcfg, stored, nil
	}

	// Check config compatibility and write the config. Compatibility errors
	// are returned to the caller unless we're already at block zero.
	height := rawdb.ReadHeaderNumber(db, rawdb.ReadHeadHeaderHash(db))
	if height == nil {
		return newcfg, stored, fmt.Errorf("missing block number for head header hash")
	}
	compatErr := storedcfg.CheckCompatible(newcfg, *height)
	if compatErr != nil && *height != 0 && compatErr.RewindTo != 0 {
		return newcfg, stored, compatErr
	}
	rawdb.WriteChainConfig(db, stored, newcfg)
	return newcfg, stored, nil
}

func (g *Genesis) configOrDefault(ghash common.Hash) *params.ChainConfig {
	switch {
	case g != nil:
		return g.Config
	case ghash == params.MainnetGenesisHash:
		return params.MainnetChainConfig
	case ghash == params.TestnetGenesisHash:
		return params.TestnetChainConfig
	default:
		return params.AllEthashProtocolChanges
	}
}

// ToBlock creates the genesis block and writes state of a genesis specification
// to the given database (or discards it if nil).
func (g *Genesis) ToBlock(db ethdb.Database) *types.Block {
	if db == nil {
		db = ethdb.NewMemDatabase()
	}
	statedb, _ := state.New(common.Hash{}, state.NewDatabase(db))
	for addr, account := range g.Alloc {
		statedb.AddBalance(addr, account.Balance)
		statedb.SetCode(addr, account.Code)
		statedb.SetNonce(addr, account.Nonce)
		for key, value := range account.Storage {
			statedb.SetState(addr, key, value)
		}
	}
	root := statedb.IntermediateRoot(false)
	head := &types.Header{
		Number:     new(big.Int).SetUint64(g.Number),
		Nonce:      types.EncodeNonce(g.Nonce),
		Time:       new(big.Int).SetUint64(g.Timestamp),
		ParentHash: g.ParentHash,
		Extra:      g.ExtraData,
		GasLimit:   g.GasLimit,
		GasUsed:    g.GasUsed,
		Difficulty: g.Difficulty,
		MixDigest:  g.Mixhash,
		Coinbase:   g.Coinbase,
		Root:       root,
	}
	if g.GasLimit == 0 {
		head.GasLimit = params.GenesisGasLimit
	}
	if g.Difficulty == nil {
		head.Difficulty = params.GenesisDifficulty
	}
	statedb.Commit(false)
	statedb.Database().TrieDB().Commit(root, true)

	return types.NewBlock(head, nil, nil, nil)
}

// Commit writes the block and state of a genesis specification to the database.
// The block is committed as the canonical head block.
func (g *Genesis) Commit(db ethdb.Database) (*types.Block, error) {
	block := g.ToBlock(db)
	if block.Number().Sign() != 0 {
		return nil, fmt.Errorf("can't commit genesis block with number > 0")
	}
	rawdb.WriteTd(db, block.Hash(), block.NumberU64(), g.Difficulty)
	rawdb.WriteBlock(db, block)
	rawdb.WriteReceipts(db, block.Hash(), block.NumberU64(), nil)
	rawdb.WriteCanonicalHash(db, block.Hash(), block.NumberU64())
	rawdb.WriteHeadBlockHash(db, block.Hash())
	rawdb.WriteHeadHeaderHash(db, block.Hash())

	config := g.Config
	if config == nil {
		config = params.AllEthashProtocolChanges
	}
	rawdb.WriteChainConfig(db, block.Hash(), config)
	return block, nil
}

// MustCommit writes the genesis block and state to db, panicking on error.
// The block is committed as the canonical head block.
func (g *Genesis) MustCommit(db ethdb.Database) *types.Block {
	block, err := g.Commit(db)
	if err != nil {
		panic(err)
	}
	return block
}

// GenesisBlockForTesting creates and writes a block in which addr has the given wei balance.
func GenesisBlockForTesting(db ethdb.Database, addr common.Address, balance *big.Int) *types.Block {
	g := Genesis{Alloc: GenesisAlloc{addr: {Balance: balance}}}
	return g.MustCommit(db)
}

// DefaultGenesisBlock returns the Ethereum main net genesis block.
func DefaultGenesisBlock() *Genesis {
	genesis := &Genesis{
		Config:     params.MainnetChainConfig,
		Timestamp:  0x1,
		GasLimit:   0x2068F7700,
		Difficulty: big.NewInt(1),
		Alloc:      nil,
	}
	extra := make([]byte, 0)
	extra = append(extra, bytes.Repeat([]byte{0x00}, 32)...)
	address1 := hexutil.MustDecode("0x5141f2c88e84c0f9b4c1876df59e2530fbdc42f2")
	address2 := hexutil.MustDecode("0xf02090aec7d41b1880058cf5155fe030c3ec404e")
	address3 := hexutil.MustDecode("0x08319622794adcf7c05d9e8f6d251c3309e0ec3c")
	address4 := hexutil.MustDecode("0x8609fadecdc27e135343b7c8bdbf25f09a014582")
	address5 := hexutil.MustDecode("0x77466b8b6fbb66ac1db38343af158b2263699678")
	address6 := hexutil.MustDecode("0x8207c68a3345104698ae24c4847bf748a32c97d1")
	address7 := hexutil.MustDecode("0x58f0d33873227887ecf514064e3a7b94754ecb68")
	address8 := hexutil.MustDecode("0x470e2a9002b6aa3197cbec11f46971c73d79a8e8")
	address9 := hexutil.MustDecode("0x0ddb5d59d98347dbbc60c63d09213e5fac0cbc6d")
	address10 := hexutil.MustDecode("0x8f547cedf3b298182abf49be4a717db9a0d14509")
	address11 := hexutil.MustDecode("0xfddb0ca03ffc56463ec0e6dda91c99f70a7bca50")
	address12 := hexutil.MustDecode("0x5223c679c4bfdaee3463636ee4a8ff022b46e9eb")
	extra = append(extra, address1...)
	extra = append(extra, address2...)
	extra = append(extra, address3...)
	extra = append(extra, address4...)
	extra = append(extra, address5...)
	extra = append(extra, address6...)
	extra = append(extra, address7...)
	extra = append(extra, address8...)
	extra = append(extra, address9...)
	extra = append(extra, address10...)
	extra = append(extra, address11...)
	extra = append(extra, address12...)
	extra = append(extra, bytes.Repeat([]byte{0x00}, 65)...)
	genesis.ExtraData = extra
	return genesis
}

// DefaultTestnetGenesisBlock returns the Ropsten network genesis block.
func DefaultTestnetGenesisBlock() *Genesis {
	genesis := &Genesis{
		Config:     params.TestnetChainConfig,
		Timestamp:  0x5bda9da0,
		GasLimit:   0x2068F7700,
		Difficulty: big.NewInt(1),
		Alloc:      nil,
	}
	extra := make([]byte, 0)
	extra = append(extra, bytes.Repeat([]byte{0x00}, 32)...)
	address1 := hexutil.MustDecode("0xd7b0ddec94d96d4c7870deac1a2fe3347b9b4b85")
	address2 := hexutil.MustDecode("0x4dd0dd5e78c10842544cc1e88b5e1fcc3532abe1")
	address3 := hexutil.MustDecode("0xbb6ef39991b88e0121689a298d16b34dfca43156")
	address4 := hexutil.MustDecode("0x1d8b61c0300fa3b6167bd76ad82c90feab038af0")
	address5 := hexutil.MustDecode("0xf3ca004f36ee4d3510553564bdb81ab5f1a5d4ed")
	address6 := hexutil.MustDecode("0x412a0777ad9bed14c4d53a883f618eb86de1723d")
	address7 := hexutil.MustDecode("0xedabf5d5fb905ef2148dcf3fdc08d53f03d534a5")
	address8 := hexutil.MustDecode("0x143b49ff57efc134816017a5cd0b99058946781f")
	address9 := hexutil.MustDecode("0x8c60febab3495b66047aaaac8639a2d0bd911737")
	address10 := hexutil.MustDecode("0x473e7ea53fbf71e091703893dd9b6d5b96a83db1")
	address11 := hexutil.MustDecode("0xb8ae7e3346330073552e1b7c8403f4e107406ff7")
	address12 := hexutil.MustDecode("0xd168f2f37649f1a5cf1cbc173e02e2897ccd83b1")
	extra = append(extra, address1...)
	extra = append(extra, address2...)
	extra = append(extra, address3...)
	extra = append(extra, address4...)
	extra = append(extra, address5...)
	extra = append(extra, address6...)
	extra = append(extra, address7...)
	extra = append(extra, address8...)
	extra = append(extra, address9...)
	extra = append(extra, address10...)
	extra = append(extra, address11...)
	extra = append(extra, address12...)
	extra = append(extra, bytes.Repeat([]byte{0x00}, 65)...)
	genesis.ExtraData = extra
	return genesis

}

// DefaultRinkebyGenesisBlock returns the Rinkeby network genesis block.
func DefaultRinkebyGenesisBlock() *Genesis {
	genesis := &Genesis{
		Config:     params.RinkebyChainConfig,
		Timestamp:  0x5bda9da8,
		GasLimit:   0x2068F7700,
		Difficulty: big.NewInt(1),
		Alloc:      nil,
	}
	extra := make([]byte, 0)
	extra = append(extra, bytes.Repeat([]byte{0x00}, 32)...)
	address1 := hexutil.MustDecode("0x7117360e9165f11c51c4231be16c602a5dd250b6")
	address2 := hexutil.MustDecode("0xaeabe3dd7b80adf0a884e572e84daf5eba0aa4ae")
	address3 := hexutil.MustDecode("0x8bb95e8f1ec991b71789497a09fd4997c468c3c2")
	address4 := hexutil.MustDecode("0x77cd931d4864039fa48220f817519f5a8b0715b3")
	address5 := hexutil.MustDecode("0x34185174992c36c79f78da5e009b9c22732738d6")
	address6 := hexutil.MustDecode("0x35ae1cbf8c5a01a40943aff7e0c8d5d3c625bf96")
	address7 := hexutil.MustDecode("0x971037c3e90bb239b5b219b7ac14c8e301dca5f8")
	address8 := hexutil.MustDecode("0x75ed22d90568dd99c56cb34c8d5204d610cb2b61")
	address9 := hexutil.MustDecode("0x07ebaaee24a9d8dd625ca9863af2347656df876a")
	address10 := hexutil.MustDecode("0x369c5f2b099abb3c050b8ba4355cf0dc29ebf429")
	address11 := hexutil.MustDecode("0x6e32b1fff289d05e719a58e1ad1fff1a924014c8")
	address12 := hexutil.MustDecode("0x72064cd776e12d7163d329cc003bffb1b8b9de44")
	extra = append(extra, address1...)
	extra = append(extra, address2...)
	extra = append(extra, address3...)
	extra = append(extra, address4...)
	extra = append(extra, address5...)
	extra = append(extra, address6...)
	extra = append(extra, address7...)
	extra = append(extra, address8...)
	extra = append(extra, address9...)
	extra = append(extra, address10...)
	extra = append(extra, address11...)
	extra = append(extra, address12...)
	extra = append(extra, bytes.Repeat([]byte{0x00}, 65)...)
	genesis.ExtraData = extra
	return genesis
}

// Convert a string to an int
func convertBigIntData(intNumber string) *big.Int {
	number, err := big.NewInt(0).SetString(intNumber, 10)
	if !err {
		fmt.Print(err)
		return big.NewInt(0)
	}
	return number
}

// DeveloperGenesisBlock returns the 'geth --dev' genesis block. Note, this must
// be seeded with the
func DeveloperGenesisBlock(period uint64, faucet common.Address) *Genesis {
	// Override the default period to the user requested one
	config := *params.AllCliqueProtocolChanges
	config.Clique.Period = period

	// Assemble and return the genesis with the precompiles and faucet pre-funded
	return &Genesis{
		Config:     &config,
		ExtraData:  append(append(make([]byte, 32), faucet[:]...), make([]byte, 65)...),
		GasLimit:   6283185,
		Difficulty: big.NewInt(1),
		Alloc: map[common.Address]GenesisAccount{
			common.BytesToAddress([]byte{1}): {Balance: big.NewInt(1)}, // ECRecover
			common.BytesToAddress([]byte{2}): {Balance: big.NewInt(1)}, // SHA256
			common.BytesToAddress([]byte{3}): {Balance: big.NewInt(1)}, // RIPEMD
			common.BytesToAddress([]byte{4}): {Balance: big.NewInt(1)}, // Identity
			common.BytesToAddress([]byte{5}): {Balance: big.NewInt(1)}, // ModExp
			common.BytesToAddress([]byte{6}): {Balance: big.NewInt(1)}, // ECAdd
			common.BytesToAddress([]byte{7}): {Balance: big.NewInt(1)}, // ECScalarMul
			common.BytesToAddress([]byte{8}): {Balance: big.NewInt(1)}, // ECPairing
			faucet: {Balance: new(big.Int).Sub(new(big.Int).Lsh(big.NewInt(1), 256), big.NewInt(9))},
		},
	}
}

func decodePrealloc(data string) GenesisAlloc {
	var p []struct{ Addr, Balance *big.Int }
	if err := rlp.NewStream(strings.NewReader(data), 0).Decode(&p); err != nil {
		panic(err)
	}
	ga := make(GenesisAlloc, len(p))
	for _, account := range p {
		ga[common.BigToAddress(account.Addr)] = GenesisAccount{Balance: account.Balance}
	}
	return ga
}
