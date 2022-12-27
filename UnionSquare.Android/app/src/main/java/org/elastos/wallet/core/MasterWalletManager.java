// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

package org.elastos.wallet.core;

import org.elastos.wallet.ela.utils.Log;

import java.util.ArrayList;

public class MasterWalletManager {
    private String TAG = "MasterWalletManager";
    private long mInstance;
    private String mRootPath;
    private ArrayList<MasterWallet> mMasterWallets = new ArrayList<MasterWallet>();

    private boolean MasterWalletExist(String walletID) {
        String[] allID = GetAllMasterWalletID();
        for (int i = 0; i < allID.length; ++i) {
            if (allID[i].equals(walletID)) {
                return true;
            }
        }

        return false;
    }

    public MasterWalletManager(String rootPath, String netType, String config, String dataPath) throws WalletException {
        mRootPath = rootPath;
        mInstance = InitMasterWalletManager(mRootPath, netType, config, dataPath);
    }

    public void Dispose() {
        Log.i(TAG, "Dispose");
        DisposeNative(mInstance);
    }

    public MasterWallet CreateMasterWallet(
            String masterWalletId, String mnemonic, String phrasePassword, String payPassword,
            boolean singleAddress) throws WalletException {

        if (MasterWalletExist(masterWalletId)) {
            Log.e(TAG, "Master wallet [" + masterWalletId + "] exist");
            return null;
        }

        long instance = CreateMasterWallet(mInstance, masterWalletId, mnemonic,
                phrasePassword, payPassword, singleAddress);

        if (instance == 0) {
            Log.e(TAG, "Create master wallet fail");
            return null;
        }

        MasterWallet masterWallet = new MasterWallet(instance);
        mMasterWallets.add(masterWallet);

        return masterWallet;
    }

    public ArrayList<MasterWallet> GetLoadedMasterWallets() throws WalletException {
        return mMasterWallets;
    }

    public ArrayList<MasterWallet> GetAllMasterWallets() throws WalletException {

        long[] masterWalletProxies = GetAllMasterWallets(mInstance);

        for (int i = 0; i < masterWalletProxies.length; i++) {
            MasterWallet masterWallet = new MasterWallet(masterWalletProxies[i]);
            boolean found = false;
            for (int j = 0; j < mMasterWallets.size(); ++j) {
                if (mMasterWallets.get(j).GetID().equals(masterWallet.GetID())) {
                    found = true;
                    break;
                }
            }

            if (!found)
                mMasterWallets.add(masterWallet);
        }

        return mMasterWallets;
    }

    public String[] GetAllMasterWalletID() throws WalletException {
        return GetAllMasterWalletID(mInstance);
    }

    public MasterWallet GetMasterWallet(String masterWalletId) throws WalletException {
        for (int i = 0; i < mMasterWallets.size(); ++i) {
            if (mMasterWallets.get(i).GetID().equals(masterWalletId))
                return mMasterWallets.get(i);
        }

        long instance = GetMasterWallet(mInstance, masterWalletId);
        if (instance == 0) {
            Log.e(TAG, "master wallet [" + masterWalletId + "] not found");
            return null;
        }

        MasterWallet masterWallet = new MasterWallet(instance);
        mMasterWallets.add(masterWallet);

        return masterWallet;
    }

    public void DestroyWallet(String masterWalletId) throws WalletException {
        for (int i = 0; i < mMasterWallets.size(); ++i) {
            if (mMasterWallets.get(i).GetID().equals(masterWalletId)) {
                mMasterWallets.remove(i);
                break;
            }
        }
        DestroyWallet(mInstance, masterWalletId);
    }

    public MasterWallet ImportWalletWithKeystore(
            String masterWalletId, String keystoreContent, String backupPassWord,
            String payPassWord) throws WalletException {

        long masterProxy = ImportWalletWithKeystore(mInstance, masterWalletId,
                keystoreContent, backupPassWord, payPassWord);

        if (masterProxy == 0) {
            Log.e(TAG, "Import master wallet with key store fail");
            return null;
        }
        MasterWallet masterWallet = new MasterWallet(masterProxy);
        mMasterWallets.add(masterWallet);

        return masterWallet;
    }

    public MasterWallet ImportWalletWithMnemonic(
            String masterWalletId, String mnemonic, String phrasePassword, String payPassWord,
            boolean singleAddress, long timestamp) throws WalletException {

        if (MasterWalletExist(masterWalletId)) {
            Log.e(TAG, "Master wallet [" + masterWalletId + "] exist");
            return null;
        }

        long masterProxy = ImportWalletWithMnemonic(mInstance, masterWalletId,
                mnemonic, phrasePassword, payPassWord, singleAddress, timestamp);

        if (masterProxy == 0) {
            Log.e(TAG, "Import master wallet with mnemonic fail");
            return null;
        }
        MasterWallet masterWallet = new MasterWallet(masterProxy);
        mMasterWallets.add(masterWallet);

        return masterWallet;
    }

