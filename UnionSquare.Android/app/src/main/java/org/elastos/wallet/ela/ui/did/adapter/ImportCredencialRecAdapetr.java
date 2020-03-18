package org.elastos.wallet.ela.ui.did.adapter;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;

import java.io.File;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * 资产页面的rv
 */

public class ImportCredencialRecAdapetr extends RecyclerView.Adapter<ImportCredencialRecAdapetr.ViewHolderParent> {


    private Context context;

    private File[] files;
    private int chosePosition = -1;

    public ImportCredencialRecAdapetr(Context context, File[] files) {
        this.context = context;

        this.files = files;


    }

    public int getChosePosition() {
        return chosePosition;
    }

    public void setChosePosition(int chosePosition) {
        this.chosePosition = chosePosition;
    }

    @Override
    public ViewHolderParent onCreateViewHolder(ViewGroup parent, int viewType) {
        return new ViewHolderParent(LayoutInflater.from(parent.getContext()).inflate(R.layout.item_credencial_import, parent, false));

    }

    @Override
    public void onBindViewHolder(ViewHolderParent holder, int position) {
        File file = files[position];
        if (position == chosePosition) {
            holder.iv.setImageResource(R.mipmap.asset_adding_select);
        } else {
            holder.iv.setImageDrawable(null);
        }
        holder.tv.setText(file.getName());
        holder.itemView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                chosePosition = position;
                notifyDataSetChanged();
            }
        });

    }


    @Override
    public int getItemCount() {
        return files.length;
    }

    public static class ViewHolderParent extends RecyclerView.ViewHolder {

        @BindView(R.id.tv)
        TextView tv;
        @BindView(R.id.iv)
        ImageView iv;

        ViewHolderParent(View view) {
            super(view);
            ButterKnife.bind(this, view);
        }
    }


}
