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

package org.elastos.wallet.ela.db;

import android.support.annotation.NonNull;

import com.alibaba.fastjson.JSON;

import org.elastos.wallet.core.MasterWallet;
import org.elastos.wallet.ela.db.listener.RealmTransactionAbs;
import org.elastos.wallet.ela.db.table.Contact;
import org.elastos.wallet.ela.db.table.SubWallet;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.bean.SubWalletBasicInfo;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import javax.inject.Inject;

import io.realm.Realm;
import io.realm.RealmConfiguration;
import io.realm.RealmResults;

public class RealmUtil {

    public static final long DB_VERSION_CODE = 6;//当前数据库版本号
    public static final String DB_NAME = "DB";//BuildConfig

    @Inject
    public RealmUtil() {
    }

    /*数据库的相关配置*/
    public RealmConfiguration initRealmConfig() {
        RealmConfiguration config = new RealmConfiguration.Builder()
                .name(DB_NAME)
                .schemaVersion(DB_VERSION_CODE)
                .migration(new CustomMigration())
                //.modules(new MySchemaModule())
                .build();
        return config;
    }

    /*获取数据库实例*/
    public Realm getInstanceRealm() {
        return Realm.getInstance(initRealmConfig());
    }

    public void closeRealm(Realm realm) {
        if (!realm.isClosed()) {
            realm.close();
        }
    }

    public void insertContact(Contact contact, RealmTransactionAbs listener) {
        Realm realm = getInstanceRealm();
        realm.beginTransaction();
        realm.copyToRealmOrUpdate(contact);
        realm.commitTransaction();
        closeRealm(realm);
        listener.onSuccess();
    }

    public List<Contact> queryAllContact() {
        Realm realm = getInstanceRealm();
        //查询默认钱包
        RealmResults<Contact> list = realm.where(Contact.class)
                .findAll();
        List<Contact> contacts = getContactList(list);
        closeRealm(realm);
        return contacts;
    }

    private List<Contact> getContactList(RealmResults<Contact> list) {
        List<Contact> contacts = new ArrayList<>();
        for (Contact item : list) {
            Contact contact = new Contact();
            contact.setContact(item);
            contacts.add(contact);
        }

        return contacts;
    }

    public void deleteContact(String id) {
        Realm realm = getInstanceRealm();
        final Contact contact = realm.where(Contact.class)
                .equalTo("id", id)
                .findFirst();

        realm.executeTransaction(new Realm.Transaction() {
            @Override
            public void execute(Realm realm) {
                contact.deleteFromRealm();
            }
        });
        closeRealm(realm);
    }

    /*******************************钱包****************************************/
    /*更新钱包的名称 密码等信息*/
    public void updateWalletDetial(@NonNull Wallet wallet) {
        Realm realm = getInstanceRealm();
        realm.beginTransaction();
        realm.copyToRealmOrUpdate(wallet);
        realm.commitTransaction();
        realm.close();

    }

    public Wallet updateWalletDetial(String walletName, String masterWalletID, String basecInfo) {
        SubWalletBasicInfo walletBasicInfo = JSON.parseObject(basecInfo, SubWalletBasicInfo.class);
        SubWalletBasicInfo.InfoBean.AccountBean account = walletBasicInfo.getInfo().getAccount();

        Wallet masterWallet = new Wallet();
        masterWallet.setWalletName(walletName);
        masterWallet.setWalletId(masterWalletID);
        masterWallet.setSingleAddress(account.isSingleAddress());
        masterWallet.setType(getType(account));
        updateWalletDetial(masterWallet);
        return masterWallet;
    }

    public Wallet updateWalletDetial(String walletName, String masterWalletID, String basecInfo, String did) {
        SubWalletBasicInfo walletBasicInfo = JSON.parseObject(basecInfo, SubWalletBasicInfo.class);
        SubWalletBasicInfo.InfoBean.AccountBean account = walletBasicInfo.getInfo().getAccount();

        Wallet masterWallet = new Wallet();
        masterWallet.setWalletName(walletName);
        masterWallet.setWalletId(masterWalletID);
        masterWallet.setDid(did);
        masterWallet.setSingleAddress(account.isSingleAddress());
        masterWallet.setType(getType(account));
        updateWalletDetial(masterWallet);
        return masterWallet;
    }

    public boolean queryUserWalletExist(String walletId) {
        Realm realm = getInstanceRealm();

        RealmResults<Wallet> list = realm.where(Wallet.class)
                .equalTo("walletId", walletId)
                .findAll();
        if (list == null || list.size() == 0) {
            closeRealm(realm);
            return true;
        }
        closeRealm(realm);
        return false;
    }

