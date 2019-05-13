package org.elastos.wallet.ela.db;

import android.support.annotation.NonNull;

import org.elastos.wallet.ela.db.listener.RealmTransactionAbs;
import org.elastos.wallet.ela.db.table.Contact;
import org.elastos.wallet.ela.db.table.SubWallet;
import org.elastos.wallet.ela.db.table.Wallet;

import java.util.ArrayList;
import java.util.List;

import javax.inject.Inject;

import io.realm.Realm;
import io.realm.RealmConfiguration;
import io.realm.RealmResults;

public class RealmUtil {

    public static final long DB_VERSION_CODE =2;//当前数据库版本号
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


    public void updateWalletKeystore(String walletAddr, String keyStore) {
        //更新钱包keystore
        Realm realm = getInstanceRealm();
        realm.beginTransaction();
        Wallet wallet = realm.where(Wallet.class).equalTo("walletAddr", walletAddr.toLowerCase()).findFirst();
        wallet.setKeyStore(keyStore);
        realm.commitTransaction();
        closeRealm(realm);
    }

    public void upDataWalletName(String walletId, String name) {
        Realm realm = getInstanceRealm();
        Wallet wallet = realm.where(Wallet.class).equalTo("walletId", walletId).findFirst();
        realm.beginTransaction();
        wallet.setWalletName(name);
        realm.commitTransaction();
        closeRealm(realm);
    }


    public void deleteAllWallet() {
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
    }


    public void deleteMnemonic(@NonNull String walletAddress, String memberId) {
        Realm realm = getInstanceRealm();
        Wallet wallet = realm.where(Wallet.class)
                .equalTo("memberId", memberId).equalTo("walletAddr", walletAddress.toLowerCase())
                .findFirst();
        if (wallet != null) {
            realm.beginTransaction();
            wallet.setMnemonic(null);
            realm.commitTransaction();
        }
        realm.close();
    }

    private List<Wallet> getWalletList(RealmResults<Wallet> list) {
        List<Wallet> wallets = new ArrayList<>();
        for (Wallet item : list) {
            Wallet wallet = new Wallet();
            wallet.setWalletData(item);
            wallets.add(wallet);
        }
        return wallets;
    }
    public void sync(ArrayList<String> ids) {
        Realm realm = getInstanceRealm();
        RealmResults<Wallet> list = realm.where(Wallet.class)
                .findAll();
        for (Wallet wallet : list) {
            String walletId = wallet.getWalletId();
            if (!ids.contains(walletId)) {
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

    /***********子钱包*********************************/
    /*更新钱包的名称 密码等信息*/
    public void updateSubWalletDetial(@NonNull SubWallet subWallet, RealmTransactionAbs listener) {
        Realm realm = getInstanceRealm();
        realm.beginTransaction();
        subWallet.setWallletId(subWallet.getBelongId() + subWallet.getChainId());
        realm.copyToRealmOrUpdate(subWallet);
        realm.commitTransaction();
        realm.close();
        listener.onSuccess();
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

    public void updateWalletSyncTime(String belongId, String chainId, String syncTime, RealmTransactionAbs listener) {
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
    }
    public SubWallet querySubWallet(String belongId, String chainId) {
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
        }
        SubWallet tempSubWallet = new SubWallet();
        tempSubWallet.setSubWallet(subWallet);
        closeRealm(realm);
        return tempSubWallet;
    }
    public String querySubWalletSyncTime(String belongId, String chainId) {
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
    }


}
