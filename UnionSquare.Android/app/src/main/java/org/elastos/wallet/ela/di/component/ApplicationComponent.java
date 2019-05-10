package org.elastos.wallet.ela.di.component;

import android.content.Context;

import org.elastos.wallet.ela.di.moudule.ApplicationModule;
import org.elastos.wallet.ela.di.scope.ContextLife;
import org.elastos.wallet.ela.di.scope.PerApp;

import dagger.Component;


/**
 * Created by lw on 2017/1/19.
 */
@PerApp
@Component(modules = ApplicationModule.class)
public interface ApplicationComponent {
    @ContextLife("Application")
    Context getApplication();
}