package org.elastos.wallet.ela.utils;

import android.content.Context;

import org.elastos.wallet.ela.ui.vote.SuperNodeList.NodeDotJsonViewData;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.SuperNodeListPresenter;

public class GetDynanicUrl {
    public static void getData(String url, Context context, NodeDotJsonViewData nodeDotJsonViewData) {

        if (url.endsWith("bpinfo.json")) {

        } else if (url.endsWith("/")) {
            url += "bpinfo.json";
        } else {
            url += "/bpinfo.json";
        }
        new SuperNodeListPresenter().getUrlJson(context, url, nodeDotJsonViewData);
    }
}
