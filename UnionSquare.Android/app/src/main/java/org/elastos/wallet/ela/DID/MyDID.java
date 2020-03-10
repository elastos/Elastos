package org.elastos.wallet.ela.DID;

import org.elastos.did.Constants;
import org.elastos.did.DID;
import org.elastos.did.DIDBackend;
import org.elastos.did.DIDDocument;
import org.elastos.did.DIDStore;
import org.elastos.did.DIDURL;
import org.elastos.did.VerifiableCredential;
import org.elastos.did.exception.DIDBackendException;
import org.elastos.did.exception.DIDException;
import org.elastos.did.exception.DIDStoreException;
import org.elastos.did.exception.InvalidKeyException;
import org.elastos.did.exception.MalformedDIDException;
import org.elastos.wallet.ela.DID.adapter.MyDIDAdapter;
import org.elastos.wallet.ela.ElaWallet.WalletNet;
import org.elastos.wallet.ela.MyApplication;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.CommonEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonObjEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.utils.JwtUtils;
import org.elastos.wallet.ela.utils.Log;

import java.io.File;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;

import static org.elastos.did.Constants.MAX_VALID_YEARS;
import static org.elastos.wallet.ela.ElaWallet.MyWallet.SUCCESSCODE;

public class MyDID {
    private static final String DIDSDKEXCEPTIOM = "DIDException";
    private DIDStore didStore;
    private String walletId;
    private DID did;
    private MyDIDAdapter myDIDAdapter;

    private MyDID() {

    }

    private volatile static MyDID instance = null;


    public static MyDID getInstance() {
        if (instance == null) {
            synchronized (MyDID.class) {
                if (instance == null) {
                    instance = new MyDID();
                }
            }

        }
        return instance;
    }


