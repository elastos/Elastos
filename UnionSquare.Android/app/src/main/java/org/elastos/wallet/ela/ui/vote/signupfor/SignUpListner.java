package org.elastos.wallet.ela.ui.vote.signupfor;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.SubscriberOnNextLisenner;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;

public class SignUpListner extends SubscriberOnNextLisenner {
    @Override
    protected void onNextLisenner(BaseEntity t) {
        SignUpViewData viewData = (SignUpViewData) getViewData();
        viewData.onIsVote(((CommmonStringEntity) t).getData());
    }
}
