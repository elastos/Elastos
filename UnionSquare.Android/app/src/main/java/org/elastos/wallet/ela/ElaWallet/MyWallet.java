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
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringListEntity;
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
    public static final String IdChain = "IdChain";
    public static final long RATE = 100000000;
    public static final String RATE_S = RATE + "";
    public static final Double RATE_ = 100000000.0;
    public static final long feePerKb = 10000L;
    public static String Url = MyApplication.chainID == 0 ? "https://blockchain.elastos.org/tx/" : "https://blockchain-beta.elastos.org/tx/";

    public MasterWalletManager mMasterWalletManager = null;
    //private IDidManager mDidManager = null;
    private String mRootPath = null;
    public static final String SUCESSCODE = "00000";
    private static final String WalletException = "000001";
    private static final String JSONEXCEPTION = "000002";

    private int errCodeInvalidMasterWallet = 10002;
    private int errCodeInvalidSubWallet = 10003;
    private int errCodeCreateMasterWallet = 10004;
    private int errCodeCreateSubWallet = 10005;
    private int errCodeImportFromKeyStore = 10008;
    private int errCodeImportFromMnemonic = 10009;
    private int errCodeSubWalletInstance = 10010;

    public MyWallet() {

        mRootPath = MyUtil.getRootPath();
//		mDIDManagerSupervisor = new DIDManagerSupervisor(mRootPath);
        mMasterWalletManager = new MasterWalletManager(mRootPath);
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
        Log.d(TAG, "onDestroy");
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
    // args[2]: long feePerKb

    public BaseEntity createSubWallet(String masterWalletID, String chainID) {
        long feePerKb = 10000;

        Log.d(TAG, "<<< createSubWallet >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);

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
            return new CommmonStringEntity(SUCESSCODE, chainID);
        } catch (WalletException e) {
            return exceptionProcess(e, "Create " + formatWalletName(masterWalletID, chainID));
        }

    }

    // args[0]: String masterWalletID
    public BaseEntity getMasterWalletBasicInfo(String masterWalletID) {
        Log.d(TAG, "<<< getMasterWalletBasicInfo >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);

        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));
            }

            String basicInfo = masterWallet.GetBasicInfo();
            Log.d(TAG, "result: " + basicInfo);

            return new CommmonStringEntity(SUCESSCODE, basicInfo);
        } catch (WalletException e) {
            return exceptionProcess(e, "Get " + formatWalletName(masterWalletID) + " basic info");
        }
    }

    public BaseEntity getAllMasterWallets() {
        Log.d(TAG, "<<< getAllMasterWallets >>>");
        ArrayList<String> list = new ArrayList<>();
        try {
            ArrayList<MasterWallet> masterWalletList = mMasterWalletManager.GetAllMasterWallets();
            for (int i = 0; i < masterWalletList.size(); i++) {
                list.add(masterWalletList.get(i).GetID());
            }
            Log.d(TAG, "result: count = " + list.size());
            return new CommmonStringListEntity(SUCESSCODE, list);
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

        Log.d(TAG, "<<< getAllSubWallets >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);

        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get->getAllSubWallets " + formatWalletName(masterWalletID));

            }
            ArrayList<SubWallet> subWalletList = masterWallet.GetAllSubWallets();
            ArrayList<org.elastos.wallet.ela.db.table.SubWallet> newSubWalletList =new ArrayList<>();
            for (SubWallet subWallet : subWalletList) {
                org.elastos.wallet.ela.db.table.SubWallet newSubWallet = new org.elastos.wallet.ela.db.table.SubWallet();
                newSubWallet.setBalance(subWallet.GetBalance(SubWallet.BalanceType.Total) + "");
                newSubWallet.setBelongId(masterWalletID);
                newSubWallet.setChainId(subWallet.GetChainID());
                newSubWalletList.add(newSubWallet);
            }


            Log.d(TAG, "result: count = " + subWalletList.size());

            return new ISubWalletListEntity(SUCESSCODE, newSubWalletList);
        } catch (WalletException e) {
            return exceptionProcess(e, "Get " + masterWalletID + " all subwallets");
        }
    }


    // args[0]: String language
    public BaseEntity generateMnemonic(String language) {
        Log.d(TAG, "<<< generateMnemonic >>>");
        Log.d(TAG, "arg[0]: " + language);

        String mnemonic = null;
        try {
            mnemonic = mMasterWalletManager.GenerateMnemonic(language);
            return new CommmonStringEntity(SUCESSCODE, mnemonic);
        } catch (WalletException e) {
            return exceptionProcess(e, "Generate mnemonic in '" + language + "'");

        }

    }

    // args[0]: String masterWalletID
    // args[1]: String address
    public BaseEntity isAddressValid(String masterWalletID, String addr) {

        Log.d(TAG, "<<< isAddressValid >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + addr);

        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));

            }

            boolean valid = masterWallet.IsAddressValid(addr);
            Log.d(TAG, "result: " + valid);
            return new CommmonBooleanEntity(SUCESSCODE, valid);
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

        Log.d(TAG, "<<< createMasterWallet >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + "mnemonic");
        Log.d(TAG, "arg[2]: " + "phrasePasswd");
        Log.d(TAG, "arg[3]: " + "payPasswd");
        Log.d(TAG, "arg[4]: " + singleAddress);

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
            return new CommmonStringEntity(SUCESSCODE, basicInfo);
        } catch (WalletException e) {
            return exceptionProcess(e, "Create " + formatWalletName(masterWalletID));
        }

    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    public BaseEntity destroySubWallet(String masterWalletID, String chainID) {

        Log.d(TAG, "<<< destroySubWallet >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);

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

            return new CommmonStringEntity(SUCESSCODE, chainID);
            // successProcess(cc, "Destroy " + formatWalletName(masterWalletID, chainID) + " OK");
        } catch (WalletException e) {
            return exceptionProcess(e, "Destroy " + formatWalletName(masterWalletID, chainID));
        }
    }

    // args[0]: String masterWalletID
    public BaseEntity destroyWallet(String masterWalletID) {

        Log.d(TAG, "<<< destroyWallet >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);

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

            return new CommmonStringEntity(SUCESSCODE, "成功");
        } catch (WalletException e) {
            return exceptionProcess(e, "Destroy " + formatWalletName(masterWalletID));
        }
    }


    // args[0]: String masterWalletID
    // args[1]: String keystoreContent
    // args[2]: String backupPassword
    // args[3]: String payPassword
    public BaseEntity importWalletWithKeystore(String masterWalletID, String keystoreContent, String backupPassword, String payPassword) {

        Log.d(TAG, "<<< importWalletWithKeystore >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + keystoreContent);
        Log.d(TAG, "arg[2]: " + "backupPasswd");
        Log.d(TAG, "arg[3]: " + "payPasswd");

        try {
            MasterWallet masterWallet = mMasterWalletManager.ImportWalletWithKeystore(
                    masterWalletID, keystoreContent, backupPassword, payPassword);
            if (masterWallet == null) {
                return errorProcess(errCodeImportFromKeyStore + "", "Import " + formatWalletName(masterWalletID) + " with keystore");

            }

            createDIDManager(masterWallet);

            Log.d(TAG, "result: " + masterWallet.GetBasicInfo());
            return new CommmonStringEntity(SUCESSCODE, masterWallet.GetBasicInfo());

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

        Log.d(TAG, "<<< importWalletWithMnemonic >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + "mnemonic");
        Log.d(TAG, "arg[2]: " + "phrasePasswd");
        Log.d(TAG, "arg[3]: " + "payPasswd");
        Log.d(TAG, "arg[4]: " + singleAddress);

        try {
            MasterWallet masterWallet = mMasterWalletManager.ImportWalletWithMnemonic(
                    masterWalletID, mnemonic, phrasePassword, payPassword, singleAddress);
            if (masterWallet == null) {
                return errorProcess(errCodeImportFromMnemonic + "", "Import " + formatWalletName(masterWalletID) + " with mnemonic");

            }

            createDIDManager(masterWallet);

            Log.d(TAG, "result: " + masterWallet.GetBasicInfo());
            return new CommmonStringEntity(SUCESSCODE, masterWallet.GetBasicInfo());

        } catch (WalletException e) {
            return exceptionProcess(e, "Import " + formatWalletName(masterWalletID) + " with mnemonic");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String backupPassword
    // args[2]: String payPassword
    public BaseEntity exportWalletWithKeystore(String masterWalletID, String backupPassword, String payPassword) {

        Log.d(TAG, "<<< exportWalletWithKeystore >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: backupPasswd");
        Log.d(TAG, "arg[2]: payPasswd");

        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));

            }

            String keystore = mMasterWalletManager.ExportWalletWithKeystore(masterWallet, backupPassword, payPassword);

            Log.d(TAG, "result: " + keystore);
            return new CommmonStringEntity(SUCESSCODE, keystore);
        } catch (WalletException e) {
            return exceptionProcess(e, "Export " + formatWalletName(masterWalletID) + "to keystore");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String payPassword
    public BaseEntity exportWalletWithMnemonic(String masterWalletID, String backupPassword) {

        Log.d(TAG, "<<< exportWalletWithMnemonic >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: backupPasswd");

        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));

            }

            String mnemonic = mMasterWalletManager.ExportWalletWithMnemonic(masterWallet, backupPassword);

            return new CommmonStringEntity(SUCESSCODE, mnemonic);
        } catch (WalletException e) {
            return exceptionProcess(e, "Export " + masterWalletID + " to mnemonic");
        }
    }


    // args[0]: String masterWalletID
    // args[1]: String payPassword
    public BaseEntity exportWalletWithMnemonic_1(String masterWalletID, String backupPassword) {

        Log.d(TAG, "<<< exportWalletWithMnemonic_1 >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: backupPasswd");

        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));

            }

            String mnemonic = mMasterWalletManager.ExportWalletWithMnemonic(masterWallet, backupPassword);

            //  return new CommmonStringEntity(SUCESSCODE, mnemonic);
            return new CommmonStringWithiMethNameEntity(SUCESSCODE, mnemonic, "exportWalletWithMnemonic");
        } catch (WalletException e) {
            return exceptionProcess(e, "Export " + masterWalletID + " to mnemonic");
        }
    }


    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: int BalanceType (0: Default, 1: Voted, 2: Total)
    public BaseEntity getBalance(String masterWalletID, String chainID, SubWallet.BalanceType BalanceType) {

        Log.d(TAG, "<<< getBalance >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);
        Log.d(TAG, "arg[2]: " + BalanceType);

        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID) + " balance");

            }

            Log.d(TAG, "result: " + subWallet.GetBalance(BalanceType));
            return new CommmonObjEntity(SUCESSCODE, new BalanceEntity(chainID, subWallet.GetBalance(BalanceType) + "", masterWalletID));

        } catch (WalletException e) {
            return exceptionProcess(e, "Get " + formatWalletName(masterWalletID, chainID) + " balance");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    public BaseEntity createAddress(String masterWalletID, String chainID) {

        Log.d(TAG, "<<< createAddress >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);

        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            String address = subWallet.CreateAddress();
            Log.d(TAG, "result: " + address);
            return new CommmonStringEntity(SUCESSCODE, address);
        } catch (WalletException e) {
            return exceptionProcess(e, "Create " + formatWalletName(masterWalletID, chainID) + " address");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: int start
    // args[3]: int count
    public BaseEntity getAllAddress(String masterWalletID, String chainID, int start, int count) {

        Log.d(TAG, "<<< getAllAddress >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);
        Log.d(TAG, "arg[2]: " + start);
        Log.d(TAG, "arg[3]: " + count);

        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }
            String allAddresses = subWallet.GetAllAddress(start, count);

            Log.d(TAG, "result: " + allAddresses);

            return new CommmonStringEntity(SUCESSCODE, allAddresses);
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
    // args[6]: String remark
    public BaseEntity createTransaction(String masterWalletID, String chainID, String fromAddress,
                                        String toAddress, long amount, String memo, String remark, boolean useVotedUTXO) {

        Log.d(TAG, "<<< createTransaction >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);
        Log.d(TAG, "arg[2]: " + fromAddress);
        Log.d(TAG, "arg[3]: " + toAddress);
        Log.d(TAG, "arg[4]: " + amount);
        Log.d(TAG, "arg[5]: " + memo);
        Log.d(TAG, "arg[6]: " + remark);
        Log.d(TAG, "arg[7]: " + useVotedUTXO);

        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            String tx = subWallet.CreateTransaction(fromAddress, toAddress, amount, memo, remark, useVotedUTXO);

            Log.d(TAG, "result: " + tx);

            return new CommmonStringEntity(SUCESSCODE, tx);
        } catch (WalletException e) {
            return exceptionProcess(e, "Create " + formatWalletName(masterWalletID, chainID) + " tx");
        }
    }


    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String rawTransaction
    // args[3]: long feePerKb
    // return:  long fee
    public BaseEntity calculateTransactionFee(String masterWalletID, String chainID, String rawTransaction, long feePerKb) {

        Log.d(TAG, "<<< calculateTransactionFee >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);
        Log.d(TAG, "arg[2]: " + rawTransaction);
        Log.d(TAG, "arg[3]: " + feePerKb);

        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            long fee = subWallet.CalculateTransactionFee(rawTransaction, feePerKb);

            Log.d(TAG, "result: " + fee);
            return new CommmonLongEntity(SUCESSCODE, fee);
        } catch (WalletException e) {
            return exceptionProcess(e, "Calculate " + formatWalletName(masterWalletID, chainID) + " tx fee");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String rawTransaction
    // args[3]: long fee
    // return:  String txJson
    public BaseEntity updateTransactionFee(String masterWalletID, String chainID, String rawTransaction, long fee, String fromAddress) {

        Log.d(TAG, "<<< updateTransactionFee >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);
        Log.d(TAG, "arg[2]: " + rawTransaction);
        Log.d(TAG, "arg[3]: " + fee);
        Log.d(TAG, "arg[4]: " + fromAddress);

        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            String result = subWallet.UpdateTransactionFee(rawTransaction, fee, fromAddress);
            Log.d(TAG, "result: " + result);

            //  successProcess(cc, result);
            return new CommmonStringWithiMethNameEntity(SUCESSCODE, result, "updateTransactionFee");
        } catch (WalletException e) {
            return exceptionProcess(e, "Update " + formatWalletName(masterWalletID, chainID) + " tx fee");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String rawTransaction
    // args[3]: String payPassword
    // return:  String txJson
    public BaseEntity signTransaction(String masterWalletID, String chainID, String rawTransaction, String payPassword) {

        Log.d(TAG, "<<< signTransaction >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);
        Log.d(TAG, "arg[2]: " + rawTransaction);
        Log.d(TAG, "arg[3]: " + "payPassword");

        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            String result = subWallet.SignTransaction(rawTransaction, payPassword);
            Log.d(TAG, "result: " + result);
            return new CommmonStringWithiMethNameEntity(SUCESSCODE, result, "signTransaction");
        } catch (WalletException e) {
            return exceptionProcess(e, "Sign " + formatWalletName(masterWalletID, chainID) + " tx");
        }
    }


    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String rawTxJson
    // return:  String resultJson
    public BaseEntity publishTransaction(String masterWalletID, String chainID, String rawTxJson) {

        Log.d(TAG, "<<< publishTransaction >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);
        Log.d(TAG, "arg[2]: " + rawTxJson);

        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            String resultJson = subWallet.PublishTransaction(rawTxJson);
            Log.d(TAG, "result: " + resultJson);
            return new CommmonStringWithiMethNameEntity(SUCESSCODE, resultJson, "publishTransaction");
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

        Log.d(TAG, "<<< getAllTransaction >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);
        Log.d(TAG, "arg[2]: " + start);
        Log.d(TAG, "arg[3]: " + count);
        Log.d(TAG, "arg[4]: " + addressOrTxId);

        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            String txJson = subWallet.GetAllTransaction(start, count, addressOrTxId);
            Log.d(TAG, "result: " + txJson);
            return new CommmonStringEntity(SUCESSCODE, txJson);
        } catch (WalletException e) {
            return exceptionProcess(e, "Get " + formatWalletName(masterWalletID, chainID) + " all tx");
        }
    }


    // args[0]: String masterWalletID
    // args[1]: String chainID
    public BaseEntity registerWalletListener(String masterWalletID, String chainID, ISubWalletListener listener) {

        Log.d(TAG, "<<< registerWalletListener >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);
        Log.d(TAG, "arg[2]: " + listener);

        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));
            }

            if (subWallet.IsCallbackRegistered()) {
                Log.d(TAG, "listener already registered, do nothing");
                return new CommmonStringEntity(SUCESSCODE, "listener already registered");
            }

            subWallet.AddCallback(new SubWalletCallback(masterWalletID, chainID, listener));
            return new CommmonStringEntity(SUCESSCODE, "registerWalletListener");
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
    // args[7]: String remark
    // args[8]: boolean useVotedUTXO
    public BaseEntity createDepositTransaction(String masterWalletID, String chainID,
                                               String fromAddress, String lockedAddress,
                                               long amount, String sideChainAddress, String memo,
                                               String remark, boolean useVotedUTXO) {

        Log.d(TAG, "<<< createDepositTransaction >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);
        Log.d(TAG, "arg[2]: " + fromAddress);
        Log.d(TAG, "arg[3]: " + lockedAddress);
        Log.d(TAG, "arg[4]: " + amount);
        Log.d(TAG, "arg[5]: " + sideChainAddress);
        Log.d(TAG, "arg[6]: " + memo);
        Log.d(TAG, "arg[7]: " + remark);
        Log.d(TAG, "arg[8]: " + useVotedUTXO);

        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess(errCodeSubWalletInstance + "", formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String txJson = mainchainSubWallet.CreateDepositTransaction(fromAddress, lockedAddress, amount, sideChainAddress, memo, remark, useVotedUTXO);
            Log.d(TAG, "result: " + txJson);

            return new CommmonStringWithiMethNameEntity(SUCESSCODE, txJson, "createDepositTransaction");
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " create deposit tx");
        }
    }


    // args[0]: String masterWalletID
    public BaseEntity getSupportedChains(String masterWalletID) {

        Log.d(TAG, "<<< getSupportedChains >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);

        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));

            }

            String[] supportedChains = masterWallet.GetSupportedChains();

            Log.d(TAG, "result: " + supportedChains);

            //successProcess(cc, supportedChainsJson);
            return new CommonStringArrayEntity(SUCESSCODE, supportedChains);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID) + " get support chain");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String oldPassword
    // args[2]: String newPassword
    public BaseEntity changePassword(String masterWalletID, String oldPassword, String newPassword) {

        Log.d(TAG, "<<< changePassword >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: oldPasswd");
        Log.d(TAG, "arg[2]: newPasswd");

        try {
            MasterWallet masterWallet = getMasterWallet(masterWalletID);
            if (masterWallet == null) {
                return errorProcess(errCodeInvalidMasterWallet + "", "Get " + formatWalletName(masterWalletID));

            }

            masterWallet.ChangePassword(oldPassword, newPassword);
            return new CommmonStringEntity(SUCESSCODE, "成功");
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
    // args[6]: String remark
    public BaseEntity createWithdrawTransaction(String masterWalletID, String chainID, String fromAddress,
                                                long amount, String mainchainAddress, String memo, String remark) {

        Log.d(TAG, "<<< createWithdrawTransaction >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);
        Log.d(TAG, "arg[2]: " + fromAddress);
        Log.d(TAG, "arg[3]: " + amount);
        Log.d(TAG, "arg[4]: " + mainchainAddress);
        Log.d(TAG, "arg[5]: " + memo);
        Log.d(TAG, "arg[6]: " + remark);

        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof SidechainSubWallet)) {
                return errorProcess(errCodeSubWalletInstance + "", formatWalletName(masterWalletID, chainID) + " is not instance of SidechainSubWallet");

            }

            SidechainSubWallet sidechainSubWallet = (SidechainSubWallet) subWallet;
            String tx = sidechainSubWallet.CreateWithdrawTransaction(fromAddress, amount, mainchainAddress, memo, remark);

            Log.d(TAG, "result: " + tx);

            return new CommmonStringWithiMethNameEntity(SUCESSCODE, tx, "createWithdrawTransaction");
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " create withdraw tx");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID
    public BaseEntity getGenesisAddress(String masterWalletID, String chainID) {

        Log.d(TAG, "<<< getGenesisAddress >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);

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

            Log.d(TAG, "result: " + address);

            return new CommmonStringWithiMethNameEntity(SUCESSCODE, address, "getGenesisAddress");
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

        Log.d(TAG, "<<< generateProducerPayload >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);
        Log.d(TAG, "arg[2]: " + ownerPublicKey);
        Log.d(TAG, "arg[3]: " + nodePublicKey);
        Log.d(TAG, "arg[4]: " + nickName);
        Log.d(TAG, "arg[5]: " + url);
        Log.d(TAG, "arg[6]: " + IPAddress);
        Log.d(TAG, "arg[7]: " + location);
        Log.d(TAG, "arg[8]: payPasswd");

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

            Log.d(TAG, "result: " + payloadJson);

            KLog.a(payloadJson);
            return new CommmonStringWithiMethNameEntity(SUCESSCODE, payloadJson, "payload");
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

        Log.d(TAG, "<<< createRegisterProducerTransaction >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);
        Log.d(TAG, "arg[2]: " + fromAddress);
        Log.d(TAG, "arg[3]: " + payloadJson);
        Log.d(TAG, "arg[4]: " + amount);
        Log.d(TAG, "arg[5]: " + memo);
        Log.d(TAG, "arg[6]: " + useVotedUTXO);

        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess("" + errCodeInvalidSubWallet, "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String txJson = mainchainSubWallet.CreateRegisterProducerTransaction(fromAddress, payloadJson, amount, memo, "", useVotedUTXO);

            Log.d(TAG, "result: " + txJson);

            KLog.a(txJson);
            return new CommmonStringWithiMethNameEntity(SUCESSCODE, txJson, "createRegisterProducerTransaction");
        } catch (WalletException e) {
            return new CommmonStringWithiMethNameEntity("", "", "");
        }

    }


    // args[0]: String masterWalletID
    // args[1]: String chainID
    public BaseEntity getPublicKeyForVote(String masterWalletID, String chainID) {

        Log.d(TAG, "<<< getPublicKeyForVote >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);

        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess(errCodeInvalidSubWallet + "", "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String publicKey = mainchainSubWallet.GetPublicKeyForVote();

            Log.d(TAG, "result: " + publicKey);

            return new CommmonStringWithiMethNameEntity(SUCESSCODE, publicKey, "getPublicKeyForVote");
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

        Log.d(TAG, "<<< createVoteProducerTransaction >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);
        Log.d(TAG, "arg[2]: " + fromAddress);
        Log.d(TAG, "arg[3]: " + stake);
        Log.d(TAG, "arg[4]: " + publicKeys);
        Log.d(TAG, "arg[5]: " + memo);
        Log.d(TAG, "arg[6]: " + useVotedUTXO);

        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess("" + errCodeInvalidSubWallet, "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String txJson = mainchainSubWallet.CreateVoteProducerTransaction(fromAddress, stake, publicKeys, memo, "", useVotedUTXO);

            Log.d(TAG, "result: " + txJson);

            return new CommmonStringWithiMethNameEntity(SUCESSCODE, txJson, "createVoteProducerTransaction");
            // successProcess(cc, txJson);
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " create vote producer tx");
        }
        // return new CommmonStringWithiMethNameEntity(SUCESSCODE, "", "createVoteProducerTransaction");
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID (only main chain ID 'ELA')
    public BaseEntity getVotedProducerList(String masterWalletID, String chainID) {

        Log.d(TAG, "<<< getVotedProducerList >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);

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
            Log.d(TAG, "result: " + list);

            return new CommmonStringWithiMethNameEntity(SUCESSCODE, list, "getVotedProducerList");

        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " get voted producer list");
        }
    }

    // args[0]: String masterWalletID
    // args[1]: String chainID (only main chain ID 'ELA')
    public BaseEntity getRegisteredProducerInfo(String masterWalletID, String chainID) {

        Log.d(TAG, "<<< getRegisteredProducerInfo >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);

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
            Log.d(TAG, "result: " + info);
            KLog.a(info);
            return new CommmonStringEntity(SUCESSCODE, info);
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

        Log.d(TAG, "<<< generateCancelProducerPayload >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);
        Log.d(TAG, "arg[2]: " + publicKey);
        Log.d(TAG, "arg[3]: " + "payPasswd");

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
            Log.d(TAG, "result: " + payloadJson);
            return new CommmonStringWithiMethNameEntity(SUCESSCODE, payloadJson, "generateCancelProducerPayload");
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
    // args[5]: String remark
    // args[6]: boolean useVotedUTXO
    public BaseEntity createCancelProducerTransaction(String masterWalletID, String chainID, String fromAddress, String payloadJson, String memo, String remark, boolean useVotedUTXO) {

        Log.d(TAG, "<<< createCancelProducerTransaction >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);
        Log.d(TAG, "arg[2]: " + fromAddress);
        Log.d(TAG, "arg[3]: " + payloadJson);
        Log.d(TAG, "arg[4]: " + memo);
        Log.d(TAG, "arg[5]: " + remark);
        Log.d(TAG, "arg[6]: " + useVotedUTXO);

        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess("" + errCodeInvalidSubWallet, "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String txJson = mainchainSubWallet.CreateCancelProducerTransaction(fromAddress, payloadJson, memo, remark, useVotedUTXO);
            Log.d(TAG, "result: " + txJson);
            return new CommmonStringWithiMethNameEntity(SUCESSCODE, txJson, "createCancelProducerTransaction");
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " create cancel producer tx");
        }
    }

    //取回押金
    public BaseEntity createRetrieveDepositTransaction(String masterWalletID, String chainID, long amount, String memo, String remark) {

        Log.d(TAG, "<<< createRetrieveDepositTransaction >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);
        Log.d(TAG, "arg[2]: " + amount);
        Log.d(TAG, "arg[3]: " + memo);
        Log.d(TAG, "arg[4]: " + remark);

        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess("" + errCodeInvalidSubWallet, "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");
            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String txJson = mainchainSubWallet.CreateRetrieveDepositTransaction(amount, memo, remark);
            Log.d(TAG, "result: " + txJson);

            return new CommmonStringWithiMethNameEntity(SUCESSCODE, txJson, "createRetrieveDepositTransaction");
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " create retrieve deposit tx");
        }
    }


    // args[0]: String masterWalletID
    // args[1]: String chainID
    // args[2]: String fromAddress
    // args[3]: String payloadJson
    // args[4]: String memo
    // args[5]: String remark
    // args[6]: boolean useVotedUTXO
    public BaseEntity createUpdateProducerTransaction(String masterWalletID, String chainID, String fromAddress, String payloadJson, String memo, String remark, boolean useVotedUTXO) {

        Log.d(TAG, "<<< createUpdateProducerTransaction >>>");
        Log.d(TAG, "arg[0]: " + masterWalletID);
        Log.d(TAG, "arg[1]: " + chainID);
        Log.d(TAG, "arg[2]: " + fromAddress);
        Log.d(TAG, "arg[3]: " + payloadJson);
        Log.d(TAG, "arg[4]: " + memo);
        Log.d(TAG, "arg[5]: " + remark);
        Log.d(TAG, "arg[6]: " + useVotedUTXO);

        try {
            SubWallet subWallet = getSubWallet(masterWalletID, chainID);
            if (subWallet == null) {
                return errorProcess("" + errCodeInvalidSubWallet, "Get " + formatWalletName(masterWalletID, chainID));

            }

            if (!(subWallet instanceof MainchainSubWallet)) {
                return errorProcess("" + errCodeSubWalletInstance, formatWalletName(masterWalletID, chainID) + " is not instance of MainchainSubWallet");

            }

            MainchainSubWallet mainchainSubWallet = (MainchainSubWallet) subWallet;

            String txJson = mainchainSubWallet.CreateUpdateProducerTransaction(fromAddress, payloadJson, memo, remark, useVotedUTXO);
            Log.d(TAG, "result: " + txJson);
            return new CommmonStringWithiMethNameEntity(SUCESSCODE, txJson, "createUpdateProducerTransaction");
        } catch (WalletException e) {
            return exceptionProcess(e, formatWalletName(masterWalletID, chainID) + " create update producer tx");
        }
    }
}