    /*删除制定钱包包括名下子钱包*/
    public void deleteWallet(String walletId) {
        Realm realm = getInstanceRealm();
        Wallet wallet = realm.where(Wallet.class)
                .equalTo("walletId", walletId)
                .findFirst();

        realm.executeTransaction(new Realm.Transaction() {
            @Override
            public void execute(Realm realm) {
                wallet.deleteFromRealm();
            }
        });

        RealmResults<SubWallet> list = realm.where(SubWallet.class)
                .equalTo("belongId", walletId)
                .findAll();

        realm.executeTransaction(new Realm.Transaction() {
            @Override
            public void execute(Realm realm) {
                list.deleteAllFromRealm();
            }
        });
        closeRealm(realm);
    }

    /*查询默认钱包*/
    public Wallet queryDefauleWallet() {
        Realm realm = getInstanceRealm();

        Wallet wallet = realm.where(Wallet.class)
                .equalTo("isDefault", true)
                .findFirst();

        Wallet wallet1 = new Wallet();
        if (wallet != null) {
            wallet1.setWalletData(wallet);
        } else {
            RealmResults<Wallet> list = realm.where(Wallet.class)
                    .findAll();
            if (list.size() > 0) {
                wallet1.setWalletData(list.get(0));
            } else {
                wallet1 = null;
            }
        }
        closeRealm(realm);
        return wallet1;
    }


    /*查询所有钱包*/
    public List<Wallet> queryUserAllWallet() {
        Realm realm = getInstanceRealm();
        RealmResults<Wallet> list = realm.where(Wallet.class)
                .findAll();

        List<Wallet> wallets = getWalletList(list);
        closeRealm(realm);
        return wallets;
    }

    /*查询所有钱包*/
    public List<Wallet> queryTypeUserAllWallet(int type) {
        Realm realm = getInstanceRealm();
        RealmResults<Wallet> list = realm.where(Wallet.class).equalTo("type", type)
                .findAll();

        List<Wallet> wallets = getWalletList(list);
        closeRealm(realm);
        return wallets;
    }

    public List<Wallet> queryUnReadOnlyUserAllWallet() {
        Realm realm = getInstanceRealm();
        RealmResults<Wallet> list = realm.where(Wallet.class).beginGroup()
                .equalTo("type", 0).or().equalTo("type", 2).endGroup()
                .findAll();

        List<Wallet> wallets = getWalletList(list);
        closeRealm(realm);
        return wallets;
    }

    public Wallet queryUserWallet(String walletId) {
        Realm realm = getInstanceRealm();
        Wallet wallet = realm.where(Wallet.class)
                .equalTo("walletId", walletId)
                .findFirst();


        if (wallet != null) {
            Wallet wallet1 = new Wallet();
            wallet1.setWalletData(wallet);
            closeRealm(realm);
            return wallet1;
        }
        closeRealm(realm);
        return null;
    }

    /*更新钱包的默认状态*/
    public void updateWalletDefault(String walletId, RealmTransactionAbs listener) {
        Realm realm = getInstanceRealm();
        try {
            realm.beginTransaction();
            RealmResults<Wallet> wallets = realm.where(Wallet.class)
                    .equalTo("isDefault", true)
                    .findAll();

            if (wallets != null && wallets.size() > 0) {
                for (Wallet walletDB1 : wallets) {
                    walletDB1.setDefault(false);
                }
            }

            Wallet walletDB2 = realm.where(Wallet.class)
                    .equalTo("walletId", walletId)
                    .findFirst();
            walletDB2.setDefault(true);

            realm.commitTransaction();
            closeRealm(realm);
            listener.onSuccess();
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            closeRealm(realm);
        }
    }


    public void upDataWalletName(String walletId, String name) {
        Realm realm = getInstanceRealm();
        Wallet wallet = realm.where(Wallet.class).equalTo("walletId", walletId).findFirst();
        realm.beginTransaction();
        wallet.setWalletName(name);
        realm.commitTransaction();
        closeRealm(realm);
    }

