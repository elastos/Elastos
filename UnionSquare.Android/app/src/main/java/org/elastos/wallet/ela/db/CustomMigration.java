package org.elastos.wallet.ela.db;

import io.realm.DynamicRealm;
import io.realm.DynamicRealmObject;
import io.realm.RealmMigration;
import io.realm.RealmObjectSchema;
import io.realm.RealmSchema;



/*数据库升级,更新迁移,类*/

public class CustomMigration implements RealmMigration {

    @Override
    public void migrate(DynamicRealm realm, long oldVersion, long newVersion) {
        RealmSchema schema = realm.getSchema();
        if (oldVersion == 1) {
            updata1to2(schema);
            oldVersion++;

        }
        if (oldVersion == 2) {
            uodata2to3(schema);
            oldVersion++;
        }
        if (oldVersion == 3) {
            uodata3to4(schema);
            oldVersion++;
        }
        if (oldVersion == 4) {
            uodata4to5(schema);
            // oldVersion++;
        }
        if (oldVersion == 5) {
            uodata5to6(schema);
            // oldVersion++;
        }
    }

    private void updata1to2(RealmSchema schema) {
        schema.get("SubWallet").addField("progress", int.class).transform(new RealmObjectSchema.Function() {
            @Override
            public void apply(DynamicRealmObject obj) {
                obj.set("progress", "0");//为id设置值
            }
        });
    }

    private void uodata2to3(RealmSchema schema) {
        RealmObjectSchema personSchema = schema.get("Wallet");
        //新增@Required的id
        personSchema.addField("type", int.class)
                .transform(new RealmObjectSchema.Function() {
                    @Override
                    public void apply(DynamicRealmObject obj) {
                        obj.set("type", 0);//默认普通单签
                    }
                });
        // oldVersion++;
    }

    private void uodata3to4(RealmSchema schema) {
        RealmObjectSchema personSchema = schema.get("Wallet");
        //新增@Required的id
        if (personSchema.hasField("privateKey")) {
            personSchema.removeField("privateKey").removeField("keyStore").removeField("mnemonic");//移除属性
        }
    }

    private void uodata4to5(RealmSchema schema) {
        RealmObjectSchema subWallet = schema.get("SubWallet");
        if (!subWallet.hasField("bytesPerSecond")) {
            subWallet.addField("bytesPerSecond", long.class)
                    .addField("downloadPeer", String.class);

        }

    }

    private void uodata5to6(RealmSchema schema) {
        RealmObjectSchema personSchema = schema.get("Wallet");
        //新增@Required的id
        personSchema.addField("did", String.class);

    }

    @Override
    public int hashCode() {
        return 37;
    }

    @Override
    public boolean equals(Object o) {
        return (o instanceof CustomMigration);
    }

}
