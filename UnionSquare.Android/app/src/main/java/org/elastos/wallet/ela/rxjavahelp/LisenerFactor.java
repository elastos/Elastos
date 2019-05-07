package org.elastos.wallet.ela.rxjavahelp;

/**
 * Created by wangdongfeng on 2018/4/11.
 */

public class LisenerFactor<T>{

    public static <T extends SubscriberOnNextLisenner> T create(
            Class<? extends SubscriberOnNextLisenner> listener,Object parameter){
        try {
            T listener1 = (T) listener.newInstance();
            listener1.setObj(parameter);
            return listener1;
        } catch (InstantiationException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
        return null;
    }

    public static <T extends SubscriberOnNextLisenner> T create(Class<? extends SubscriberOnNextLisenner> listener){
        try {
            return (T) listener.newInstance();
        } catch (InstantiationException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
        return null;
    }

}