    public void upDataWalletDid(String walletId, String didString) {
        Realm realm = getInstanceRealm();
        Wallet wallet = realm.where(Wallet.class).equalTo("walletId", walletId).findFirst();
        realm.beginTransaction();
        wallet.setDid(didString);
        realm.commitTransaction();
        closeRealm(realm);
    }

/*    public void deleteAllWallet() {
        Realm realm = getInstanceRealm();
        try {
            realm.beginTransaction();
            RealmResults<Wallet> wallets = realm.where(Wallet.class).findAll();
            wallets.deleteAllFromRealm();
            realm.commitTransaction();
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            closeRealm(realm);
        }
    }*/


    private List<Wallet> getWalletList(RealmResults<Wallet> list) {
        List<Wallet> wallets = new ArrayList<>();
        for (Wallet item : list) {
            Wallet wallet = new Wallet();
            wallet.setWalletData(item);
            wallets.add(wallet);
        }
        return wallets;
    }

    public void sync(ArrayList<MasterWallet> masterWallets) {
        Realm realm = getInstanceRealm();
        RealmResults<Wallet> list = realm.where(Wallet.class)
                .findAll();
        for (MasterWallet masterWallet : masterWallets) {
            String id = masterWallet.GetID();
            boolean flag = false;
            for (Wallet wallet : list) {
                String walletId = wallet.getWalletId();
                if (id.equals(walletId)) {
                    flag = true;
                    break;
                }

            }
            if (!flag) {
                SubWalletBasicInfo.InfoBean.AccountBean account = JSON.parseObject(masterWallet.GetBasicInfo(), SubWalletBasicInfo.InfoBean.AccountBean.class);
                boolean singleAddress = account.isSingleAddress();
                int type = getType(account);
                Wallet newWallet = new Wallet();
                newWallet.setWalletId(id);
                newWallet.setType(type);
                newWallet.setWalletName("Unknown");
                newWallet.setSingleAddress(singleAddress);
                realm.beginTransaction();
                realm.copyToRealmOrUpdate(newWallet);
                realm.commitTransaction();


            }

        }
        for (Wallet wallet : list) {
            boolean flag = false;
            String walletId = wallet.getWalletId();
            for (MasterWallet masterWallet : masterWallets) {
                String id = masterWallet.GetID();
                if (id.equals(walletId)) {
                    flag = true;
                    break;
                }
            }
            if (!flag) {
                realm.executeTransaction(new Realm.Transaction() {
                    @Override
                    public void execute(Realm realm) {
                        wallet.deleteFromRealm();
                    }
                });

                RealmResults<SubWallet> subList = realm.where(SubWallet.class)
                        .equalTo("belongId", walletId)
                        .findAll();

                realm.executeTransaction(new Realm.Transaction() {
                    @Override
                    public void execute(Realm realm) {
                        subList.deleteAllFromRealm();
                    }
                });
            }
        }
        closeRealm(realm);
    }

    private int getType(SubWalletBasicInfo.InfoBean.AccountBean account) {
        boolean Readonly = account.isReadonly();
        int N = account.getN();
        int M = account.getM();
        //0 普通单签 1单签只读 2普通多签 3多签只读
        if (N > 1) {
            //多签
            if (Readonly) {
                return 3;
            } else {
                return 2;
            }

        } else {
            //单签
            if (Readonly) {
                return 1;
            } else {
                return 0;
            }

        }

    }

    /***********子钱包*********************************/
    /*更新钱包的名称 密码等信息*/
    private void updateSubWalletDetial(@NonNull SubWallet subWallet, RealmTransactionAbs listener) {
        Realm realm = getInstanceRealm();
        realm.beginTransaction();
        subWallet.setWallletId(subWallet.getBelongId() + subWallet.getChainId());
        realm.copyToRealmOrUpdate(subWallet);
        realm.commitTransaction();
        realm.close();
        listener.onSuccess();
    }

    public void updateSubWalletDetial(String masterWalletID, String baseInfo, RealmTransactionAbs listener) {

        // 注意这里是SubWalletBasicInfo不是MasterWalletBasicInfo
        SubWalletBasicInfo walletBasicInfo = JSON.parseObject(baseInfo, SubWalletBasicInfo.class);
        SubWallet subWallet = new SubWallet();
        subWallet.setBelongId(masterWalletID);
        subWallet.setChainId(walletBasicInfo.getChainID());
        updateSubWalletDetial(subWallet, listener);

    }

