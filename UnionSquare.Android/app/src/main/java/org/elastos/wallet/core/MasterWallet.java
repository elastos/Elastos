// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

package org.elastos.wallet.core;

import org.elastos.wallet.ela.utils.Log;

import java.util.ArrayList;

/**
 * MasterWallet
 */
public class MasterWallet {
    static public class CHAINID {
        public static String MAIN = "ELA";
        public static String ID = "IDChain";
    }

    static public String TAG = "IMasterWallet";

    private long mInstance;
    private ArrayList<SubWallet> mSubWallets = new ArrayList<SubWallet>();

    public MasterWallet(long instance) throws WalletException {
        mInstance = instance;
        Object[] allSubWallets = GetAllSubWallets(mInstance);
        for (int i = 0; i < allSubWallets.length; i++) {
            mSubWallets.add((SubWallet) allSubWallets[i]);
        }
    }

    public String GetID() {
        return GetID(mInstance);
    }

    public String GetBasicInfo() throws WalletException {
        return GetBasicInfo(mInstance);
    }

    public ArrayList<SubWallet> GetAllSubWallets() throws WalletException {
        Object[] allSubWallets = GetAllSubWallets(mInstance);

        for (int i = 0; i < allSubWallets.length; i++) {
            SubWallet subWallet = (SubWallet) allSubWallets[i];
            boolean found = false;
            for (int j = 0; j < mSubWallets.size(); ++j) {
                if (mSubWallets.get(j).GetChainID().equals(subWallet.GetChainID())) {
                    found = true;
                    break;
                }
            }
            if (!found)
                mSubWallets.add(subWallet);
        }

        return mSubWallets;
    }

    public SubWallet GetSubWallet(String chainID) throws WalletException {
        for (int i = 0; i < mSubWallets.size(); i++) {
            if (chainID.equals(mSubWallets.get(i).GetChainID())) {
                return mSubWallets.get(i);
            }
        }

        return null;
    }

    public SubWallet CreateSubWallet(String chainID) throws WalletException {
        if ((!CHAINID.MAIN.equals(chainID)) && (!CHAINID.ID.equals(chainID))) {
            throw new WalletException("Not support the other sidechain now.");
        }

        for (int i = 0; i < mSubWallets.size(); ++i) {
            if (mSubWallets.get(i).GetChainID().equals(chainID)) {
                return mSubWallets.get(i);
            }
        }

        long subProxy = CreateSubWallet(mInstance, chainID);
        if (subProxy == 0) {
            Log.e(TAG, "Native create subwallet fail: subProxy is 0");
            throw new WalletException("Native create subwallet fail");
        }

        SubWallet subWallet = null;

        if (CHAINID.MAIN.equals(chainID)) {
            subWallet = new MainchainSubWallet(subProxy);
        } else if (CHAINID.ID.equals(chainID)) {
            subWallet = new IDChainSubWallet(subProxy);
        } else {
            throw new WalletException("Unsupport chainID: " + chainID);
        }

        mSubWallets.add(subWallet);

        return subWallet;
    }

    public void DestroyWallet(SubWallet wallet) throws WalletException {
        for (int i = 0; i < mSubWallets.size(); ++i) {
            if (mSubWallets.get(i).GetChainID().equals(wallet.GetChainID())) {
                mSubWallets.remove(i);
                break;
            }
        }
        wallet.RemoveCallback();
        DestroyWallet(mInstance, wallet.GetChainID());
    }

    public String GetOwnerPublicKeyRing() throws WalletException {
        return GetOwnerPublicKeyRing(mInstance);
    }

    public String GetPublicKeyRing() throws WalletException {
        return GetPublicKeyRing(mInstance);
    }

    public boolean IsAddressValid(String address) throws WalletException {
        return IsAddressValid(mInstance, address);
    }

    public String[] GetSupportedChains() throws WalletException {
        return GetSupportedChains(mInstance);
    }

    public long GetInstance() {
        return mInstance;
    }

    public void ChangePassword(String oldPassword, String newPassword) throws WalletException {
        ChangePassword(mInstance, oldPassword, newPassword);
    }

    public String GetPubKeyInfo() throws WalletException {
        return GetPubKeyInfo(mInstance);
    }

    public boolean VerifyPrivateKey(String mnemonic, String passphrase) throws WalletException {
        return VerifyPrivateKey(mInstance, mnemonic, passphrase);
    }

    public boolean VerifyPassPhrase(String passPhrase, String payPasswd) throws WalletException {
        return VerifyPassPhrase(mInstance, passPhrase, payPasswd);
    }

    public boolean VerifyPayPassword(String payPasswd) throws WalletException {
        return VerifyPayPassword(mInstance, payPasswd);
    }

    public String ExportKeystore(String backPasswd, String payPasswd) throws WalletException {
        return ExportKeystore(mInstance, backPasswd, payPasswd);
    }

    public String ExportMnemonic(String payPasswd) throws WalletException {
        return ExportMnemonic(mInstance, payPasswd);
    }

    public String ExportReadonlyWallet() throws WalletException {
        return ExportReadonlyWallet(mInstance);
    }

    public String ExportPrivateKey(String payPasswd) throws WalletException {
        return ExportPrivateKey(mInstance, payPasswd);
    }

    public String ExportMasterPublicKey() throws WalletException {
        return ExportMasterPublicKey(mInstance);
    }

    private native String GetID(long instance);

    private native String GetBasicInfo(long instance);

    private native Object[] GetAllSubWallets(long instance);

    private native long GetSubWallet(long instance, String chainID);

    private native long CreateSubWallet(long instance, String chainID);

    private native String GetOwnerPublicKeyRing(long instance);

    private native String GetPublicKeyRing(long instance);

    private native void DestroyWallet(long instance, String chainID);

    private native boolean IsAddressValid(long instance, String address);

    private native String[] GetSupportedChains(long instance);

    private native void ChangePassword(long instance, String oldPassword, String newPassword);

    private native String GetPubKeyInfo(long instance);

    private native boolean VerifyPrivateKey(long instance, String mnemonic, String passphrase);

    private native boolean VerifyPassPhrase(long instance, String passPhrase, String payPasswd);

    private native boolean VerifyPayPassword(long instance, String payPasswd);

    private native String ExportKeystore(long instance, String backPasswd, String payPasswd);

    private native String ExportMnemonic(long instance, String payPasswd);

    private native String ExportReadonlyWallet(long instance);

    private native String ExportPrivateKey(long instance, String payPasswd);

    private native String ExportMasterPublicKey(long instance);
}
