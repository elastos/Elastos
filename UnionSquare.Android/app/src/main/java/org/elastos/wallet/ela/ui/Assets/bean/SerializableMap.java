package org.elastos.wallet.ela.ui.Assets.bean;

import org.elastos.wallet.ela.db.table.SubWallet;

import java.io.Serializable;
import java.util.List;
import java.util.Map;

public class SerializableMap implements Serializable {


    private Map<String, List<SubWallet>> map;


    public Map<String, List<SubWallet>> getMap() {
        return map;
    }


    public void setMap(Map<String, List<SubWallet>> map) {
        this.map = map;
    }
}
