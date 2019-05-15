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
        if (newVersion == 2 && oldVersion == 1) {
            uodata1to2(oldVersion, schema);

        } /*else if (newVersion == 3 && oldVersion == 2) {
            updata2to3(oldVersion, schema);
        } else if (newVersion == 3 && oldVersion == 1) {
            uodata1to2(oldVersion, schema);
            updata2to3(oldVersion, schema);
        }*/
    }

    private void uodata1to2(long oldVersion, RealmSchema schema) {
        schema.get("SubWallet").addField("progress", int.class).transform(new RealmObjectSchema.Function() {
            @Override
            public void apply(DynamicRealmObject obj) {
                obj.set("progress", "0");//为id设置值
            }
        });
    }

    private void uodata2to3(long oldVersion, RealmSchema schema) {
        RealmObjectSchema personSchema = schema.get("User");
        //新增@Required的id
        personSchema.addField("areaCode", String.class).addField("hasWealthPwd", int.class)
                .transform(new RealmObjectSchema.Function() {
                    @Override
                    public void apply(DynamicRealmObject obj) {
                        obj.set("areaCode", "0086");//为id设置值
                        obj.set("hasWealthPwd", 0);//为hasWealthPwd设置值
                    }
                });
        // oldVersion++;
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
