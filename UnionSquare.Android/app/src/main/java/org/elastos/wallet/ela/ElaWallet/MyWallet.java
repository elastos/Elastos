package org.elastos.wallet.ela.ElaWallet;

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
 * wallet webview jni
 */
public class MyWallet {
    private static final String TAG = "Wallet";
    public static final String ELA = "ELA";
    public static final long RATE = 100000000;
    public static final String RATE_S = RATE + "";
    public static final Double RATE_ = 100000000.0;
    public static final long feePerKb = 10000L;
    public static String Url = getTxHashBaseUrl();
    public MasterWalletManager mMasterWalletManager = null;
    //private IDidManager mDidManager = null;
    private String mRootPath = null;
    public static final String SUCCESSCODE = "00000";
    private static final String WalletException = "000001";
    private static final String JSONEXCEPTION = "000002";

    private int errCodeInvalidMasterWallet = 10002;
    private int errCodeInvalidSubWallet = 10003;
    private int errCodeCreateMasterWallet = 10004;
    private int errCodeCreateSubWallet = 10005;
    private int errCodeImportFromKeyStore = 10008;
    private int errCodeImportFromMnemonic = 10009;
    private int errCodeSubWalletInstance = 10010;
    private int errCodeImportReadonlyWallet = 10011;


    public MyWallet() {

        mRootPath = MyApplication.getAppContext().getFilesDir().getParent();
//		mDIDManagerSupervisor = new DIDManagerSupervisor(mRootPath);
        mMasterWalletManager = new MasterWalletManager(mRootPath);
    }

    private static String getTxHashBaseUrl() {
        String baseUrl = "https://blockchain.elastos.org/tx/";
        switch (MyApplication.chainID) {
            case 1:
                baseUrl = "https://blockchain-beta.elastos.org/tx/";
                break;
            case 2:
                baseUrl = "https://blockchain-regtest.elastos.org/";
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
        if (mMasterWalletManager != null) {
            mMasterWalletManager.SaveConfigs();
        }

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
            ArrayList<MasterWallet> masterWalletList = mMasterWalletManager.GetAllMasterWallets();
            for (int i = 0; i < masterWalletList.size(); i++) {
                MasterWallet masterWallet = masterWalletList.get(i);
                ArrayList<SubWallet> subWallets = masterWallet.GetAllSubWallets();
                for (int j = 0; j < subWallets.size(); ++j) {
                    subWallets.get(j).RemoveCallback();
                }
            }

            mMasterWalletManager.Dispose();
            mMasterWalletManager = null;
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

        return mMasterWalletManager.GetWallet(masterWalletID);
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

    // args[0]: String masterWalletID
    // args[1]: String chainID

    public BaseEntity createSubWallet(String masterWalletID, String chainID) {
        long feePerKb = 10000;
        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));
            }

            SubWallet subWallet = masterWallet.CreateSubWallet(chainID, feePerKb);
            if (subWallet == null) {
                return errorProcess(errCodeCreateSubWallet + "", "Create " + formatWalletName(masterWalletID, chainID));

            }

            Log.d(TAG, "result " + subWallet.GetBasicInfo());
            //basicInfo = subWallet.GetBasicInfo();
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
            Log.d(TAG, "result: " + basicInfo);

