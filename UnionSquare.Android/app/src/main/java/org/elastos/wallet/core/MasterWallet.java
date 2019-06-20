// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

package org.elastos.wallet.core;

import android.util.Log;

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

    public MasterWallet(long instance) {
        mInstance = instance;
        Object[] allSubWallets = GetAllSubWallets(mInstance);
        for (int i = 0; i < allSubWallets.length; i++) {
            mSubWallets.add((SubWallet) allSubWallets[i]);
        }
    }

    public String GetID() {
        return GetID(mInstance);
    }

    public String GetBasicInfo() {
        return GetBasicInfo(mInstance);
    }

    public ArrayList<SubWallet> GetAllSubWallets() {
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

    public SubWallet GetSubWallet(String chainID) {
        for (int i = 0; i < mSubWallets.size(); i++) {
            if (chainID.equals(mSubWallets.get(i).GetChainID())) {
                return mSubWallets.get(i);
            }
        }

        return null;
    }

    public SubWallet CreateSubWallet(String chainID, long feePerKb) throws WalletException {
        if ((!CHAINID.MAIN.equals(chainID)) && (!CHAINID.ID.equals(chainID))) {
            throw new WalletException("Not support the other sidechain now.");
        }

        for (int i = 0; i < mSubWallets.size(); ++i) {
            if (mSubWallets.get(i).GetChainID().equals(chainID)) {
                return mSubWallets.get(i);
            }
        }

        long subProxy = CreateSubWallet(mInstance, chainID, feePerKb);
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

    public void DestroyWallet(SubWallet wallet) {
        for (int i = 0; i < mSubWallets.size(); ++i) {
            if (mSubWallets.get(i).GetChainID().equals(wallet.GetChainID())) {
                mSubWallets.remove(i);
                break;
            }
        }
        wallet.RemoveCallback();
        DestroyWallet(mInstance, wallet.GetProxy());
    }

    public String GetPublicKey() {
        return GetPublicKey(mInstance);
    }

    public String Sign(String message, String payPassword) throws WalletException {
        return Sign(mInstance, message, payPassword);
    }

    public boolean CheckSign(String publicKey, String message, String signature) throws WalletException {
        return CheckSign(mInstance, publicKey, message, signature);
    }


    public boolean IsAddressValid(String address) {
        return IsAddressValid(mInstance, address);
    }

    public String[] GetSupportedChains() {
        return GetSupportedChains(mInstance);
    }

    public long GetInstance() {
        return mInstance;
    }

    public void ChangePassword(String oldPassword, String newPassword) throws WalletException {
        ChangePassword(mInstance, oldPassword, newPassword);
    }

    private native String GetID(long instance);

    private native String GetBasicInfo(long instance);

    private native Object[] GetAllSubWallets(long instance);

    private native long CreateSubWallet(long instance, String chainID, long feePerKb);

    private native String GetPublicKey(long instance);

    private native void DestroyWallet(long instance, long subWalletProxy);

    private native String Sign(long instance, String message, String payPassword);

    private native boolean CheckSign(long instance, String publicKey, String message, String signature);

    private native boolean IsAddressValid(long instance, String address);

    private native String[] GetSupportedChains(long instance);

    private native void ChangePassword(long instance, String oldPassword, String newPassword);
}
