package org.elastos.wallet.ela.ui.vote.ElectoralAffairs;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;

public class VotelistbeanListener extends SubscriberOnNextLisenner {


    @Override
    protected void onNextLisenner(BaseEntity t) {
        VotelistViewData viewData = (VotelistViewData) getViewData();
        viewData.onGetVoteList((VoteListBean) t);
    }


}
