package org.elastos.wallet.ela.ui.vote.SuperNodeList;

import android.widget.ImageView;

import org.elastos.wallet.ela.bean.ImageBean;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

import io.reactivex.disposables.Disposable;

public abstract class NodeDotJsonViewData {

    public  void onGetNodeDotJsonData(NodeInfoBean t,String url){};
    public  void onGetNodeDotJsonData(ImageView iv,BaseEntity t, String url){};
    public  void onGetNodeDotJsonData(ImageView iv,NodeInfoBean t, String url){};
   public void onError(String url){};
   public void onSubscribe(String url, Disposable d){};
   public void onGetImage(ImageView iv,String url,ImageBean urlbean){};
}
