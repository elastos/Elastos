package org.elastos.wallet.ela.utils.listener;

/**
 * Created by wangdongfeng on 2018/4/16.
 */

public interface TopPopupClickListener extends CommonTopPopupClickListener{

    void topPopupClick(TopPopupClickEnum clickEnum);


    enum TopPopupClickEnum {
        LEFT,RIGHT
    }
}