    /**
     * 请用此对象DidStore前必须initDidStore
     * 第一次加载钱包和切换钱包时候调用
     *
     * @param walletId
     */
    public void init(String walletId) {
        if (!walletId.equals(this.walletId) || didStore == null) {
            try {
                String didResolveUrl = WalletNet.MAINDID;
                if (MyApplication.currentWalletNet == WalletNet.TESTNET) {
                    didResolveUrl = WalletNet.TESTDID;
                } else if (MyApplication.currentWalletNet == WalletNet.REGTESTNET) {
                    didResolveUrl = WalletNet.REGDID;
                } else if (MyApplication.currentWalletNet == WalletNet.PRVNET) {
                    didResolveUrl = WalletNet.PRIDID;
                }
                DIDBackend.initialize(didResolveUrl, MyApplication.getRoutDir() + File.separator + walletId + File.separator + ".cache");
                myDIDAdapter = new MyDIDAdapter();
                didStore = DIDStore.open("filesystem", MyApplication.getRoutDir() + File.separator + walletId + File.separator + "store", myDIDAdapter);//通过TestConfig.storeRoot管理多个DIDStore
                this.did = null;
                this.walletId = walletId;
            } catch (DIDException e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * 使用did前必须执行此方法
     *
     * @param payPasswd
     * @return
     */
    public DID initDID(String payPasswd) {
        if (did == null) {
            try {
                this.did = didStore.getDid(0);
                DIDURL didurl = new DIDURL(did, "primary");
                if (!didStore.containsDid(did) || !didStore.containsPrivateKey(did, didurl)) {
                    didStore.newDid(0, payPasswd);//为了什么  我直接new DID(String) 为了生成doc load
                }
            } catch (DIDStoreException e) {
                e.printStackTrace();
            }
        }
        return did;
    }

    public DID getDid() {
        return did;
    }

    public MyDIDAdapter getMyDIDAdapter() {
        return myDIDAdapter;
    }

    public void setMyDIDAdapter(MyDIDAdapter myDIDAdapter) {
        this.myDIDAdapter = myDIDAdapter;
    }

    public String getWalletId() {
        return walletId;
    }

    public void setWalletId(String walletId) {
        this.walletId = walletId;
    }

    public DIDStore getDidStore() {
        return didStore;
    }

    // 获得did的字符串的放"did:ela:"
    public String getDidString() {
        if (did == null) {
            return null;
        }
        return did.toString();
    }

   /* public Date getExpre() {
        try {
            DIDDocument doc = didStore.loadDid(did);
            return doc.getExpires();
        } catch (DIDStoreException e) {
            e.printStackTrace();
        }
        return null;
    }*/


    public String getDidPublicKey(DIDDocument doc) {
        DIDURL url = doc.getDefaultPublicKey();
        // String pkbase58 = doc.getPublicKey(url).getPublicKeyBase58();
        // String pk1 = JwtUtils.byte2hex(Base58.decode(pkbase58));
        //  String pk = JwtUtils.byte2hex(doc.getPublicKey(url).getPublicKeyBytes());
        return JwtUtils.byte2hex(doc.getPublicKey(url).getPublicKeyBytes());

    }

    public Date getExpires(DIDDocument doc) {

        return doc.getExpires();

    }

    public String getName(DIDDocument doc) {
        try {
            VerifiableCredential cre = doc.getCredential("name");
            return cre.getSubject().getPropertyAsString("name");
        } catch (Exception e) {
            return null;
        }


    }

    public DIDDocument getDIDDocument() {
        try {
            return didStore.loadDid(did);
        } catch (DIDStoreException e) {
            e.printStackTrace();
        }
        return null;
    }



    public void setDIDDocumentExprise(Date expires, String pwd, String name) {

        try {
            DIDDocument document = didStore.loadDid(did);
            DIDDocument.Builder build = document.edit();
            build.setExpires(expires);
            String[] SelfProclaimedCredential = {"SelfProclaimedCredential"};
            Map<String, String> props = new HashMap<String, String>();
            props.put("name", name);
            DIDURL didurl = new DIDURL(did, "name");
            VerifiableCredential vc1 = document.getCredential(didurl);
            if (vc1 != null) {
                build.removeCredential(didurl);
            }
            build.addCredential(didurl, SelfProclaimedCredential, props, pwd);
            DIDDocument newDoc = build.seal(pwd);
            didStore.storeDid(newDoc);//存储本地
        } catch (DIDStoreException | InvalidKeyException e) {
            e.printStackTrace();
        }


    }

  /*  private boolean resolve() throws DIDStoreException, DIDBackendException {
        //List<DID> dids = store.listDids(DIDStore.DID_ALL);//获得所有私钥 本地
        DID did = null;//todo 获取did的方法  是否同步单个
    *//*  DIDDocument doc = store.loadDid(did);
        DIDURL url = doc.getDefaultPublicKey();*//*
        // containsPrivateKey(DID did, DIDURL id)
        DIDURL didurl = new DIDURL(did, "primary");
        DIDDocument doc;
        if (didStore.containsPrivateKey(did, didurl)) {
            doc = did.resolve();//一般耗时
            return doc == null;

        } else {
            didStore.synchronize("");//比较耗时
            doc = didStore.loadDid(did);
            return doc == null;
        }
       *//* if (dids.size() == 0) {
            DIDDocument doc1 = store.newDid(payPasswd);
            did = doc1.getSubject();
        } else {
            did = dids.get(0);
        }*//*

        DIDDocument doc = did.resolve();//判断doc为空?判断是发布过did? 同步api 子线程开启


    }*/

    public BaseEntity DIDResolve(DID did) {
        try {

            return new CommmonObjEntity(SUCCESSCODE, did.resolve());
        } catch (DIDBackendException e) {
            return exceptionProcess(e, "DIDResolve");

        }
    }

    public BaseEntity DIDResolve(String didString) {
        try {
            DID did = new DID(didString);
            return DIDResolve(did);
        } catch (MalformedDIDException e) {
            return exceptionProcess(e, "DIDResolve");

        }
    }

 /*   public BaseEntity DIDResolve() {
        try {

            return new CommmonBooleanEntity(SUCCESSCODE, did.resolve() == null);
        } catch (DIDBackendException e) {
            return exceptionProcess(e, formatWalletName(did.toString(), "") + "DIDResolveByDID");

        }
    }*/

    public BaseEntity DIDPublish(String pwd) {
        try {
            String lastTxid = didStore.publishDid(did, 0, pwd);
            return new CommmonStringEntity(SUCCESSCODE, lastTxid);
        } catch (DIDException e) {
            return exceptionProcess(e, "DIDPublish");

        }
    }

    private BaseEntity exceptionProcess(DIDException e, String methodName) {
        e.printStackTrace();
        String message = "method:" + methodName + "\nwalletId:" + walletId + "\nException:" + e.getClass().getName() +
                "\nmes:" + e.getMessage();
        return new CommonEntity(DIDSDKEXCEPTIOM, message);

    }

}
