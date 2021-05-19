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
