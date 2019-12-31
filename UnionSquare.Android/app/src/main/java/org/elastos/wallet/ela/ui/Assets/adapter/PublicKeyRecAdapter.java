package org.elastos.wallet.ela.ui.Assets.adapter;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.google.gson.JsonArray;

import org.elastos.wallet.ela.utils.ScreenUtil;

public class PublicKeyRecAdapter extends RecyclerView.Adapter<PublicKeyRecAdapter.MyViewHolder> {


    private JsonArray datas;

    private Context context;


    public PublicKeyRecAdapter(Context context, JsonArray datas) {
        this.datas = datas;
        this.context = context;

    }


    @Override
    public MyViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        TextView textView = new TextView(context);
        textView.setTextSize(11);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        textView.setPadding(0, ScreenUtil.dp2px(context,15), 0,  ScreenUtil.dp2px(context,15));
        textView.setLayoutParams(params);
        return new MyViewHolder(textView);
    }

    @Override
    public void onBindViewHolder(MyViewHolder holder, final int position) {
        String data;
        try {
            data = datas.get(position).getAsString();
            ((TextView) (holder.itemView)).setText(data);
        } catch (Exception e) {
            e.printStackTrace();
        }


    }

    @Override
    public int getItemCount() {
        return datas.size();
    }


    class MyViewHolder extends RecyclerView.ViewHolder {
        MyViewHolder(TextView itemView) {
            super(itemView);
        }
    }

}