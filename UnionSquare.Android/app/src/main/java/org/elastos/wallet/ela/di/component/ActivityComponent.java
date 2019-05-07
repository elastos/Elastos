package org.elastos.wallet.ela.di.component;

import android.app.Activity;
import android.content.Context;

import org.elastos.wallet.ela.di.moudule.ActivityModule;
import org.elastos.wallet.ela.di.scope.ContextLife;
import org.elastos.wallet.ela.di.scope.PerActivity;


import dagger.Component;

/**
 * author: fangxiaogang
 * date: 2018/9/3
 */

@PerActivity
@Component(dependencies = ApplicationComponent.class, modules = ActivityModule.class)
public interface ActivityComponent {

    @ContextLife("Activity")
    Context getActivityContext();

    @ContextLife("Application")
    Context getApplicationContext();

    Activity getActivity();


}