    public void updateSubWalletDetial(@NonNull Map<String, List<org.elastos.wallet.ela.db.table.SubWallet>> listMap) {
        Realm realm = getInstanceRealm();
        realm.beginTransaction();
        for (Map.Entry<String, List<SubWallet>> entry : listMap.entrySet()) {
            List<SubWallet> assetList = entry.getValue();
            for (SubWallet subWallet : assetList) {
                subWallet.setFiled1("Connecting");
                subWallet.setSyncTime("- -");
                subWallet.setProgress(0);
                subWallet.setWallletId(subWallet.getBelongId() + subWallet.getChainId());
                subWallet.setFiled2("false");
                subWallet.setBytesPerSecond(0);
                subWallet.setDownloadPeer(null);
                realm.copyToRealmOrUpdate(subWallet);
            }
        }
        realm.commitTransaction();
        realm.close();
    }
    /*

     */
    /*查询所有钱包*//*
//不可靠  需要同步子钱包 本地不一定有
    public List<SubWallet> queryAllSubWallet(String belongId) {
        Realm realm = getInstanceRealm();
        RealmResults<SubWallet> list = realm.where(SubWallet.class).equalTo("belongId", belongId)
                .findAll();

        List<SubWallet> wallets = getSubWalletList(list);
        closeRealm(realm);
        return wallets;
    }
*/

    private List<SubWallet> getSubWalletList(RealmResults<SubWallet> list) {
        List<SubWallet> wallets = new ArrayList<>();
        for (SubWallet item : list) {
            SubWallet wallet = new SubWallet();
            wallet.setSubWallet(item);
            wallets.add(wallet);
        }
        return wallets;
    }
    //*删除制定钱包包括名下所有子钱包*//*

    public void deleteSubWallet(String belongId) {
        Realm realm = getInstanceRealm();
        RealmResults<SubWallet> list = realm.where(SubWallet.class)
                .equalTo("belongId", belongId)
                .findAll();

        realm.executeTransaction(new Realm.Transaction() {
            @Override
            public void execute(Realm realm) {
                list.deleteAllFromRealm();
            }
        });
        closeRealm(realm);
    }

    public void deleteSubWallet(String belongId, String chainId) {
        Realm realm = getInstanceRealm();
        RealmResults<SubWallet> list = realm.where(SubWallet.class)
                .equalTo("belongId", belongId).equalTo("chainId", chainId)
                .findAll();

        realm.executeTransaction(new Realm.Transaction() {
            @Override
            public void execute(Realm realm) {
                list.deleteAllFromRealm();
            }
        });
        closeRealm(realm);
    }

   /* public void updateWalletSyncTime(String belongId, String chainId, String syncTime, RealmTransactionAbs listener) {
        //更新钱包keystore
        Realm realm = getInstanceRealm();
        realm.beginTransaction();
        SubWallet subWallet = realm.where(SubWallet.class)
                .equalTo("belongId", belongId).equalTo("chainId", chainId)
                .findFirst();
        if (subWallet == null) {
            closeRealm(realm);
            return;
        }
        subWallet.setSyncTime(syncTime);
        realm.commitTransaction();
        closeRealm(realm);
        listener.onSuccess();
    }*/

    public SubWallet querySubWallet(String belongId, String chainId) {
        Realm realm = getInstanceRealm();
        SubWallet subWallet = realm.where(SubWallet.class)
                .equalTo("belongId", belongId).equalTo("chainId", chainId)
                .findFirst();
        if (subWallet == null) {
            subWallet = new SubWallet();
            subWallet.setBelongId(belongId);
            subWallet.setChainId(chainId);
            subWallet.setSyncTime("- -");
            subWallet.setWallletId(subWallet.getBelongId() + subWallet.getChainId());
            realm.beginTransaction();
            realm.copyToRealmOrUpdate(subWallet);
            realm.commitTransaction();
        }
        SubWallet tempSubWallet = new SubWallet();
        tempSubWallet.setSubWallet(subWallet);
        closeRealm(realm);
        return tempSubWallet;
    }

   /* public String querySubWalletSyncTime(String belongId, String chainId) {
        Realm realm = getInstanceRealm();
        SubWallet subWallet = realm.where(SubWallet.class)
                .equalTo("belongId", belongId).equalTo("chainId", chainId)
                .findFirst();
        if (subWallet == null) {
            subWallet = new SubWallet();
            subWallet.setBelongId(belongId);
            subWallet.setChainId(chainId);
            subWallet.setWallletId(subWallet.getBelongId() + subWallet.getChainId());
            realm.beginTransaction();
            realm.copyToRealmOrUpdate(subWallet);
            realm.commitTransaction();
            closeRealm(realm);
            return null;
        }
        String syncTime = subWallet.getSyncTime();
        closeRealm(realm);
        return syncTime;
    }*/


}
