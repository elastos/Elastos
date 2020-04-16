package mempool

import (
	"bytes"
	"encoding/json"
	"errors"
	"math"
	"strings"
	"time"

	"github.com/btcsuite/btcutil/base58"
	"github.com/syndtr/goleveldb/leveldb"

	"github.com/elastos/Elastos.ELA.SideChain.ID/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain.ID/pact"
	id "github.com/elastos/Elastos.ELA.SideChain.ID/types"
	"github.com/elastos/Elastos.ELA.SideChain.ID/types/base64url"
	"github.com/elastos/Elastos.ELA.SideChain/mempool"
	"github.com/elastos/Elastos.ELA.SideChain/service"
	"github.com/elastos/Elastos.ELA.SideChain/spv"
	"github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA.SideChain/vm"
	"github.com/elastos/Elastos.ELA.SideChain/vm/interfaces"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/crypto"
)

const (
	CheckRegisterDIDFuncName = "checkregisterdid"
	CheckUpdateDIDFuncName   = "checkupdatedid"
)
const PrefixCRDID contract.PrefixType = 0x67

func CreateCRDIDContractByCode(code []byte) (*contract.Contract, error) {
	if len(code) == 0 {
		return nil, errors.New("code is nil")
	}
	return &contract.Contract{
		Code:   code,
		Prefix: PrefixCRDID,
	}, nil
}

type validator struct {
	*mempool.Validator

	systemAssetID common.Uint256
	foundation    common.Uint168
	spvService    *spv.Service
	Store         *blockchain.IDChainStore
}

func NewValidator(cfg *mempool.Config, store *blockchain.IDChainStore) *mempool.Validator {
	var val validator
	val.Validator = mempool.NewValidator(cfg)
	val.systemAssetID = cfg.ChainParams.ElaAssetId
	val.foundation = cfg.ChainParams.Foundation
	val.spvService = cfg.SpvService
	val.Store = store
	val.RegisterSanityFunc(mempool.FuncNames.CheckTransactionOutput, val.checkTransactionOutput)
	val.RegisterSanityFunc(mempool.FuncNames.CheckTransactionPayload, val.checkTransactionPayload)
	val.RegisterSanityFunc(CheckRegisterDIDFuncName, val.checkRegisterDID)

	val.RegisterContextFunc(mempool.FuncNames.CheckTransactionSignature, val.checkTransactionSignature)
	return val.Validator
}

func (v *validator) checkTransactionPayload(txn *types.Transaction) error {
	switch pld := txn.Payload.(type) {
	case *types.PayloadRegisterAsset:
		if pld.Asset.Precision < types.MinPrecision || pld.Asset.Precision > types.MaxPrecision {
			return errors.New("[ID CheckTransactionPayload] Invalide asset Precision.")
		}
		if !checkAmountPrecise(pld.Amount, pld.Asset.Precision, types.MaxPrecision) {
			return errors.New("[ID CheckTransactionPayload] Invalide asset value,out of precise.")
		}
	case *types.PayloadTransferAsset:
	case *types.PayloadRecord:
	case *types.PayloadCoinBase:
	case *types.PayloadRechargeToSideChain:
	case *types.PayloadTransferCrossChainAsset:
	case *id.PayloadRegisterIdentification:
	case *id.Operation:
	default:
		return errors.New("[ID CheckTransactionPayload] [txValidator],invalidate transaction payload type.")
	}
	return nil
}

func checkAmountPrecise(amount common.Fixed64, precision byte, assetPrecision byte) bool {
	return amount.IntValue()%int64(math.Pow10(int(assetPrecision-precision))) == 0
}

func (v *validator) checkTransactionOutput(txn *types.Transaction) error {
	if txn.IsCoinBaseTx() {
		if len(txn.Outputs) < 2 {
			return errors.New("[checkTransactionOutput] coinbase output is not enough, at least 2")
		}

		var totalReward = common.Fixed64(0)
		var foundationReward = common.Fixed64(0)
		for _, output := range txn.Outputs {
			if !output.AssetID.IsEqual(v.systemAssetID) {
				return errors.New("[checkTransactionOutput] asset ID in coinbase is invalid")
			}
			totalReward += output.Value
			if output.ProgramHash.IsEqual(v.foundation) {
				foundationReward += output.Value
			}
		}
		if common.Fixed64(foundationReward) < common.Fixed64(float64(totalReward)*0.3) {
			return errors.New("[checkTransactionOutput] Reward to foundation in coinbase < 30%")
		}

		return nil
	}

	if len(txn.Outputs) < 1 {
		return errors.New("[checkTransactionOutput] transaction has no outputs")
	}

	// check if output address is valid
	for _, output := range txn.Outputs {
		if output.AssetID != v.systemAssetID {
			return errors.New("[checkTransactionOutput] asset ID in output is invalid")
		}

		if !checkOutputProgramHash(output.ProgramHash) {
			return errors.New("[checkTransactionOutput] output address is invalid")
		}
	}

	return nil
}

