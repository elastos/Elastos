// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

package org.elastos.wallet.core;

import android.util.Log;

import java.util.ArrayList;

public class MasterWalletManager {
    private String TAG = "MasterWalletManager";
    private long mInstance;
    private String mRootPath;

    public MasterWalletManager(String rootPath) {
        mRootPath = rootPath;
        mInstance = InitMasterWalletManager(mRootPath);
    }

    public void Dispose() {
        Log.i(TAG, "Dispose");
        DisposeNative(mInstance);
    }

    public MasterWallet CreateMasterWallet(
            String masterWalletId, String mnemonic, String phrasePassword, String payPassword,
            boolean singleAddress) throws WalletException {

        long instance = CreateMasterWallet(mInstance, masterWalletId, mnemonic,
                phrasePassword, payPassword, singleAddress);

        if (instance == 0) {
            Log.e(TAG, "Create master wallet fail");
            return null;
        }

        return new MasterWallet(instance);
    }

    public ArrayList<MasterWallet> GetAllMasterWallets() throws WalletException {

        ArrayList<MasterWallet> list = new ArrayList<MasterWallet>();
        long[] masterWalletProxies = GetAllMasterWallets(mInstance);

        for (int i = 0; i < masterWalletProxies.length; i++) {
            list.add(new MasterWallet(masterWalletProxies[i]));
        }

        return list;
    }

    public String[] GetAllMasterWalletIds() throws WalletException {
        return GetAllMasterWalletIds(mInstance);
    }

    public MasterWallet GetWallet(String masterWalletId) throws WalletException {
        long masterWalletProxy = GetWallet(mInstance, masterWalletId);

        if (masterWalletProxy == 0) {
            Log.e(TAG, "Get master wallet fail");
            return null;
        }

        return new MasterWallet(masterWalletProxy);
    }

    public void DestroyWallet(String masterWalletId) throws WalletException {
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

        return new MasterWallet(masterProxy);
    }

    public MasterWallet ImportWalletWithMnemonic(
            String masterWalletId, String mnemonic, String phrasePassword, String payPassWord,
            boolean singleAddress) throws WalletException {

        long masterProxy = ImportWalletWithMnemonic(mInstance, masterWalletId,
                mnemonic, phrasePassword, payPassWord, singleAddress);

        if (masterProxy == 0) {
            Log.e(TAG, "Import master wallet with mnemonic fail");
            return null;
        }

        return new MasterWallet(masterProxy);
    }

    public String ExportWalletWithKeystore(MasterWallet masterWallet, String backupPassWord,
                                           String payPassword) throws WalletException {

        return ExportWalletWithKeystore(mInstance, masterWallet, backupPassWord, payPassword);
    }

    public String ExportWalletWithMnemonic(MasterWallet masterWallet,
                                           String payPassWord) throws WalletException {

        return ExportWalletWithMnemonic(mInstance, masterWallet, payPassWord);
    }

    public String GenerateMnemonic(String language) throws WalletException {
        return GenerateMnemonic(mInstance, language);
    }

    public String GetMultiSignPubKeyWithMnemonic(String phrase, String phrasePassword)
            throws WalletException {

        return GetMultiSignPubKeyWithMnemonic(mInstance, phrase, phrasePassword);
    }

    public String GetMultiSignPubKeyWithPrivKey(String privKey) throws WalletException {
        return GetMultiSignPubKeyWithPrivKey(mInstance, privKey);
    }

    public void SaveConfigs() {
        SaveConfigs(mInstance);
    }

    public MasterWallet CreateMultiSignMasterWallet(String masterWallet, String coSigners,
                                                    int requiredSignCount) throws WalletException {

        long masterProxy = CreateMultiSignMasterWallet(mInstance, masterWallet,
                coSigners, requiredSignCount);

        if (masterProxy == 0) {
            Log.e(TAG, "Create multi sign master wallet fail");
            return null;
        }

        return new MasterWallet(masterProxy);
    }

    public MasterWallet CreateMultiSignMasterWallet(
            String masterWallet, String privKey, String payPassword, String coSigners,
            int requiredSignCount) throws WalletException {

        long masterProxy = CreateMultiSignMasterWalletWithPrivKey(mInstance, masterWallet,
                privKey, payPassword, coSigners, requiredSignCount);

        if (masterProxy == 0) {
            Log.e(TAG, "Create multi sign master wallet with private key fail");
            return null;
        }

        return new MasterWallet(masterProxy);
    }

    public MasterWallet CreateMultiSignMasterWallet(
            String masterWalletId, String mnemonic, String phrasePassword, String payPassword,
            String coSigners, int requiredSignCount) throws WalletException {

        long masterProxy = CreateMultiSignMasterWalletWithMnemonic(mInstance, masterWalletId,
                mnemonic, phrasePassword, payPassword, coSigners, requiredSignCount);

        if (masterProxy == 0) {
            Log.e(TAG, "Create multi sign master wallet with mnemonic fail");
            return null;
        }

        return new MasterWallet(masterProxy);
    }

    public String EncodeTransactionToString(String txJson) {
        return EncodeTransactionToString(mInstance, txJson);
    }

    public String DecodeTransactionFromString(String cipher) {
        return DecodeTransactionFromString(mInstance, cipher);
    }

    private native void SaveConfigs(long instance);

    private native String GenerateMnemonic(long instance, String language);

    private native String GetMultiSignPubKeyWithMnemonic(long instance, String phrase,
                                                         String phrasePassword);

    private native String GetMultiSignPubKeyWithPrivKey(long instance, String privKey);

    private native long CreateMasterWallet(
            long instance, String masterWalletId, String mnemonic, String phrasePassword,
            String payPassword, boolean singleAddress);

    private native long CreateMultiSignMasterWallet(long instance, String masterWalletId,
                                                    String coSigners, int requiredSignCount);

    private native long CreateMultiSignMasterWalletWithPrivKey(
            long instance, String masterWalletId, String privKey, String payPassword,
            String coSigners, int requiredSignCount);

    private native long CreateMultiSignMasterWalletWithMnemonic(
            long instance, String masterWalletId, String mnemonic, String phrasePassword,
            String payPassword, String coSigners, int requiredSignCount);

    private native long ImportWalletWithKeystore(
            long instance, String masterWalletId, String keystoreContent, String backupPassWord,
            String payPassWord);

    private native long ImportWalletWithMnemonic(
            long instance, String masterWalletId, String mnemonic, String phrasePassword,
            String payPassWord, boolean singleAddress);

    private native String ExportWalletWithKeystore(long instance, MasterWallet masterWallet,
                                                   String backupPassWord, String payPassword);

    private native String ExportWalletWithMnemonic(long instance, MasterWallet masterWallet,
                                                   String backupPassWord);

    private native void DestroyWallet(long instance, String masterWalletId);

    private native long[] GetAllMasterWallets(long instance);

    private native String[] GetAllMasterWalletIds(long instance);

    private native long GetWallet(long instance, String masterWalletId);

    private native String EncodeTransactionToString(long instance, String txJson);

    private native String DecodeTransactionFromString(long instance, String cipher);

    private native long InitMasterWalletManager(String rootPath);

    private native void DisposeNative(long instance);
}
