package org.elastos.wallet.ela.utils;

import android.app.Dialog;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.os.Handler;
import android.support.annotation.LayoutRes;
import android.support.annotation.NonNull;
import android.text.Html;
import android.text.TextUtils;
import android.view.Display;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.view.animation.LinearInterpolator;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.NumberPicker;
import android.widget.PopupWindow;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.utils.listener.NewWarmPromptListener;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener2;
import org.elastos.wallet.ela.utils.widget.TextConfigNumberPicker;

import java.lang.reflect.Field;

import javax.inject.Inject;


public class DialogUtil {

    private PopupWindow popupWindow;

    @Inject
    public DialogUtil() {
    }

    private static Dialog httpialog = null;//全局的滚动条

    public synchronized Dialog getHttpDialog(Context context, String msg) {
       /* if (httpialog != null) {
            return httpialog;
        }*/
        View v = LayoutInflater.from(context).inflate(R.layout.loading_dialog, null);// 得到加载view
        LinearLayout layout = v.findViewById(R.id.dialog_view);// 加载布局
        ImageView spaceshipImage = v.findViewById(R.id.img);
        TextView tipTextView = v.findViewById(R.id.tipTextView);// 提示文字
        Animation hyperspaceJumpAnimation = AnimationUtils.loadAnimation(
                context, R.anim.load_animation);
        hyperspaceJumpAnimation.setInterpolator(new LinearInterpolator());//匀速插值器

        spaceshipImage.startAnimation(hyperspaceJumpAnimation);
        tipTextView.setText(msg);// 设置加载信息
        Dialog loadingDialog = new Dialog(context, R.style.loading_dialog);
        loadingDialog.setCancelable(true);// 可以用“返回键”取消
        loadingDialog.setCanceledOnTouchOutside(false);
        loadingDialog.setContentView(layout, new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.MATCH_PARENT));// 设置布局
       // httpialog = loadingDialog;
        return loadingDialog;
    }

    public static synchronized Dialog getHttpialog() {
        return httpialog;
    }

    @NonNull
    private Dialog getDialogs(BaseActivity activity, @LayoutRes int id) {
        Dialog dialog = new Dialog(activity);
        dialog.setContentView(id);
        dialog.setCanceledOnTouchOutside(false);
        dialog.setCancelable(false);
        return dialog;
    }

    public static synchronized void setHttpialogNull() {
        httpialog = null;
    }

    @NonNull
    private Dialog getDialogs1(BaseActivity activity, @LayoutRes int id) {
        Dialog dialog = new Dialog(activity);
        dialog.setContentView(id);
        dialog.setCanceledOnTouchOutside(true);
        dialog.setCancelable(true);
        return dialog;
    }

    public void dialogDismiss(Dialog dialog) {
        dialog.dismiss();

    }


    public void dialogDismiss() {
        if (dialog != null)
            dialog.dismiss();
    }

    private void setDialogAttributes(BaseActivity activity, Dialog dialog) {
        dialog.show();
        WindowManager m = activity.getWindowManager();
        Display d = m.getDefaultDisplay(); // 获取屏幕宽、高度
        WindowManager.LayoutParams params = dialog.getWindow().getAttributes();
//        params.height = (int) (d.getHeight() * 0.4); // 高度设置为屏幕的0.6，根据实际情况调整
        params.width = (int) (d.getWidth() * 0.8); // 根据实际情况调整
        dialog.getWindow().setAttributes(params);
    }

    /*提示1textview*/
    public void showWarmPrompt1(BaseActivity activity, String contentStr, WarmPromptListener listener) {
        Dialog dialog = getDialogs(activity, R.layout.dialog_settingtip1);

        ImageView ivCancel = dialog.findViewById(R.id.iv_cancel);
        TextView tvCancel = dialog.findViewById(R.id.tv_cancel);
        TextView contentTv = dialog.findViewById(R.id.tv_content);
        TextView tvSure = dialog.findViewById(R.id.tv_sure);

        if (contentStr != null) {
            contentTv.setText(Html.fromHtml(contentStr));
        }

        ivCancel.setOnClickListener(v -> dialogDismiss(dialog));
        tvCancel.setOnClickListener(v -> dialogDismiss(dialog));

        tvSure.setOnClickListener(v -> {
            dialogDismiss(dialog);
            listener.affireBtnClick(contentTv);
        });
        dialog.show();
    }

    Dialog dialog;

    /*提示1textview 有确定 取消 */
    public void showWarmPrompt2(BaseActivity activity, String contentStr, WarmPromptListener listener) {
        dialog = getDialogs(activity, R.layout.dialog_text_2);

        ImageView ivCancel = dialog.findViewById(R.id.iv_cancel);
        TextView tvCancel = dialog.findViewById(R.id.tv_cancel);
        TextView contentTv = dialog.findViewById(R.id.tv_content);
        TextView tvSure = dialog.findViewById(R.id.tv_sure);

        if (!TextUtils.isEmpty(contentStr)) {
            contentTv.setText(Html.fromHtml(contentStr));
        }

        ivCancel.setOnClickListener(v -> dialogDismiss(dialog));
        tvCancel.setOnClickListener(v -> dialogDismiss(dialog));

        tvSure.setOnClickListener(v -> {
            dialogDismiss(dialog);
            listener.affireBtnClick(contentTv);
        });
        dialog.show();
    }

    /*提示1textview 有确定 取消 */
    public void showWarmPrompt2(BaseActivity activity, String contentStr, NewWarmPromptListener listener) {
        dialog = getDialogs(activity, R.layout.dialog_text_2);

        ImageView ivCancel = dialog.findViewById(R.id.iv_cancel);
        TextView tvCancel = dialog.findViewById(R.id.tv_cancel);
        TextView contentTv = dialog.findViewById(R.id.tv_content);
        TextView tvSure = dialog.findViewById(R.id.tv_sure);


        if (!TextUtils.isEmpty(contentStr)) {
            contentTv.setText(Html.fromHtml(contentStr));
        }

        ivCancel.setOnClickListener(v -> {
            listener.onCancel(contentTv);
            dialogDismiss(dialog);
        });
        tvCancel.setOnClickListener(v -> {
            listener.onCancel(contentTv);
            dialogDismiss(dialog);
        });

        tvSure.setOnClickListener(v -> {
            dialogDismiss(dialog);
            listener.affireBtnClick(contentTv);
        });
        dialog.show();
    }

    /*提示imageview*/
    public void showImage(BaseActivity activity, Bitmap mBitmap) {
        Dialog dialog = getDialogs1(activity, R.layout.dialog_image);
        ImageView iv = dialog.findViewById(R.id.iv);
        iv.setImageBitmap(mBitmap);
        dialog.show();
    }

    /*input*/
    public Dialog showWarmPromptInput(BaseActivity activity, String title, String hint, WarmPromptListener listener) {
        dialog = getDialogs(activity, R.layout.dialog_input);

        TextView tvTitle = dialog.findViewById(R.id.tv_title);
        ImageView ivCancel = dialog.findViewById(R.id.iv_cancel);
        EditText etContent = dialog.findViewById(R.id.et_content);
        TextView tvCancel = dialog.findViewById(R.id.tv_cancel);
        TextView tvSure = dialog.findViewById(R.id.tv_sure);

        if (title != null) {
            tvTitle.setText(Html.fromHtml(title));
        }
        if (hint != null) {
            etContent.setHint(Html.fromHtml(hint));
        }

        ivCancel.setOnClickListener(v -> dialogDismiss(dialog));
        tvCancel.setOnClickListener(v -> dialogDismiss(dialog));

        tvSure.setOnClickListener(v -> {
            listener.affireBtnClick(etContent);
        });
        dialog.show();
        return dialog;
    }

    public Dialog showWarmPromptInput3(BaseActivity activity, String title, String hint, WarmPromptListener2 listener) {
        dialog = getDialogs(activity, R.layout.dialog_input3);

        TextView tvTitle = dialog.findViewById(R.id.tv_title);
        ImageView ivCancel = dialog.findViewById(R.id.iv_cancel);
        EditText etContent = dialog.findViewById(R.id.et_content);
        TextView tvCancel = dialog.findViewById(R.id.tv_cancel);
        TextView tvSure = dialog.findViewById(R.id.tv_sure);
        TextView tvnoSure = dialog.findViewById(R.id.tv_nosure);

        if (title != null) {
            tvTitle.setText(Html.fromHtml(title));
        }
        if (hint != null) {
            etContent.setHint(Html.fromHtml(hint));
        }

        tvnoSure.setOnClickListener(v -> {
            dialogDismiss(dialog);
            listener.noAffireBtnClick(v);
        });
        ivCancel.setOnClickListener(v -> dialogDismiss(dialog));
        tvCancel.setOnClickListener(v -> dialogDismiss(dialog));

        tvSure.setOnClickListener(v -> {
            listener.affireBtnClick(etContent);
        });
        dialog.show();
        return dialog;
    }
    public void showTransferSucess(BaseActivity activity) {
        Dialog dialog = new Dialog(activity, R.style.coustom_dialog);
        dialog.setContentView(R.layout.dialog_transfersuccess);
        setDialogAttributes(activity, dialog);
        Handler handler = new Handler();
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                if (dialog != null)
                    dialog.dismiss();
            }
        }, 2000);//3秒后执行Runnable中的run方法
        dialog.show();
    }

    public void showTransferSucess(BaseActivity activity, WarmPromptListener listener) {
        Dialog dialog = new Dialog(activity, R.style.coustom_dialog);
        dialog.setContentView(R.layout.dialog_transfersuccess);
        dialog.setCancelable(false);
        setDialogAttributes(activity, dialog);
        Handler handler = new Handler();
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                if (dialog != null)
                    dialog.dismiss();
                listener.affireBtnClick(null);
            }
        }, 2888);//3秒后执行Runnable中的run方法
        dialog.show();
    }

    public void showSelectNum(BaseActivity activity, WarmPromptListener listener) {
        Dialog dialog = new Dialog(activity);
        dialog.setContentView(R.layout.dialog_numberpicker);
        dialog.setCancelable(true);
        dialog.setCanceledOnTouchOutside(true);
        TextView tvSure = dialog.findViewById(R.id.tv_sure);
        TextConfigNumberPicker numberPicker = dialog.findViewById(R.id.np);
        tvSure.setOnClickListener(v -> {
            dialog.dismiss();
            listener.affireBtnClick(numberPicker);
        });

        //设置需要显示的内容数组
        // numberPicker.setDisplayedValues(numbers);
        //设置最大最小值
        numberPicker.setMinValue(2);
        numberPicker.setMaxValue(6);
        //设置默认的位置
        numberPicker.setValue(2);
        numberPicker.setWrapSelectorWheel(false);
        numberPicker.setDescendantFocusability(NumberPicker.FOCUS_BLOCK_DESCENDANTS);
        numberPicker.setMInputStyle(16f);
        numberPicker.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            @Override
            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
                picker.performClick();
            }
        });

        WindowManager m = activity.getWindowManager();
        Window window = dialog.getWindow();
        window.getDecorView().setPadding(0, 0, 0, 0);
        WindowManager.LayoutParams params = dialog.getWindow().getAttributes();
        params.width = WindowManager.LayoutParams.MATCH_PARENT;
        dialog.getWindow().setAttributes(params);
        window.getDecorView().setBackgroundResource(R.color.pickerbg);
        dialog.getWindow().setGravity(Gravity.BOTTOM);
        dialog.show();
    }

}
