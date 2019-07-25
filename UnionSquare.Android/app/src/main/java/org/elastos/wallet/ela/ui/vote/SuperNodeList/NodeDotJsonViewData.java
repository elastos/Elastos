package org.elastos.wallet.ela.ui.vote.SuperNodeList;

import android.widget.ImageView;

import io.reactivex.disposables.Disposable;

public abstract class NodeDotJsonViewData {

    public  void onGetNodeDotJsonData(NodeInfoBean t,String url){};
    public  void onGetNodeDotJsonData(ImageView iv,NodeInfoBean t, String url){};
   public void onError(String url){};
   public void onSubscribe(String url, Disposable d){};
}
