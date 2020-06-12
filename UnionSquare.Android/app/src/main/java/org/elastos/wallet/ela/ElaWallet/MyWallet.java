/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

package org.elastos.wallet.ela.ElaWallet;

import org.elastos.wallet.core.IDChainSubWallet;
import org.elastos.wallet.core.MainchainSubWallet;
import org.elastos.wallet.core.MasterWallet;
import org.elastos.wallet.core.MasterWalletManager;
import org.elastos.wallet.core.SidechainSubWallet;
import org.elastos.wallet.core.SubWallet;
import org.elastos.wallet.core.SubWalletCallback;
import org.elastos.wallet.core.WalletException;
import org.elastos.wallet.ela.MyApplication;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.CommonEntity;
import org.elastos.wallet.ela.ui.Assets.bean.BalanceEntity;
import org.elastos.wallet.ela.ui.Assets.listener.ISubWalletListener;
import org.elastos.wallet.ela.ui.common.bean.CommmonBooleanEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonLongEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonObjEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonObjectWithMethNameEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringWithiMethNameEntity;
import org.elastos.wallet.ela.ui.common.bean.CommonStringArrayEntity;
import org.elastos.wallet.ela.ui.common.bean.ISubWalletListEntity;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.klog.KLog;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;

/**
 * wallet  jni
 */
public class MyWallet {
    private static final String TAG = "Wallet";
    public static final String ELA = "ELA";
    public static final String IDChain = "IDChain";
    public static final String ETHSC = "ETHSC";
    public static final long RATE = 100000000;
    public static final String RATE_S = RATE + "";
    public static final long feePerKb = 10000L;
    public static String Url = getTxHashBaseUrl();
    public MasterWalletManager mMasterWalletManager;
    //private IDidManager mDidManager = null;
    private String mRootPath;
    public static final String SUCCESSCODE = "00000";
    private static final String WalletException = "000001";
    private static final String JSONEXCEPTION = "000002";
    public static final String errorCodeDoInMeathed = "000003";//没有此钱包 要在代码里处理


    private int errCodeInvalidMasterWallet = 10002;
    private int errCodeInvalidSubWallet = 10003;
    private int errCodeCreateMasterWallet = 10004;
    private int errCodeCreateSubWallet = 10005;
    private int errCodeImportFromKeyStore = 10008;
    private int errCodeImportFromMnemonic = 10009;
    private int errCodeSubWalletInstance = 10010;
    private int errCodeImportReadonlyWallet = 10011;

    private volatile static MyWallet instance = null;

    public static MyWallet getInstance() {
        if (instance == null) {
            synchronized (MyWallet.class) {
                if (instance == null) {
                    instance = new MyWallet();
                }
            }

        }
        return instance;
    }

    private MyWallet() {

        mRootPath = MyApplication.getAppContext().getFilesDir().getParent();
        String net = "MainNet";
        String config = "";
        switch (MyApplication.currentWalletNet) {
            case WalletNet.TESTNET:
                net = "TestNet";
                break;
            case WalletNet.REGTESTNET:
                net = "RegTest";
                break;
            case WalletNet.PRVNET:
                net = "PrvNet";
                config = WalletNet.PRICONFIG;
                break;

        }
        mMasterWalletManager = new MasterWalletManager(mRootPath, net, config, "");
    }

    private static String getTxHashBaseUrl() {
        String baseUrl = "https://blockchain.elastos.org/tx/";
        switch (MyApplication.currentWalletNet) {
            case WalletNet.TESTNET:
                baseUrl = "https://blockchain-beta.elastos.org/tx/";
                break;
            case WalletNet.REGTESTNET:
                baseUrl = "https://blockchain-regtest.elastos.org/tx/";
                break;

        }
        return baseUrl;
    }

    /**
     * Called when the system is about to start resuming a previous activity.
     *
     * @param multitasking Flag indicating if multitasking is turned on for app
     */

    public void onPause(boolean multitasking) {
        Log.d(TAG, "onPause");
        flushData();
    }

    /**
     * Called when the activity will start interacting with the user.
     *
     * @param multitasking Flag indicating if multitasking is turned on for app
     */

    public void onResume(boolean multitasking) {
        Log.d(TAG, "onResume");

    }

    /**
     * Called when the activity is becoming visible to the user.
     */

    public void onStart() {
        Log.d(TAG, "onStart");

    }

    /**
     * Called when the activity is no longer visible to the user.
     */

    public void onStop() {
        Log.d(TAG, "onStop");

    }

    /**
     * The final call you receive before your activity is destroyed.
     */

    public void onDestroy() {
        if (mMasterWalletManager != null) {
            ArrayList<MasterWallet> masterWalletList = mMasterWalletManager.GetLoadedMasterWallets();
            for (int i = 0; i < masterWalletList.size(); i++) {
                MasterWallet masterWallet = masterWalletList.get(i);
                ArrayList<SubWallet> subWallets = masterWallet.GetAllSubWallets();
                for (int j = 0; j < subWallets.size(); ++j) {
                    subWallets.get(j).RemoveCallback();
                }
            }

            mMasterWalletManager.Dispose();
            mMasterWalletManager = null;
            instance = null;
        }


    }


    private boolean createDIDManager(MasterWallet masterWallet) {
        try {
//			String masterWalletID = masterWallet.GetId();
//			JSONObject basicInfo = new JSONObject(masterWallet.GetBasicInfo());
//			String accountType = basicInfo.getJSONObject("Account").getString("Type");
//			if (! accountType.equals("Standard")) {
//				Log.w(TAG, "Master wallet '" + masterWalletID + "' is not standard account, can't create DID manager");
//				return false;
//			}
//
//			if (null != getDIDManager(masterWalletID)) {
//				Log.w(TAG, "Master wallet '" + masterWalletID + "' already contain DID manager");
//				return false;
//			}
//
//			Log.d(TAG, "Master wallet '" + masterWallet.GetId() + "' create DID manager with root path '" + mRootPath + "'");
//			IDidManager DIDManager = mDIDManagerSupervisor.CreateDIDManager(masterWallet, mRootPath);
//			putDIDManager(masterWalletID, DIDManager);
            return true;
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "DID manager initialize exception");
        }