func checkOutputProgramHash(programHash common.Uint168) bool {
	switch contract.PrefixType(programHash[0]) {
	case contract.PrefixStandard, contract.PrefixMultiSig, contract.PrefixCrossChain,
		pact.PrefixRegisterId:
		return true
	}
	var empty = common.Uint168{}
	if programHash == empty {
		return true
	}
	return false
}

func (v *validator) checkTransactionSignature(txn *types.Transaction) error {
	if txn.IsRechargeToSideChainTx() {
		if err := v.spvService.VerifyTransaction(txn); err != nil {
			return errors.New("[ID checkTransactionSignature] Invalide recharge to side chain tx: " + err.Error())
		}
		return nil
	}

	hashes, err := v.TxProgramHashes(txn)
	if err != nil {
		return errors.New("[ID checkTransactionSignature] Get program hashes error:" + err.Error())
	}

	// Add ID program hash to hashes
	if id.IsRegisterIdentificationTx(txn) {
		for _, output := range txn.Outputs {
			if output.ProgramHash[0] == pact.PrefixRegisterId {
				hashes = append(hashes, output.ProgramHash)
				break
			}
		}
	}

	// Sort first
	common.SortProgramHashByCodeHash(hashes)
	if err := mempool.SortPrograms(txn.Programs); err != nil {
		return errors.New("[ID checkTransactionSignature] Sort program hashes error:" + err.Error())
	}

	err = mempool.RunPrograms(txn, hashes, txn.Programs)
	if err != nil {
		return errors.New("[ID checkTransactionSignature] Run program error:" + err.Error())
	}

	return nil
}

func getUriSegment(uri string) string {
	index := strings.LastIndex(uri, "#")
	if index == -1 {
		return ""
	}
	return uri[index:]
}

//DIDProofInfo VerificationMethod must be in DIDPayloadInfo Authentication or
//is did publickKey
func (v *validator) checkVerificationMethod(proof *id.DIDProofInfo,
	payloadInfo *id.DIDPayloadInfo) error {
	proofUriSegment := getUriSegment(proof.VerificationMethod)
	for _, auth := range payloadInfo.Authentication {
		switch auth.(type) {
		case string:
			keyString := auth.(string)
			if proofUriSegment == getUriSegment(keyString) {
				return nil
			}
		case map[string]interface{}:
			data, err := json.Marshal(auth)
			if err != nil {
				return err
			}
			didPublicKeyInfo := new(id.DIDPublicKeyInfo)
			err = json.Unmarshal(data, didPublicKeyInfo)
			if err != nil {
				return err
			}
			if proofUriSegment == getUriSegment(didPublicKeyInfo.ID) {
				return nil
			}
		default:
			return errors.New("[ID checkVerificationMethod] invalid  auth.(type)")
		}
	}
	//if not in Authentication
	//VerificationMethod uri -------->to find publicKeyBase58 in publicKey array which id is
	//VerificationMethod uri and publicKeyBase58 can derive id address
	for i := 0; i < len(payloadInfo.PublicKey); i++ {
		//get PublicKeyBase58 accord to VerificationMethod
		if proofUriSegment == getUriSegment(payloadInfo.PublicKey[i].ID) {
			pubKeyByte := base58.Decode(payloadInfo.PublicKey[i].PublicKeyBase58)
			//get did address
			didAddress, err := getDIDAdress(pubKeyByte)
			if err != nil {
				return err
			}
			//didAddress must equal address in DID
			if didAddress == v.Store.GetDIDFromUri(payloadInfo.ID) {
				return nil
			}
		}
	}
	return errors.New("[ID checkVerificationMethod] wrong public key by VerificationMethod ")
}

func getDIDByPublicKey(publicKey []byte) (*common.Uint168, error) {
	pk, _ := crypto.DecodePoint(publicKey)
	redeemScript, err := contract.CreateStandardRedeemScript(pk)
	if err != nil {
		return nil, err
	}
	return getDIDHashByCode(redeemScript)
}

func getDIDHashByCode(code []byte) (*common.Uint168, error) {
	ct1, error := CreateCRDIDContractByCode(code)
	if error != nil {
		return nil, error
	}
	return ct1.ToProgramHash(), error
}

