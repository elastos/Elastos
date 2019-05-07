package org.elastos.wallet.ela.rxjavahelp;



/**
 * Created by wangdongfeng on 2018/4/11.
 */

public abstract class SubscriberOnNextLisenner<T1 extends BaseEntity,T extends BaseViewData> {
    private T viewData;
    private Object obj;
    protected abstract void onNextLisenner(T1 t);

    protected void setViewData(T viewData){
        this.viewData = viewData;
    }

    public T getViewData() {
        return viewData;
    }

    public Object getObj() {
        return obj;
    }

    public void setObj(Object obj) {
        this.obj = obj;
    }
}