            return new CommmonStringEntity(SUCCESSCODE, basicInfo);
        } catch (WalletException e) {
            return exceptionProcess(e, "Get " + formatWalletName(masterWalletID) + " basic info");
        }
    }

    public BaseEntity getAllMasterWallets() {
        try {
            ArrayList<MasterWallet> masterWalletList = mMasterWalletManager.GetAllMasterWallets();
            for (int i = 0; i < masterWalletList.size(); i++) {
                Log.d(TAG, "result:  = " + masterWalletList.get(i).GetBasicInfo());
            }

            return new CommmonObjectWithMethNameEntity(SUCCESSCODE, masterWalletList, "getAllMasterWallets");
        } catch (WalletException e) {
            return exceptionProcess(e, "Get all master wallets");
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
                newSubWallet.setBalance(subWallet.GetBalance(SubWallet.BalanceType.Total) + "");
                newSubWallet.setBelongId(masterWalletID);
                newSubWallet.setChainId(subWallet.GetChainID());
                newSubWalletList.add(newSubWallet);
            }


            Log.d(TAG, "result: count = " + subWalletList.size());

            return new ISubWalletListEntity(SUCCESSCODE, newSubWalletList);
        } catch (WalletException e) {
            return exceptionProcess(e, "Get " + masterWalletID + " all subwallets");
        }
    }


    // args[0]: String language
    public BaseEntity generateMnemonic(String language) {
        String mnemonic = null;
        try {
            mnemonic = mMasterWalletManager.GenerateMnemonic(language);
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
            Log.d(TAG, "result: " + valid);
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

            Log.d(TAG, "result: " + masterWallet.GetBasicInfo());
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

            return new CommmonStringEntity(SUCCESSCODE, "成功");
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

            Log.d(TAG, "result: " + masterWallet.GetBasicInfo());
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
                    masterWalletID, mnemonic, phrasePassword, payPassword, singleAddress);
            if (masterWallet == null) {
                return errorProcess(errCodeImportFromMnemonic + "", "Import " + formatWalletName(masterWalletID) + " with mnemonic");
            }

            createDIDManager(masterWallet);

            Log.d(TAG, "result: " + masterWallet.GetBasicInfo());
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

            String keystore = mMasterWalletManager.ExportWalletWithKeystore(masterWallet, backupPassword, payPassword);

            Log.d(TAG, "result: " + keystore);
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

            String mnemonic = mMasterWalletManager.ExportWalletWithMnemonic(masterWallet, backupPassword);

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

            String walletJson = mMasterWalletManager.ExportReadonlyWallet(masterWallet);

            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, walletJson, "exportReadonlyWallet");
        } catch (WalletException e) {
            return exceptionProcess(e, "Export read-only wallet " + formatWalletName(masterWalletID));
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: int BalanceType (0: Default, 1: Voted, 2: Total)
    public BaseEntity getBalance(String masterWalletID, String chainID, SubWallet.BalanceType BalanceType) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID) + " balance");

            }

            return new CommmonObjEntity(SUCCESSCODE, new BalanceEntity(chainID, subWallet.GetBalance(BalanceType) + "", masterWalletID));

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
    // args[2]: String fromAddress
    // args[3]: String toAddress
    // args[4]: long amount
    // args[5]: String memo
    // args[6]: Boolean useVotedUTXO
    public BaseEntity createTransaction(String masterWalletID, String chainID, String fromAddress,
                                        String toAddress, long amount, String memo, boolean useVotedUTXO) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));
            }

            String tx = subWallet.CreateTransaction(fromAddress, toAddress, amount, memo, useVotedUTXO);

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
            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, result, "signTransaction");
        } catch (WalletException e) {
            return exceptionProcess(e, "Sign " + formatWalletName(masterWalletID, chainID) + " tx");
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

            if (subWallet.IsCallbackRegistered()) {
                Log.d(TAG, "listener already registered, do nothing");
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
    // args[3]: String lockedAddress
    // args[4]: long amount
    // args[5]: String sideChainAddress
    // args[6]: String memo
    // args[7]: boolean useVotedUTXO
    public BaseEntity createDepositTransaction(String masterWalletID, String chainID,
                                               String fromAddress, String lockedAddress,
                                               long amount, String sideChainAddress, String memo, boolean useVotedUTXO) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess(errCodeSubWalletInstance + "", formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String txJson = mainchainSubWallet.CreateDepositTransaction(fromAddress, lockedAddress, amount, sideChainAddress, memo, useVotedUTXO);

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
            return new CommmonStringEntity(SUCCESSCODE, "成功");
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
                                                long amount, String mainchainAddress, String memo) {
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
    public BaseEntity generateProducerPayload(String masterWalletID, String chainID, String ownerPublicKey, String nodePublicKey, String nickName, String url, String IPAddress, long location, String payPasswd) throws JSONException {
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
            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, payloadJson, "payload");
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
    public BaseEntity createRegisterProducerTransaction(String masterWalletID, String chainID, String fromAddress, String payloadJson, long amount, String memo, boolean useVotedUTXO) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess("" + errCodeInvalidSubWallet, "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String txJson = mainchainSubWallet.CreateRegisterProducerTransaction(fromAddress, payloadJson, amount, memo, useVotedUTXO);

            KLog.a(txJson);
            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, txJson, "createRegisterProducerTransaction");
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
    // args[6]: boolean useVotedUTXO
    public BaseEntity createVoteProducerTransaction(String masterWalletID, String chainID, String fromAddress, long stake, String publicKeys, String memo, boolean useVotedUTXO) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess("" + errCodeInvalidSubWallet, "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String txJson = mainchainSubWallet.CreateVoteProducerTransaction(fromAddress, stake, publicKeys, memo, useVotedUTXO);

            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, txJson, "createVoteProducerTransaction");
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
            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, payloadJson, "generateCancelProducerPayload");
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
    public BaseEntity createCancelProducerTransaction(String masterWalletID, String chainID, String fromAddress, String payloadJson, String memo, boolean useVotedUTXO) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess("" + errCodeInvalidSubWallet, "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String txJson = mainchainSubWallet.CreateCancelProducerTransaction(fromAddress, payloadJson, memo, useVotedUTXO);

            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, txJson, "createCancelProducerTransaction");
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " create cancel producer tx");
        }
    }

    //取回押金
    public BaseEntity createRetrieveDepositTransaction(String masterWalletID, String chainID, long amount, String memo) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess("" + errCodeInvalidSubWallet, "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");
            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String txJson = mainchainSubWallet.CreateRetrieveDepositTransaction(amount, memo);

            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, txJson, "createRetrieveDepositTransaction");
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

            String txJson = mainchainSubWallet.CreateUpdateProducerTransaction(fromAddress, payloadJson, memo, useVotedUTXO);

            return new CommmonStringWithiMethNameEntity(SUCCESSCODE, txJson, "createUpdateProducerTransaction");
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " create update producer tx");
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

    public BaseEntity createCombineUTXOTransaction(String masterWalletID, String chainID, String memo, boolean useVotedUTXO) {
        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            String result = subWallet.CreateCombineUTXOTransaction(memo, useVotedUTXO);
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
        return new CommmonStringWithiMethNameEntity(SUCCESSCODE, "", "getVersion");
    }
}