func getDIDAdress(publicKey []byte) (string, error) {
	hash, err := getDIDByPublicKey(publicKey)
	if err != nil {
		return "", err
	}
	return hash.ToAddress()
}

func getPublicKey(proof *id.DIDProofInfo, payloadInfo *id.DIDPayloadInfo) string {
	proofUriSegment := getUriSegment(proof.VerificationMethod)

	for _, pkInfo := range payloadInfo.PublicKey {
		if proofUriSegment == getUriSegment(pkInfo.ID) {
			return pkInfo.PublicKeyBase58
		}
	}
	return ""
}

func getParameterBySignature(signature []byte) []byte {
	buf := new(bytes.Buffer)
	buf.WriteByte(byte(len(signature)))
	buf.Write(signature)
	return buf.Bytes()
}

func getCodeByPubKey(publicKey []byte) ([]byte, error) {
	pk, err := crypto.DecodePoint(publicKey)
	if err != nil {
		return nil, err
	}
	code, err2 := contract.CreateStandardRedeemScript(pk)
	if err2 != nil {
		return nil, err2
	}
	return code, nil
}

func (v *validator) VerifyByVM(iDateContainer interfaces.IDataContainer,
	code []byte,
	signature []byte) (bool, error) {
	se := vm.NewExecutionEngine(iDateContainer,
		new(vm.CryptoECDsa), vm.MAXSTEPS, nil, nil)

	se.LoadScript(code, false)
	se.LoadScript(getParameterBySignature(signature), true)
	//execute program on VM
	se.Execute()

	if se.GetState() != vm.HALT {
		return false, errors.New("[VM] Finish State not equal to HALT")
	}

	if se.GetEvaluationStack().Count() != 1 {
		return false, errors.New("[VM] Execute Engine Stack Count Error")
	}

	success := se.GetExecuteResult()
	if !success {
		return false, errors.New("[VM] Check Sig FALSE")
	}
	return true, nil
}

//check operateion create---->db must not have
//                 update----->db must have
func (v *validator) checkDIDOperation(header *id.DIDHeaderInfo,
	idUri string) error {
	did := v.Store.GetDIDFromUri(idUri)
	if did == "" {
		return errors.New("WRONG DID FORMAT")
	}

	buf := new(bytes.Buffer)
	buf.WriteString(did)
	lastTXData, err := v.Store.GetLastDIDTxData(buf.Bytes())

	dbExist := true
	if err != nil {
		if err.Error() == leveldb.ErrNotFound.Error() {
			dbExist = false
		} else {
			return err
		}
	}
	if dbExist {
		if header.Operation == id.Create_DID_Operation {
			return errors.New("DID WRONG OPERATION ALREADY EXIST")
		} else if header.Operation == id.Update_DID_Operation {
			//check PreviousTxid
			hash, err := common.Uint256FromHexString(header.PreviousTxid)
			if err != nil {
				return err
			}
			preTXID := service.ToReversedString(*hash)

			if lastTXData.TXID != preTXID {
				return errors.New("PreviousTxid IS NOT CORRECT")
			}
		}
	} else {
		if header.Operation == id.Update_DID_Operation {
			return errors.New("DID WRONG OPERATION NOT EXIST")
		}
	}
	return nil
}

func (v *validator) checkRegisterDID(txn *types.Transaction) error {
	//payload type check
	if txn.TxType != id.RegisterDID {
		return nil
	}
	payloadDidInfo, ok := txn.Payload.(*id.Operation)
	if !ok {
		return errors.New("invalid Operation")
	}

	_, err := time.Parse(time.RFC3339, payloadDidInfo.PayloadInfo.Expires)
	if err != nil {
		return errors.New("invalid Expires")
	}

	if err := v.checkDIDOperation(&payloadDidInfo.Header,
		payloadDidInfo.PayloadInfo.ID); err != nil {
		return err
	}

	if err := v.checkVerificationMethod(&payloadDidInfo.Proof,
		payloadDidInfo.PayloadInfo); err != nil {
		return err
	}

	//get  public key
	publicKeyBase58 := getPublicKey(&payloadDidInfo.Proof,
		payloadDidInfo.PayloadInfo)
	//get code
	//var publicKeyByte []byte
	publicKeyByte := base58.Decode(publicKeyBase58)

	//var code []byte
	code, err := getCodeByPubKey(publicKeyByte)
	if err != nil {
		return err
	}
	signature, _ := base64url.DecodeString(payloadDidInfo.Proof.Signature)

	var success bool
	success, err = v.VerifyByVM(payloadDidInfo, code, signature)
	if err != nil {
		return err
	}
	if !success {
		return errors.New("[VM] Check Sig FALSE")
	}
	return nil
}