        return false;
    }

    private String formatWalletName(String masterWalletID) {
        return masterWalletID;
    }

    private String formatWalletName(String masterWalletID, String chainID) {
        return masterWalletID + ":" + chainID;
    }


    private BaseEntity errorProcess(String code, String msg) {
        return new CommonEntity(code, msg);
    }


    private MasterWallet getMasterWallet(String masterWalletID) {
        if (mMasterWalletManager == null) {
            Log.e(TAG, "Master wallet manager has not initialize");
            return null;
        }

        return mMasterWalletManager.GetMasterWallet(masterWalletID);
    }

    private SubWallet getSubWallet(String masterWalletID, String chainID) {
        MasterWallet masterWallet = getMasterWallet(masterWalletID);
        if (masterWallet == null) {
            Log.e(TAG, formatWalletName(masterWalletID) + " not found");
            return null;
        }

        ArrayList<SubWallet> subWalletList = masterWallet.GetAllSubWallets();
        for (int i = 0; i < subWalletList.size(); i++) {
            if (chainID.equals(subWalletList.get(i).GetChainID())) {
                return subWalletList.get(i);
            }
        }

        Log.e(TAG, formatWalletName(masterWalletID, chainID) + " not found");
        return null;
    }

/*    private BaseEntity getSubWallet1(String masterWalletID, String chainID) {
        SubWallet subWallet = getSubWallet(masterWalletID, chainID);
        if (subWallet != null) {
            org.elastos.wallet.ela.db.table.SubWallet newSubWallet = new org.elastos.wallet.ela.db.table.SubWallet();
            newSubWallet.setBalance(subWallet.GetBalance() + "");
            newSubWallet.setBelongId(masterWalletID);
            newSubWallet.setChainId(subWallet.GetChainID());
            return new CommmonObjEntity(SUCCESSCODE, newSubWallet);
        }
        return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID));
    }*/

    private IDChainSubWallet getIDChainSubWallet(String masterWalletID) {
        SubWallet subWallet = getSubWallet(masterWalletID, IDChain);

        if ((subWallet instanceof IDChainSubWallet)) {
            return (IDChainSubWallet) subWallet;
        }
        return null;

    }

    private MainchainSubWallet getMainChainSubWallet(String masterWalletID) {
        SubWallet subWallet = getSubWallet(masterWalletID, ELA);

        if ((subWallet instanceof MainchainSubWallet)) {
            return (MainchainSubWallet) subWallet;
        }
        return null;

    }
    // args[0]: String masterWalletID
    // args[1]: String chainID

    public BaseEntity createSubWallet(String masterWalletID, String chainID) {
        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));
            }

            SubWallet subWallet = masterWallet.CreateSubWallet(chainID);
            if (subWallet == null) {
                return errorProcess(errCodeCreateSubWallet + "", "Create " + formatWalletName(masterWalletID, chainID));

            }
            return new CommmonStringEntity(SUCCESSCODE, subWallet.GetBasicInfo());
        } catch (WalletException e) {
            return exceptionProcess(e, "Create " + formatWalletName(masterWalletID, chainID));
        }

    }

    // args[0]: String masterWalletID
    public BaseEntity getMasterWalletBasicInfo(String masterWalletID) {
        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));
            }

            String basicInfo = masterWallet.GetBasicInfo();

            return new CommmonStringEntity(SUCCESSCODE, basicInfo);
        } catch (WalletException e) {
            return exceptionProcess(e, "Get " + formatWalletName(masterWalletID) + " basic info");
        }
    }

    public BaseEntity getAllMasterWallets() {
        try {
            ArrayList<MasterWallet> masterWalletList = mMasterWalletManager.GetAllMasterWallets();

            return new CommmonObjectWithMethNameEntity(SUCCESSCODE, masterWalletList, "getAllMasterWallets");
        } catch (WalletException e) {
            return exceptionProcess(e, "Get all master wallets");
        }

    }

    public BaseEntity getAllMasterWalletIds() {
        try {
            String[] masterWalletIds = mMasterWalletManager.GetAllMasterWalletID();

            return new CommmonObjectWithMethNameEntity(SUCCESSCODE, masterWalletIds, "getAllMasterWalletIds");
        } catch (WalletException e) {
            return exceptionProcess(e, "Get all master wallets");
        }

    }

    public BaseEntity getMasterWalletBaseEntity(String masterWalletID) {
        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);

            return new CommmonObjectWithMethNameEntity(SUCCESSCODE, masterWallet, "getMasterWalletBaseEntity");
        } catch (WalletException e) {
            return exceptionProcess(e, "Get  master wallets");
        }


    }

    private BaseEntity exceptionProcess(WalletException e, String msg) {
        e.printStackTrace();

        try {
            JSONObject exceptionJson = new JSONObject(e.GetErrorInfo());
            String exceptionCode = exceptionJson.getString("Code");
            String exceptionMsg = exceptionJson.getString("Message");
/*
            JSONObject errJson = new JSONObject();
            errJson.put(keyCode, exceptionCode);
            errJson.put(keyMessage, msg + ": " + exceptionMsg);
            if (exceptionJson.has("Data")) {
                errJson.put("Data", exceptionJson.getInt("Data"));
            }*/
            return new CommonEntity(exceptionCode, exceptionMsg);
        } catch (JSONException je) {
            return new CommonEntity(JSONEXCEPTION, msg + ":" + e.GetErrorInfo());
        }
    }

    // args[0]: String masterWalletID
    public BaseEntity getAllSubWallets(String masterWalletID) {
        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get->getAllSubWallets " + formatWalletName(masterWalletID));

            }
            ArrayList<SubWallet> subWalletList = masterWallet.GetAllSubWallets();
            ArrayList<org.elastos.wallet.ela.db.table.SubWallet> newSubWalletList = new ArrayList<>();
            for (SubWallet subWallet : subWalletList) {
                org.elastos.wallet.ela.db.table.SubWallet newSubWallet = new org.elastos.wallet.ela.db.table.SubWallet();
                newSubWallet.setBalance(subWallet.GetBalance() + "");
                newSubWallet.setBelongId(masterWalletID);
                newSubWallet.setChainId(subWallet.GetChainID());
                newSubWalletList.add(newSubWallet);
            }

            return new ISubWalletListEntity(SUCCESSCODE, newSubWalletList);
        } catch (WalletException e) {
            return exceptionProcess(e, "Get " + masterWalletID + " all subwallets");
        }
    }


    // args[0]: String language
    public BaseEntity generateMnemonic(String language) {
        String mnemonic = null;
        try {
            mnemonic = mMasterWalletManager.GenerateMnemonic(language, 12);
            return new CommmonStringEntity(SUCCESSCODE, mnemonic);
        } catch (WalletException e) {
            return exceptionProcess(e, "Generate mnemonic in '" + language + "'");

        }

    }

    // args[0]: String masterWalletID
    // args[1]: String address
    public BaseEntity isAddressValid(String masterWalletID, String addr) {
        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));

            }

            boolean valid = masterWallet.IsAddressValid(addr);
            return new CommmonBooleanEntity(SUCCESSCODE, valid);
        } catch (WalletException e) {
            return exceptionProcess(e, "Check address valid of " + formatWalletName(masterWalletID));
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String mnemonic
    // args[2]: String phrasePassword
    // args[3]: String payPassword
    // args[4]: boolean singleAddress
    public BaseEntity createMasterWallet(String masterWalletID, String mnemonic, String phrasePassword, String payPassword, boolean singleAddress) {
        String basicInfo = null;

        try {
            MasterWallet masterWallet = mMasterWalletManager.CreateMasterWallet(
                    masterWalletID, mnemonic, phrasePassword, payPassword, singleAddress);

            if (masterWallet == null) {
                return errorProcess(errCodeCreateMasterWallet + "", "Create " + formatWalletName(masterWalletID));
            }
            createDIDManager(masterWallet);

            basicInfo = masterWallet.GetBasicInfo();
            //  successProcess(masterWallet.GetBasicInfo());
            return new CommmonStringEntity(SUCCESSCODE, basicInfo);
        } catch (WalletException e) {
            return exceptionProcess(e, "Create " + formatWalletName(masterWalletID));
        }

    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    public BaseEntity destroySubWallet(String masterWalletID, String chainID) {
        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));

            }
            SubWallet subWallet = masterWallet.GetSubWallet(chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            masterWallet.DestroyWallet(subWallet);

            return new CommmonStringEntity(SUCCESSCODE, chainID);
            // successProcess(cc, "Destroy " + formatWalletName(masterWalletID, chainID) + " OK");
        } catch (WalletException e) {
            return exceptionProcess(e, "Destroy " + formatWalletName(masterWalletID, chainID));
        }
    }

    // args[0]: String masterWalletID
    public BaseEntity destroyWallet(String masterWalletID) {
        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));
            }
            ArrayList<SubWallet> subWallets = masterWallet.GetAllSubWallets();
            for (int i = 0; subWallets != null && i < subWallets.size(); i++) {
                subWallets.get(i).RemoveCallback();
            }

            mMasterWalletManager.DestroyWallet(masterWalletID);

            return new CommmonStringEntity(SUCCESSCODE, "sucesss");
        } catch (WalletException e) {
            return exceptionProcess(e, "Destroy " + formatWalletName(masterWalletID));
        }
    }


    // args[0]: String masterWalletID
    // args[1]: String keystoreContent
    // args[2]: String backupPassword
    // args[3]: String payPassword
    public BaseEntity importWalletWithKeystore(String masterWalletID, String keystoreContent, String backupPassword, String payPassword) {
        try {
            MasterWallet masterWallet = mMasterWalletManager.ImportWalletWithKeystore(
                    masterWalletID, keystoreContent, backupPassword, payPassword);
            if (masterWallet == null) {
                return errorProcess(errCodeImportFromKeyStore + "", "Import " + formatWalletName(masterWalletID) + " with keystore");

            }

            createDIDManager(masterWallet);

            return new CommmonStringEntity(SUCCESSCODE, masterWallet.GetBasicInfo());

        } catch (WalletException e) {
            return exceptionProcess(e, "Import " + formatWalletName(masterWalletID) + " with keystore");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String mnemonic
    // args[2]: String phrasePassword
    // args[3]: String payPassword
    // args[4]: boolean singleAddress
    public BaseEntity importWalletWithMnemonic(String masterWalletID, String mnemonic, String phrasePassword,
                                               String payPassword, boolean singleAddress) {
        try {
            MasterWallet masterWallet = mMasterWalletManager.ImportWalletWithMnemonic(
                    masterWalletID, mnemonic, phrasePassword, payPassword, singleAddress, 0);
            if (masterWallet == null) {
                return errorProcess(errCodeImportFromMnemonic + "", "Import " + formatWalletName(masterWalletID) + " with mnemonic");
            }

            createDIDManager(masterWallet);

            return new CommmonStringEntity(SUCCESSCODE, masterWallet.GetBasicInfo());

        } catch (WalletException e) {
            return exceptionProcess(e, "Import " + formatWalletName(masterWalletID) + " with mnemonic");
        }
    }

    public BaseEntity importReadonlyWallet(String masterWalletID, String walletJson) {

        try {
            MasterWallet masterWallet = mMasterWalletManager.ImportReadonlyWallet(masterWalletID, walletJson);

            if (masterWallet == null) {
                return errorProcess(errCodeImportReadonlyWallet + "", "Import read-only wallet" + formatWalletName(masterWalletID));
            }

            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, masterWallet.GetBasicInfo(), "importReadonlyWallet");
        } catch (WalletException e) {
            return exceptionProcess(e, "Import read-only wallet " + formatWalletName(masterWalletID));
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String backupPassword
    // args[2]: String payPassword
    public BaseEntity exportWalletWithKeystore(String masterWalletID, String backupPassword, String payPassword) {
        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));

            }

            String keystore = masterWallet.ExportKeystore(backupPassword, payPassword);

            return new CommmonStringEntity(SUCCESSCODE, keystore);
        } catch (WalletException e) {
            return exceptionProcess(e, "Export " + formatWalletName(masterWalletID) + "to keystore");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String payPassword
    public BaseEntity exportWalletWithMnemonic(String masterWalletID, String backupPassword) {
        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));
            }

            String mnemonic = masterWallet.ExportMnemonic(backupPassword);

            //  return new CommmonStringEntity(SUCCESSCODE, mnemonic);
            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, mnemonic, "exportWalletWithMnemonic");
        } catch (WalletException e) {
            return exceptionProcess(e, "Export " + masterWalletID + " to mnemonic");
        }
    }

    public BaseEntity exportReadonlyWallet(String masterWalletID) {

        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);

            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));
            }

            String walletJson = masterWallet.ExportReadonlyWallet();

            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, walletJson, "exportReadonlyWallet");
        } catch (WalletException e) {
            return exceptionProcess(e, "Export read-only wallet " + formatWalletName(masterWalletID));
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    public BaseEntity getBalance(String masterWalletID, String chainID) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID) + " balance");

            }

            return new CommmonObjEntity(SUCCESSCODE, new BalanceEntity(chainID, subWallet.GetBalance() + "", masterWalletID));

        } catch (WalletException e) {
            return exceptionProcess(e, "Get " + formatWalletName(masterWalletID, chainID) + " balance");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    public BaseEntity createAddress(String masterWalletID, String chainID) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            String address = subWallet.CreateAddress();
            return new CommmonStringEntity(SUCCESSCODE, address);
        } catch (WalletException e) {
            return exceptionProcess(e, "Create " + formatWalletName(masterWalletID, chainID) + " address");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: int start
    // args[3]: int count
    public BaseEntity getAllAddress(String masterWalletID, String chainID, int start, int count) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }
            String allAddresses = subWallet.GetAllAddress(start, count);

            return new CommmonStringEntity(SUCCESSCODE, allAddresses);
        } catch (WalletException e) {
            return exceptionProcess(e, "Get " + formatWalletName(masterWalletID, chainID) + " all addresses");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: int start
    // args[3]: int count
    public BaseEntity getAllPublicKeys(String masterWalletID, String chainID, int start, int count) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));
            }
            String allPublicKeys = subWallet.GetAllPublicKeys(start, count);

            return new CommmonStringEntity(SUCCESSCODE, allPublicKeys);
        } catch (WalletException e) {
            return exceptionProcess(e, "Get " + formatWalletName(masterWalletID, chainID) + " all addresses");
        }
    }


    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String fromAddress
    // args[3]: String toAddress
    // args[4]: long amount
    // args[5]: String memo
    // args[6]: Boolean useVotedUTXO
    public BaseEntity getFee(String masterWalletID, String chainID, String fromAddress,
                             String toAddress, String amount) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));
            }

            String tx = subWallet.CreateTransaction(fromAddress, toAddress, amount, "");
            long fee = MyWallet.feePerKb;
            try {
                JSONObject jsonObject = new JSONObject(tx);

                if (jsonObject.has("Fee")) {
                    fee = jsonObject.getLong("Fee");
                }

            } catch (JSONException e) {
                e.printStackTrace();
            }
            return new CommmonLongEntity(SUCCESSCODE, fee);
        } catch (WalletException e) {
            return exceptionProcess(e, "Create " + formatWalletName(masterWalletID, chainID) + " tx");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String fromAddress
    // args[3]: String toAddress
    // args[4]: long amount
    // args[5]: String memo
    // args[6]: Boolean useVotedUTXO
    public BaseEntity createTransaction(String masterWalletID, String chainID, String fromAddress,
                                        String toAddress, String amount, String memo, boolean useVotedUTXO) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));
            }

            String tx = subWallet.CreateTransaction(fromAddress, toAddress, amount, memo);

            return new CommmonStringEntity(SUCCESSCODE, tx);
        } catch (WalletException e) {
            return exceptionProcess(e, "Create " + formatWalletName(masterWalletID, chainID) + " tx");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String rawTransaction
    // args[3]: String payPassword
    // return:  String txJson
    public BaseEntity signTransaction(String masterWalletID, String chainID, String rawTransaction, String payPassword) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            String result = subWallet.SignTransaction(rawTransaction, payPassword);
            return new CommmonStringEntity(SUCCESSCODE, result, "signTransaction");
        } catch (WalletException e) {
            return exceptionProcess(e, "Sign " + formatWalletName(masterWalletID, chainID) + " tx");
        }
    }

    public BaseEntity signDigest(String masterWalletID, String cid, String digest, String payPassword) {
        try {
            IDChainSubWallet idChainSubWallet = getIDChainSubWallet(masterWalletID);
            if (idChainSubWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID));
            }
            String result = idChainSubWallet.SignDigest(cid, digest, payPassword);
            return new CommmonStringEntity(SUCCESSCODE, result);
        } catch (WalletException e) {
            return exceptionProcess(e, "signDigest " + formatWalletName(masterWalletID) + " tx");
        }
    }


    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String rawTxJson
    // return:  String resultJson
    public BaseEntity publishTransaction(String masterWalletID, String chainID, String rawTxJson) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            String resultJson = subWallet.PublishTransaction(rawTxJson);
            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, resultJson, "publishTransaction");
        } catch (WalletException e) {
            return exceptionProcess(e, "Publish " + formatWalletName(masterWalletID, chainID) + " tx");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: int start
    // args[3]: int count
    // args[4]: String addressOrTxId
    // return:  String txJson
    public BaseEntity getAllTransaction(String masterWalletID, String chainID, int start, int count, String addressOrTxId) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            String txJson = subWallet.GetAllTransaction(start, count, addressOrTxId);
            return new CommmonStringEntity(SUCCESSCODE, txJson);
        } catch (WalletException e) {
            return exceptionProcess(e, "Get " + formatWalletName(masterWalletID, chainID) + " all tx");
        }
    }


    // args[0]: String masterWalletID
    // args[1]: String chainID
    public BaseEntity registerWalletListener(String masterWalletID, String chainID, ISubWalletListener listener) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));
            }

            if (subWallet.getCallback() != null) {
                Log.d(TAG, "listener already registered, do nothing");
                subWallet.getCallback().setListener(listener);
                return new CommmonStringEntity(SUCCESSCODE, "listener already registered");
            }

            subWallet.AddCallback(new SubWalletCallback(masterWalletID, chainID, listener));
            return new CommmonStringEntity(SUCCESSCODE, "registerWalletListener");
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " add callback");
        }
    }


    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String fromAddress
    // args[3]: String sideChainID
    // args[4]: long amount
    // args[5]: String sideChainAddress
    // args[6]: String memo
    // args[7]: boolean useVotedUTXO
    public BaseEntity createDepositTransaction(String masterWalletID, String chainID,
                                               String fromAddress, String sideChainID,
                                               String amount, String sideChainAddress, String memo) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess(errCodeSubWalletInstance + "", formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String txJson = mainchainSubWallet.CreateDepositTransaction(fromAddress, sideChainID, amount, sideChainAddress, memo);

            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, txJson, "createDepositTransaction");
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " create deposit tx");
        }
    }


    // args[0]: String masterWalletID
    public BaseEntity getSupportedChains(String masterWalletID) {
        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));

            }

            String[] supportedChains = masterWallet.GetSupportedChains();
            //successProcess(cc, supportedChainsJson);
            return new CommonStringArrayEntity(SUCCESSCODE, supportedChains);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID) + " get support chain");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String oldPassword
    // args[2]: String newPassword
    public BaseEntity changePassword(String masterWalletID, String oldPassword, String newPassword) {
        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));

            }

            masterWallet.ChangePassword(oldPassword, newPassword);
            return new CommmonStringEntity(SUCCESSCODE, "sucesss");
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID) + " change password");
        }
    }

    // SidechainSubWallet

    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String fromAddress
    // args[3]: long amount
    // args[4]: String mainchainAdress
    // args[5]: String memo
    public BaseEntity createWithdrawTransaction(String masterWalletID, String chainID, String fromAddress,
                                                String amount, String mainchainAddress, String memo) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof SidechainSubWallet)) {
                return errorProcess(errCodeSubWalletInstance + "", formatWalletName(masterWalletID, chainID) + " is not instance of SidechainSubWallet");

            }

            SidechainSubWallet sidechainSubWallet = (SidechainSubWallet) subWallet;
            String tx = sidechainSubWallet.CreateWithdrawTransaction(fromAddress, amount, mainchainAddress, memo);

            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, tx, "createWithdrawTransaction");
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " create withdraw tx");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    public BaseEntity getGenesisAddress(String masterWalletID, String chainID) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof SidechainSubWallet)) {
                return errorProcess(errCodeSubWalletInstance + "", formatWalletName(masterWalletID, chainID) + " is not instance of SidechainSubWallet");

            }

            SidechainSubWallet sidechainSubWallet = (SidechainSubWallet) subWallet;

            String address = sidechainSubWallet.GetGenesisAddress();

            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, address, "getGenesisAddress");
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " get genesis address");
        }
    }


    public BaseEntity getPubKeyInfo(String masterWalletID) {
        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));

            }

            String pubKeyInfo = masterWallet.GetPubKeyInfo();
            return new CommmonStringEntity(SUCCESSCODE, pubKeyInfo);
        } catch (WalletException e) {
            return exceptionProcess(e, "getPubKeyInfo " + formatWalletName(masterWalletID));
        }
    }

    public BaseEntity verifyPrivateKey(String masterWalletID, String mnemonic, String passphrase) {
        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));

            }

            boolean result = masterWallet.VerifyPrivateKey(mnemonic, passphrase);
            return new CommmonBooleanEntity(SUCCESSCODE, result);
        } catch (WalletException e) {
            return exceptionProcess(e, "verifyPrivateKey " + formatWalletName(masterWalletID));
        }
    }

    public BaseEntity verifyPayPassword(String masterWalletID, String payPasswd) {
        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));

            }

            boolean result = masterWallet.VerifyPayPassword(payPasswd);
            return new CommmonBooleanEntity(SUCCESSCODE, result);
        } catch (WalletException e) {
            return exceptionProcess(e, "verifyPayPassword " + formatWalletName(masterWalletID));
        }
    }

    public BaseEntity verifyPassPhrase(String masterWalletID, String passphrase, String payPasswd) {
        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));

            }

            boolean result = masterWallet.VerifyPassPhrase(passphrase, payPasswd);
            return new CommmonBooleanEntity(SUCCESSCODE, result);
        } catch (WalletException e) {
            return exceptionProcess(e, "verifyPassPhrase " + formatWalletName(masterWalletID));
        }
    }

    /*****************************************投票相关**************************************/
    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String publicKey
    // args[3]: String nodePublicKey
    // args[4]: String nickName
    // args[5]: String url
    // args[6]: String IPAddress
    // args[7]: long location
    // args[8]: String payPasswd
    public BaseEntity generateProducerPayload(String masterWalletID, String chainID, String ownerPublicKey, String nodePublicKey, String nickName, String url, String IPAddress, long location, String payPasswd) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess("" + errCodeInvalidSubWallet, "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess(errCodeSubWalletInstance + "", formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String payloadJson = mainchainSubWallet.GenerateProducerPayload(ownerPublicKey, nodePublicKey, nickName, url, IPAddress, location, payPasswd);

            KLog.a(payloadJson);
            return new CommmonStringEntity(SUCCESSCODE, payloadJson);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " generate producer payload");
        }
    }


    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String fromAddress
    // args[3]: String payloadJson
    // args[4]: long amount
    // args[5]: String memo
    // args[6]: boolean useVotedUTXO
    public BaseEntity createRegisterProducerTransaction(String masterWalletID, String chainID, String fromAddress, String payloadJson, String amount, String memo, boolean useVotedUTXO) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess("" + errCodeInvalidSubWallet, "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String txJson = mainchainSubWallet.CreateRegisterProducerTransaction(fromAddress, payloadJson, amount, memo);

            KLog.a(txJson);
            return new CommmonStringEntity(SUCCESSCODE, txJson);
        } catch (WalletException e) {
            //  return new CommmonStringWithiMethNameEntity("", "", "");
            return exceptionProcess(e, formatWalletName(masterWalletID) + " change password");
        }

    }


    // args[0]: String masterWalletID
    // args[1]: String chainID
    public BaseEntity getPublicKeyForVote(String masterWalletID, String chainID) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String publicKey = mainchainSubWallet.GetOwnerPublicKey();

            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, publicKey, "getPublicKeyForVote");
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " get public key for vote");
        }
    }


    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String fromAddress
    // args[3]: long   stake
    // args[4]: String publicKeys JSONArray
    // args[5]: String memo
    // args[6]: String invalidCandidates
    public BaseEntity createVoteProducerTransaction(String masterWalletID, String chainID, String fromAddress, String stake, String publicKeys, String memo, String invalidCandidates) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess("" + errCodeInvalidSubWallet, "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String txJson = mainchainSubWallet.CreateVoteProducerTransaction(fromAddress, stake, publicKeys, memo, invalidCandidates);

            return new CommmonStringEntity(SUCCESSCODE, txJson);
            // successProcess(cc, txJson);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " create vote producer tx");
        }
        // return new CommmonStringWithiMethNameEntity(SUCCESSCODE, "", "createVoteProducerTransaction");
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID (only main chain ID 'ELA')
    public BaseEntity getVotedProducerList(String masterWalletID, String chainID) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess("" + errCodeInvalidSubWallet, "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }
            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String list = mainchainSubWallet.GetVotedProducerList();

            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, list, "getVotedProducerList");

        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " get voted producer list");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID (only main chain ID 'ELA')
    public BaseEntity getRegisteredProducerInfo(String masterWalletID, String chainID) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;
            String info = mainchainSubWallet.GetRegisteredProducerInfo();
            KLog.a(info);
            return new CommmonStringEntity(SUCCESSCODE, info);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " get registerd producer info");
        }
    }


    //投票注销 验证交易交易密码
    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String publicKey
    // args[3]: String payPasswd
    public BaseEntity generateCancelProducerPayload(String masterWalletID, String chainID, String publicKey, String payPasswd) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess("" + errCodeInvalidSubWallet, "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String payloadJson = mainchainSubWallet.GenerateCancelProducerPayload(publicKey, payPasswd);
            return new CommmonStringEntity(SUCCESSCODE, payloadJson);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " generate cancel producer payload");
        }
    }

    //创建注销交易
    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String fromAddress
    // args[3]: String payloadJson
    // args[4]: String memo
    // args[5]: boolean useVotedUTXO
    public BaseEntity createCancelProducerTransaction(String masterWalletID, String chainID, String fromAddress, String payloadJson) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess("" + errCodeInvalidSubWallet, "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String txJson = mainchainSubWallet.CreateCancelProducerTransaction(fromAddress, payloadJson, "");

            return new CommmonStringEntity(SUCCESSCODE, txJson);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " create cancel producer tx");
        }
    }

    //取回押金
    public BaseEntity createRetrieveDepositTransaction(String masterWalletID, String chainID, String amount) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess("" + errCodeInvalidSubWallet, "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");
            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String txJson = mainchainSubWallet.CreateRetrieveDepositTransaction(amount, "");

            return new CommmonStringEntity(SUCCESSCODE, txJson);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " create retrieve deposit tx");
        }
    }


    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String fromAddress
    // args[3]: String payloadJson
    // args[4]: String memo
    // args[5]: boolean useVotedUTXO
    public BaseEntity createUpdateProducerTransaction(String masterWalletID, String chainID, String fromAddress, String payloadJson, String memo, boolean useVotedUTXO) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess("" + errCodeInvalidSubWallet, "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String txJson = mainchainSubWallet.CreateUpdateProducerTransaction(fromAddress, payloadJson, memo);

            return new CommmonStringEntity(SUCCESSCODE, txJson);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " create update producer tx");
        }
    }

    /***************************************** CR投票相关 **************************************/
    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String crPublickey
    // args[3]: String did
    // args[4]: String nickName
    // args[5]: String url
    // args[6]: String location
    // args[7]: String payPasswd
    public BaseEntity generateCRInfoPayload(String masterWalletID, String chainID, String crPublickey, String nickName, String url, long location, String did) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess("" + errCodeInvalidSubWallet, "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess(errCodeSubWalletInstance + "", formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;
            String payloadJson = mainchainSubWallet.GenerateCRInfoPayload(crPublickey, did, nickName, url, location);

            KLog.a(payloadJson);
            return new CommmonStringEntity(SUCCESSCODE, payloadJson);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " generate crInfo payload");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String cID
    // args[3]: String payPasswd
    public BaseEntity generateUnregisterCRPayload(String masterWalletID, String chainID, String cid) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess("" + errCodeInvalidSubWallet, "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess(errCodeSubWalletInstance + "", formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String payloadJson = mainchainSubWallet.GenerateUnregisterCRPayload(cid);
            KLog.a(payloadJson);
            return new CommmonStringEntity(SUCCESSCODE, payloadJson);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " generate unregister payload");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    public BaseEntity getCROwnerDID(String masterWalletID, String chainID) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof IDChainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of IDChainSubWallet");

            }

            IDChainSubWallet idChainSubWallet = (IDChainSubWallet) subWallet;

            String did = idChainSubWallet.GetAllDID(0, 1);

            return new CommmonStringEntity(SUCCESSCODE, did);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " get cr owner did");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    public BaseEntity getCROwnerPublicKey(String masterWalletID, String chainID) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof IDChainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            IDChainSubWallet idChainSubWallet = (IDChainSubWallet) subWallet;

            String publicKeys = idChainSubWallet.GetAllPublicKeys(0, 1);

            return new CommmonStringEntity(SUCCESSCODE, publicKeys);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " get cr owner publickey");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String fromAddress
    // args[3]: String payload
    // args[4]: String amount
    // args[5]: String memo
    // args[6]: boolean useVotedUTXO
    public BaseEntity createRegisterCRTransaction(String masterWalletID, String chainID, String fromAddress, String payload, String amount, String memo, boolean useVotedUTXO) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String tx = mainchainSubWallet.CreateRegisterCRTransaction(fromAddress, payload, amount, memo);

            return new CommmonStringEntity(SUCCESSCODE, tx);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + "create register cr tx");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String fromAddress
    // args[3]: String payload
    // args[4]: String memo
    // args[5]: boolean useVotedUTXO
    public BaseEntity createUpdateCRTransaction(String masterWalletID, String chainID, String fromAddress, String payload, String memo, boolean useVotedUTXO) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String tx = mainchainSubWallet.CreateUpdateCRTransaction(fromAddress, payload, memo);

            return new CommmonStringEntity(SUCCESSCODE, tx);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + "create update cr tx");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String fromAddress
    // args[3]: String payload
    // args[4]: String memo
    // args[5]: boolean useVotedUTXO
    public BaseEntity createUnregisterCRTransaction(String masterWalletID, String chainID, String fromAddress, String payload) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String tx = mainchainSubWallet.CreateUnregisterCRTransaction(fromAddress, payload, "");

            return new CommmonStringEntity(SUCCESSCODE, tx);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + "create unregister cr tx");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String amount
    // args[3]: String memo
    public BaseEntity createRetrieveCRDepositTransaction(String masterWalletID, String chainID, String crPublickey, String amount, String memo) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String tx = mainchainSubWallet.CreateRetrieveCRDepositTransaction(crPublickey, amount, memo);

            return new CommmonStringEntity(SUCCESSCODE, tx);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + "create retrieve cr tx");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String fromAddress
    // args[3]: String votes
    // args[4]: String memo
    // args[5]: String invalidCandidates
    public BaseEntity createVoteCRTransaction(String masterWalletID, String chainID, String fromAddress, String votes, String memo, String invalidCandidates) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String tx = mainchainSubWallet.CreateVoteCRTransaction(fromAddress, votes, memo, invalidCandidates);

            return new CommmonStringEntity(SUCCESSCODE, tx);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + "create vote cr tx");
        }
    }

    public BaseEntity createImpeachmentCRCTransaction(String masterWalletID, String chainID, String fromAddress, String votes, String memo, String invalidCandidates) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }
            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;
            String tx = mainchainSubWallet.CreateImpeachmentCRCTransaction(fromAddress, votes, memo, invalidCandidates);

            return new CommmonStringEntity(SUCCESSCODE, tx);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + "create impeachment crc tx");
        }
    }

    public BaseEntity getVoteInfo(String masterWalletID, String chainID, String type) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }
            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;
            String info = mainchainSubWallet.GetVoteInfo(type);

            return new CommmonStringEntity(SUCCESSCODE, info);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + "get vote info");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID (only main chain ID 'ELA')
    public BaseEntity getVotedCRList(String masterWalletID, String chainID) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess("" + errCodeInvalidSubWallet, "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }
            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String list = mainchainSubWallet.GetVotedCRList();

            return new CommmonStringEntity(SUCCESSCODE, list);

        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " get voted cr list");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID (only main chain ID 'ELA')
    public BaseEntity getRegisteredCRInfo(String masterWalletID, String chainID) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;
            String info = mainchainSubWallet.GetRegisteredCRInfo();
            return new CommmonStringEntity(SUCCESSCODE, info);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " get registerd cr info");
        }
    }

    /********************************************多签改动版本*******************************************/
    public BaseEntity getOwnerAddress(String masterWalletID, String chainID) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess(errCodeSubWalletInstance + "", formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String address = mainchainSubWallet.GetOwnerAddress();
            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, address, "getOwnerAddress");
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + "getOwnerAddress");
        }
    }

    public BaseEntity getAllCoinBaseTransaction(String masterWalletID, String chainID, int start, int count, String addressOrTxId) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            String txJson = subWallet.GetAllCoinBaseTransaction(start, count, addressOrTxId);
            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, txJson, "getAllCoinBaseTransaction");
        } catch (WalletException e) {
            return exceptionProcess(e, "Get " + formatWalletName(masterWalletID, chainID) + " all tx");
        }
    }

    public BaseEntity createCombineUTXOTransaction(String masterWalletID, String chainID, String memo) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            String result = subWallet.CreateConsolidateTransaction(memo);
            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, result, "createCombineUTXOTransaction");
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + "createCombineUTXOTransaction");
        }
    }

    public BaseEntity getAllUTXOs(String masterWalletID, String chainID, int start, int count, String address) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            String utxoJson = subWallet.GetAllUTXOs(start, count, address);
            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, utxoJson, "getAllUTXOs");
        } catch (WalletException e) {
            return exceptionProcess(e, "Get " + formatWalletName(masterWalletID, chainID) + " all UTXOs");
        }
    }

    public BaseEntity getVersion() {
        try {
            String version = mMasterWalletManager.GetVersion();
            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, version, "getVersion");
        } catch (WalletException e) {
            return exceptionProcess(e, "Get version");
        }
    }

    // args[0]: String masterWalletID
    public BaseEntity flushData() {
        try {
            mMasterWalletManager.FlushData();
            return new CommmonStringEntity(SUCCESSCODE, "sucesss");
        } catch (WalletException e) {
            return exceptionProcess(e, "flushData");
        }
    }

    public BaseEntity exportxPrivateKey(String masterWalletID, String payPasswd) {
        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));
            }
            String privateKey = masterWallet.ExportPrivateKey(payPasswd);
            return new CommmonStringEntity(SUCCESSCODE, privateKey);
        } catch (WalletException e) {
            return exceptionProcess(e, "ExportxPrivateKey" + formatWalletName(masterWalletID));
        }
    }

    public BaseEntity createMultiSignMasterWallet(String masterWalletID, String coSigners, int requiredSignCount,
                                                  boolean singleAddress, boolean compatible, long timestamp) {
        try {
            MasterWallet masterWallet = mMasterWalletManager.CreateMultiSignMasterWallet(masterWalletID, coSigners,
                    requiredSignCount, singleAddress, compatible, timestamp);
            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, masterWallet.GetBasicInfo(), "createMultiSignMasterWallet");
        } catch (WalletException e) {
            return exceptionProcess(e, "createMultiSignMasterWallet" + formatWalletName(masterWalletID));
        }
    }

    public BaseEntity createMultiSignMasterWallet(String masterWalletID, String privKey,
                                                  String payPassword, String coSigners,
                                                  int requiredSignCount, boolean singleAddress,
                                                  boolean compatible, long timestamp) {
        try {
            MasterWallet masterWallet = mMasterWalletManager.CreateMultiSignMasterWallet(
                    masterWalletID, privKey, payPassword, coSigners,
                    requiredSignCount, singleAddress, compatible, timestamp);
            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, masterWallet.GetBasicInfo(), "createMultiSignMasterWallet");
        } catch (WalletException e) {
            return exceptionProcess(e, "createMultiSignMasterWallet" + formatWalletName(masterWalletID));
        }
    }

    public BaseEntity createMultiSignMasterWallet(
            String masterWalletId, String mnemonic, String phrasePassword, String payPassword,
            String coSigners, int requiredSignCount, boolean singleAddress, boolean compatible, long timestamp) {
        try {
            MasterWallet masterWallet = mMasterWalletManager.CreateMultiSignMasterWallet(
                    masterWalletId, mnemonic, phrasePassword, payPassword,
                    coSigners, requiredSignCount, singleAddress, compatible, timestamp);
            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, masterWallet.GetBasicInfo(), "createMultiSignMasterWallet");
        } catch (WalletException e) {
            return exceptionProcess(e, "createMultiSignMasterWallet" + formatWalletName(masterWalletId));
        }
    }

    public BaseEntity syncStart(String masterWalletID, String chainID) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            subWallet.SyncStart();
            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, "", "syncStart");
        } catch (WalletException e) {
            return exceptionProcess(e, "syncStart " + formatWalletName(masterWalletID, chainID));
        }
    }

    public BaseEntity syncStop(String masterWalletID, String chainID) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            subWallet.SyncStop();
            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, "", "syncStop");
        } catch (WalletException e) {
            return exceptionProcess(e, "syncStop " + formatWalletName(masterWalletID, chainID));
        }
    }

    public BaseEntity setFixedPeer(String masterWalletID, String chainID, String address, int port) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }
            return new CommmonBooleanEntity(SUCCESSCODE, subWallet.SetFixedPeer(address, port));
        } catch (WalletException e) {
            return exceptionProcess(e, "setFixedPeer " + formatWalletName(masterWalletID, chainID));
        }
    }

    public BaseEntity resync(String masterWalletID, String chainID) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }
            subWallet.Resync();
            return new CommmonStringEntity(SUCCESSCODE, "");
        } catch (WalletException e) {
            return exceptionProcess(e, "resync " + formatWalletName(masterWalletID, chainID));
        }
    }

    public BaseEntity getTransactionSignedInfo(String masterWalletID, String chainID, String rawTransaction) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            String result = subWallet.GetTransactionSignedInfo(rawTransaction);
            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, result, "getTransactionSignedInfo");
        } catch (WalletException e) {
            return exceptionProcess(e, "getTransactionSignedInfo " + formatWalletName(masterWalletID, chainID));
        }
    }

    /***********************************************************DID**************************************************************/


    public BaseEntity getCIDByPublicKey(String masterWalletID, String publicKey) {
        try {
            IDChainSubWallet idChainSubWallet = getIDChainSubWallet(masterWalletID);
            if (idChainSubWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID));
            }
            String cid = idChainSubWallet.GetPublicKeyCID(publicKey);
            return new CommmonStringEntity(SUCCESSCODE, cid);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, IDChain) + "GetDIDByPublicKey");
        }
    }

    public BaseEntity getDIDByPublicKey(String masterWalletID, String publicKey) {
        try {
            IDChainSubWallet idChainSubWallet = getIDChainSubWallet(masterWalletID);
            if (idChainSubWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID));
            }
            String did = idChainSubWallet.GetPublicKeyDID(publicKey);
            return new CommmonStringEntity(SUCCESSCODE, did);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, IDChain) + "GetDIDByPublicKey");
        }
    }

    public BaseEntity generateDIDInfoPayload(String masterWalletID, String inputJson, String passwd) {
        return errorProcess("99999", "function removed");
    }

    public BaseEntity createIDTransaction(String masterWalletID, String payloadJson) {
        try {
            IDChainSubWallet idChainSubWallet = getIDChainSubWallet(masterWalletID);
            if (idChainSubWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID));
            }
            String did = idChainSubWallet.CreateIDTransaction(payloadJson, "");
            return new CommmonStringEntity(SUCCESSCODE, did);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, IDChain) + "CreateIDTransaction");
        }
    }

    public BaseEntity getResolveDIDInfo(String masterWalletID, int start, int count, String did) {
        return errorProcess("88888", "function removed");
    }

    /***********************************************************Proposal**************************************************************/

    // args[0]: String masterWalletID
    // args[1]: String chainID (only main chain ID 'ELA')
    public BaseEntity proposalOwnerDigest(String masterWalletID, String payload) {
        try {
            MainchainSubWallet subWallet = getMainChainSubWallet(masterWalletID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID));

            }
            String info = subWallet.ProposalOwnerDigest(payload);
            return new CommmonStringEntity(SUCCESSCODE, info);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID) + "proposalOwnerDigest");
        }
    }

    public BaseEntity proposalCRCouncilMemberDigest(String masterWalletID, String payload) {
        try {
            MainchainSubWallet subWallet = getMainChainSubWallet(masterWalletID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID));

            }
            String info = subWallet.ProposalCRCouncilMemberDigest(payload);
            return new CommmonStringEntity(SUCCESSCODE, info);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID) + "ProposalCRCouncilMemberDigest");
        }
    }

    public BaseEntity createProposalTransaction(String masterWalletID, String payload) {
        try {
            MainchainSubWallet subWallet = getMainChainSubWallet(masterWalletID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID));

            }
            String info = subWallet.CreateProposalTransaction(payload, "");
            return new CommmonStringEntity(SUCCESSCODE, info);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID) + "CreateProposalTransaction");
        }
    }

    public BaseEntity calculateProposalHash(String masterWalletID, String payload) {
        try {
            MainchainSubWallet subWallet = getMainChainSubWallet(masterWalletID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID));

            }
            String info = subWallet.CalculateProposalHash(payload);
            return new CommmonStringEntity(SUCCESSCODE, info);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID) + "calculateProposalHash");
        }
    }

    public BaseEntity proposalReviewDigest(String masterWalletID, String payload) {
        try {
            MainchainSubWallet subWallet = getMainChainSubWallet(masterWalletID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID));

            }
            String info = subWallet.ProposalReviewDigest(payload);
            return new CommmonStringEntity(SUCCESSCODE, info);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID) + "proposalReviewDigest");
        }
    }

    public BaseEntity createProposalReviewTransaction(String masterWalletID, String payload) {
        try {
            MainchainSubWallet subWallet = getMainChainSubWallet(masterWalletID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID));

            }
            String info = subWallet.CreateProposalReviewTransaction(payload, "");
            return new CommmonStringEntity(SUCCESSCODE, info);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID) + "createProposalReviewTransaction");
        }
    }

    public BaseEntity createVoteCRCProposalTransaction(String masterWalletID, String votes, String invalidCandidates) {
        try {
            MainchainSubWallet subWallet = getMainChainSubWallet(masterWalletID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID));

            }
            String info = subWallet.CreateVoteCRCProposalTransaction("", votes, "", invalidCandidates);
            return new CommmonStringEntity(SUCCESSCODE, info);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID) + "createVoteCRCProposalTransaction");
        }
    }


    public BaseEntity proposalTrackingOwnerDigest(String masterWalletID, String payload) {
        try {
            MainchainSubWallet subWallet = getMainChainSubWallet(masterWalletID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID));

            }
            String info = subWallet.ProposalTrackingOwnerDigest(payload);
            return new CommmonStringEntity(SUCCESSCODE, info);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID) + "ProposalTrackingOwnerDigest");
        }
    }

    public BaseEntity proposalTrackingNewOwnerDigest(String masterWalletID, String payload) {
        try {
            MainchainSubWallet subWallet = getMainChainSubWallet(masterWalletID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID));

            }
            String info = subWallet.ProposalTrackingNewOwnerDigest(payload);
            return new CommmonStringEntity(SUCCESSCODE, info);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID) + "ProposalTrackingNewOwnerDigest");
        }
    }

    public BaseEntity proposalTrackingSecretaryDigest(String masterWalletID, String payload) {
        try {
            MainchainSubWallet subWallet = getMainChainSubWallet(masterWalletID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID));

            }
            String info = subWallet.ProposalTrackingSecretaryDigest(payload);
            return new CommmonStringEntity(SUCCESSCODE, info);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID) + "ProposalTrackingSecretaryDigest");
        }
    }

    public BaseEntity createProposalTrackingTransaction(String masterWalletID, String payload) {
        try {
            MainchainSubWallet subWallet = getMainChainSubWallet(masterWalletID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID));

            }
            String info = subWallet.CreateProposalTrackingTransaction(payload, "");
            return new CommmonStringEntity(SUCCESSCODE, info);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID) + "CreateProposalTrackingTransaction");
        }
    }

    public BaseEntity getVoteInfo(String masterWalletID, String type) {
        try {
            MainchainSubWallet subWallet = getMainChainSubWallet(masterWalletID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID));

            }
            String info = subWallet.GetVoteInfo(type);
            return new CommmonStringEntity(SUCCESSCODE, info);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID) + "ProposalTrackingSecretaryDigest");
        }
    }

    public BaseEntity proposalWithdrawDigest(String masterWalletID, String payload) {
        try {
            MainchainSubWallet subWallet = getMainChainSubWallet(masterWalletID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID));

            }
            String info = subWallet.ProposalWithdrawDigest(payload);
            return new CommmonStringEntity(SUCCESSCODE, info);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID) + "ProposalWithdrawDigest");
        }
    }

    public BaseEntity createProposalWithdrawTransaction(String masterWalletID, String recipient, String amount, String utxo, String payload) {
        try {
            MainchainSubWallet subWallet = getMainChainSubWallet(masterWalletID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID));

            }
            String info = subWallet.CreateProposalWithdrawTransaction(recipient, amount, utxo, payload, "");
            return new CommmonStringEntity(SUCCESSCODE, info);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID) + "CreateProposalWithdrawTransaction");
        }
    }

}

