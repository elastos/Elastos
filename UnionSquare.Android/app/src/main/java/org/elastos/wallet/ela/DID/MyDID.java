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

package org.elastos.wallet.ela.DID;

import android.text.TextUtils;

import com.blankj.utilcode.util.ToastUtils;

import org.elastos.did.DID;
import org.elastos.did.DIDBackend;
import org.elastos.did.DIDDocument;
import org.elastos.did.DIDStore;
import org.elastos.did.DIDURL;
import org.elastos.did.Issuer;
import org.elastos.did.VerifiableCredential;
import org.elastos.did.exception.DIDException;
import org.elastos.did.exception.DIDStoreException;
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
import java.util.Date;
import java.util.HashMap;
import java.util.Map;

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
        // Log.i("??", "1");
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
                // Log.i("??", "2");
                didStore = DIDStore.open("filesystem", MyApplication.getRoutDir() + File.separator + walletId + File.separator + "store", myDIDAdapter);//通过TestConfig.storeRoot管理多个DIDStore
                // Log.i("??", "3");
                // Log.i("??", didStore.containsPrivateIdentity() + "");
                this.did = null;
                this.walletId = walletId;
            } catch (DIDException e) {
                e.printStackTrace();
                ToastUtils.showShort(e.getMessage());
            }
        }
    }

    /**
     * 使用did前必须执行此方法 使用前提containsPrivateIdentity
     *
     * @return
     */
    public DID initDID(String payPasswd) {

        try {
            if (did == null) {
                this.did = didStore.getDid(0);
            }
            DIDURL didurl = new DIDURL(did, "primary");
            if (!didStore.containsDid(did) || !didStore.containsPrivateKey(did, didurl)) {
                didStore.deleteDid(did);
                didStore.newDid(0, payPasswd);//为了什么  我直接new DID(String) 为了生成doc 是的能够loaddoc
            }
        } catch (DIDStoreException e) {
            e.printStackTrace();
            ToastUtils.showShort(e.getMessage());
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

    // 获得did的字符串的含"did:elastos:"
    public String getDidString() {
        if (did == null) {
            return null;
        }
        return did.toString();
    }

    // 获得did的字符串的不含"did:elastos:"
    public String getSpecificDidString() {
        if (did == null) {
            return null;
        }
        return did.getMethodSpecificId();
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
            //ToastUtils.showShort(e.getMessage());
            return null;
        }


    }

    public DIDDocument getDIDDocument() {
        try {
            return didStore.loadDid(did);
        } catch (DIDStoreException e) {
            ToastUtils.showShort(e.getMessage());
            e.printStackTrace();
        }
        return null;
    }

    /**
     * 设置凭证信息  不上链
     *
     * @param credencialJson
     * @param pwd
     * @param expires
     */
    public boolean storeCredential(String credencialJson, String pwd, Date expires) {

        try {
            if (TextUtils.isEmpty(credencialJson) || credencialJson.equals("null")) {
                //如果上次发送失败但是 setCredential保存成功   若重新发布时为空判空覆盖
                didStore.deleteCredential(did.toString(), "outPut");
                return true;
            }
            Issuer issuer = new Issuer(did, didStore);//发布者的did
            String[] SelfProclaimedCredential = {"BasicProfileCredential"};
            VerifiableCredential vc = issuer.issueFor(did)//被发布的did
                    .id("outPut")//唯一标识一个VerifiableCredential 相同会覆盖
                    .type(SelfProclaimedCredential)
                    .expirationDate(expires)
                    .properties(credencialJson)
                    .seal(pwd);
            didStore.storeCredential(vc);
            return true;

        } catch (DIDException e) {
            ToastUtils.showShort(e.getMessage());
            e.printStackTrace();
        }
        return false;

    }

    /**
     * 获得本地存储的不上链的凭证字符串
     *
     * @return
     */
    public String getCredentialJSon(String didString) {

        try {
            VerifiableCredential vc1 = didStore.loadCredential(didString, "outPut");
            if (vc1 == null || vc1.getSubject() == null) {
                return null;
            }
            return vc1.toString();

        } catch (Exception e) {
            ToastUtils.showShort(e.getMessage());
            e.printStackTrace();
        }

        return null;
    }

    public String getCredentialProFromStore(String didString) {

        try {
            VerifiableCredential vc1 = didStore.loadCredential(didString, "outPut");
            if (vc1 == null || vc1.getSubject() == null) {
                return null;
            }
            //Log.i("??", vc1.toString());
            return vc1.getSubject().getPropertiesAsString();

        } catch (Exception e) {
            ToastUtils.showShort(e.getMessage());
            e.printStackTrace();
        }

        return null;
    }

    public String getCredentialProFromJson(String credencialJson) {
        if (TextUtils.isEmpty(credencialJson)) {
            return null;
        }
        try {
            VerifiableCredential vcFrom = VerifiableCredential.fromJson(credencialJson);//结果
            return vcFrom.getSubject().getPropertiesAsString();

        } catch (Exception e) {
            // ToastUtils.showShort(e.getMessage());
            e.printStackTrace();
        }

        return null;
    }

    public String getCredentialJson(String props, String pwd, Date expires) {

        try {
            Issuer issuer = new Issuer(did, didStore);//发布者的did
            String[] SelfProclaimedCredential = {"BasicProfileCredential"};
            VerifiableCredential vc = issuer.issueFor(did)//被发布的did
                    .id("outPut")//唯一标识一个VerifiableCredential 相同会覆盖
                    .type(SelfProclaimedCredential)
                    .expirationDate(expires)
                    .properties(props)
                    .seal(pwd);
            return vc.toString();

        } catch (DIDException e) {
            ToastUtils.showShort(e.getMessage());
            e.printStackTrace();
        }

        return null;
    }

    /* DID did = vc.getSubject().getId();//issueFor()的did 被发布的did
                vc.getId();//didurl
                String id = vc.getId().getFragment();//.id
                 DIDURL didurl = new DIDURL(did, "outPut");
               VerifiableCredential vc1 = didStore.loadCredential(did, didurl);
               if (vc1 != null) {
                   didStore.deleteCredential(did, didurl);
               }*/
    public boolean restoreCredential(String fromJson, String pwd, Date expires) {

        try {
            VerifiableCredential vcFrom = VerifiableCredential.fromJson(fromJson);//结果
            // vcFrom.isGenuine()判断数据完整性
            //  vcFrom.toString()
            String props = vcFrom.getSubject().getPropertiesAsString();//properties(String json)

            Issuer issuer = new Issuer(did, didStore);//发布者的did
            String[] SelfProclaimedCredential = {"BasicProfileCredential"};
            VerifiableCredential vc = issuer.issueFor(did)//被发布的did
                    .id("outPut")//唯一标识一个VerifiableCredential 相同会覆盖
                    .type(SelfProclaimedCredential)
                    .expirationDate(expires)
                    .properties(props)
                    .seal(pwd);
            didStore.storeCredential(vc);
            return true;

        } catch (DIDException e) {
            ToastUtils.showShort(e.getMessage());
            e.printStackTrace();
        }

        return false;
    }

    public VerifiableCredential getCredentialFromJson(String fromJson) {

        try {
            VerifiableCredential vcFrom = VerifiableCredential.fromJson(fromJson);//结果
            return vcFrom;
        } catch (Exception e) {
            ToastUtils.showShort(e.getMessage());
            e.printStackTrace();
        }

        return null;
    }

    public void setDIDDocument(Date expires, String pwd, String name) {

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
        } catch (DIDException e) {
            ToastUtils.showShort(e.getMessage());
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


    public BaseEntity forceDIDResolve(String didString) {
        try {
            if (TextUtils.isEmpty(didString)) {
                return new CommmonObjEntity(SUCCESSCODE, null);
            }
            return new CommmonObjEntity(SUCCESSCODE, new DID(didString).resolve(true));
        } catch (DIDException e) {
            ToastUtils.showShort(e.getMessage());
            return exceptionProcess(e, "DIDResolve");

        }
    }

    /**
     * resolve() resolve(fdlse)先在didcash查没有或者过期链上查
     *
     * @param didString
     * @return
     */
    public BaseEntity DIDResolveWithTip(String didString) {
        try {
            if (TextUtils.isEmpty(didString)) {
                return new CommmonObjEntity(SUCCESSCODE, null);
            }
            DID did = new DID(didString);
            return new CommmonObjEntity(SUCCESSCODE, did.resolve());
        } catch (DIDException e) {
            ToastUtils.showShort(e.getMessage());
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
            ToastUtils.showShort(e.getMessage());
            return exceptionProcess(e, "DIDPublish");

        }
    }

    private BaseEntity exceptionProcess(DIDException e, String methodName) {
        e.printStackTrace();
        String message = "method:" + methodName + "\nwalletId:" + walletId + "\nException:" + e.getClass().getName() +
                "\nmes:" + e.getMessage();
        Log.i(methodName, message);
        return new CommonEntity(DIDSDKEXCEPTIOM, e.getClass().getName() + e.getMessage());

    }

}
