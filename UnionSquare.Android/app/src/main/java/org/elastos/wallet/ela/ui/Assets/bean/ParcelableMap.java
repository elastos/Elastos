package org.elastos.wallet.ela.ui.Assets.bean;

import android.os.Parcel;
import android.os.Parcelable;

import org.elastos.wallet.ela.db.table.SubWallet;

import java.io.Serializable;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ParcelableMap implements Parcelable {


    private Map<Integer, String > map;


    public Map<Integer, String> getMap() {
        return map;
    }

    public void setMap(Map<Integer, String> map) {
        this.map = map;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(this.map.size());
        for (Map.Entry<Integer, String> entry : this.map.entrySet()) {
            dest.writeValue(entry.getKey());
            dest.writeString(entry.getValue());
        }
    }

    public ParcelableMap() {
    }

    protected ParcelableMap(Parcel in) {
        int mapSize = in.readInt();
        this.map = new HashMap<Integer, String>(mapSize);
        for (int i = 0; i < mapSize; i++) {
            Integer key = (Integer) in.readValue(Integer.class.getClassLoader());
            String value = in.readString();
            this.map.put(key, value);
        }
    }

    public static final Parcelable.Creator<ParcelableMap> CREATOR = new Parcelable.Creator<ParcelableMap>() {
        @Override
        public ParcelableMap createFromParcel(Parcel source) {
            return new ParcelableMap(source);
        }

        @Override
        public ParcelableMap[] newArray(int size) {
            return new ParcelableMap[size];
        }
    };
}