    public MasterWallet ImportReadonlyWallet(String masterWalletID, String walletJson) throws WalletException {
        if (MasterWalletExist(masterWalletID)) {
            Log.e(TAG, "Master wallet [" + masterWalletID + "] exist");
            return null;
        }

        long masterProxy = ImportReadonlyWallet(mInstance, masterWalletID, walletJson);

        if (masterProxy == 0) {
            Log.e(TAG, "Import master wallet [" + masterWalletID + "] error");
            return null;
        }

        MasterWallet masterWallet = new MasterWallet(masterProxy);
        mMasterWallets.add(masterWallet);

        return masterWallet;
    }

    public String GenerateMnemonic(String language, int wordCount) throws WalletException {
        return GenerateMnemonic(mInstance, language, wordCount);
    }

    public MasterWallet CreateMultiSignMasterWallet(String masterWalletID, String coSigners,
                                                    int requiredSignCount, boolean singleAddress,
                                                    boolean compatible, long timestamp) throws WalletException {

        if (MasterWalletExist(masterWalletID)) {
            Log.e(TAG, "Master wallet [" + masterWalletID + "] exist");
            return null;
        }

        long masterProxy = CreateMultiSignMasterWallet(mInstance, masterWalletID,
                coSigners, requiredSignCount, singleAddress, compatible, timestamp);

        if (masterProxy == 0) {
            Log.e(TAG, "Create multi sign master wallet fail");
            return null;
        }

        MasterWallet masterWallet = new MasterWallet(masterProxy);
        mMasterWallets.add(masterWallet);

        return masterWallet;
    }

    public MasterWallet CreateMultiSignMasterWallet(
            String masterWalletID, String privKey, String payPassword, String coSigners,
            int requiredSignCount, boolean singleAddress, boolean compatible, long timestamp) throws WalletException {

        if (MasterWalletExist(masterWalletID)) {
            Log.e(TAG, "Master wallet [" + masterWalletID + "] exist");
            return null;
        }

        long masterProxy = CreateMultiSignMasterWalletWithPrivKey(mInstance, masterWalletID,
                privKey, payPassword, coSigners, requiredSignCount, singleAddress, compatible, timestamp);

        if (masterProxy == 0) {
            Log.e(TAG, "Create multi sign master wallet with private key fail");
            return null;
        }

        MasterWallet masterWallet = new MasterWallet(masterProxy);
        mMasterWallets.add(masterWallet);

        return masterWallet;
    }

    public MasterWallet CreateMultiSignMasterWallet(
            String masterWalletId, String mnemonic, String phrasePassword, String payPassword,
            String coSigners, int requiredSignCount, boolean singleAddress, boolean compatible,
            long timestamp) throws WalletException {


        if (MasterWalletExist(masterWalletId)) {
            Log.e(TAG, "Master wallet [" + masterWalletId + "] exist");
            return null;
        }

        long masterProxy = CreateMultiSignMasterWalletWithMnemonic(mInstance, masterWalletId,
                mnemonic, phrasePassword, payPassword, coSigners, requiredSignCount, singleAddress,
                compatible, timestamp);

        if (masterProxy == 0) {
            Log.e(TAG, "Create multi sign master wallet with mnemonic fail");
            return null;
        }

        MasterWallet masterWallet = new MasterWallet(masterProxy);
        mMasterWallets.add(masterWallet);

        return masterWallet;
    }

    public String GetVersion() {
        return GetVersion(mInstance);
    }

    public void FlushData() { FlushData(mInstance); }

    private native String GenerateMnemonic(long instance, String language, int wordCount);

    private native long CreateMasterWallet(
            long instance, String masterWalletId, String mnemonic, String phrasePassword,
            String payPassword, boolean singleAddress);

    private native long CreateMultiSignMasterWallet(long instance, String masterWalletId,
                                                    String coSigners, int requiredSignCount,
                                                    boolean singleAddress, boolean compatible,
                                                    long timestamp);

    private native long CreateMultiSignMasterWalletWithPrivKey(
            long instance, String masterWalletId, String privKey, String payPassword,
            String coSigners, int requiredSignCount, boolean singleAddress, boolean compatible, long timestamp);

    private native long CreateMultiSignMasterWalletWithMnemonic(
            long instance, String masterWalletId, String mnemonic, String phrasePassword,
            String payPassword, String coSigners, int requiredSignCount, boolean singleAddress,
            boolean compatible, long timestamp);

    private native long ImportWalletWithKeystore(
            long instance, String masterWalletId, String keystoreContent, String backupPassWord,
            String payPassWord);

    private native long ImportWalletWithMnemonic(
            long instance, String masterWalletId, String mnemonic, String phrasePassword,
            String payPassWord, boolean singleAddress, long timestamp);

    private native long ImportReadonlyWallet(long instance, String masterWalletID, String walletJson);

    private native String GetVersion(long instance);

    private native void DestroyWallet(long instance, String masterWalletId);

    private native void FlushData(long mInstance);

    private native long[] GetAllMasterWallets(long instance);

    private native String[] GetAllMasterWalletID(long instance);

    private native long GetMasterWallet(long instance, String masterWalletId);

    private native long InitMasterWalletManager(String rootPath, String netType, String config, String dataPath);

    private native void DisposeNative(long instance);
}
