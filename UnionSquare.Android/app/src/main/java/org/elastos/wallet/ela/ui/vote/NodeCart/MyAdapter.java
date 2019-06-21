package org.elastos.wallet.ela.ui.vote.NodeCart;

import android.content.Context;
import android.graphics.Bitmap;
import android.support.v7.widget.AppCompatCheckBox;
import android.support.v7.widget.AppCompatImageView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.NodeDotJsonViewData;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.NodeInfoBean;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.GetDynanicUrl;
import org.elastos.wallet.ela.utils.GlideApp;
import org.elastos.wallet.ela.utils.GlideRequest;
import org.elastos.wallet.ela.utils.NumberiUtil;

import java.util.HashMap;
import java.util.List;

public class MyAdapter extends BaseAdapter {
    private final GlideRequest<Bitmap> glideRequest;
    // 填充数据的list
    private List<VoteListBean.DataBean.ResultBean.ProducersBean> list;
    // 用来控制CheckBox的选中状况
    static HashMap<Integer, Boolean> isSelected;
    // 上下文
    private Context context;
    // 用来导入布局
    private LayoutInflater inflater = null;


    // 构造器
    public MyAdapter(List<VoteListBean.DataBean.ResultBean.ProducersBean> list, Context context) {
        this.context = context;
        this.list = list;
        inflater = LayoutInflater.from(context);
        isSelected = new HashMap<Integer, Boolean>();
        // 初始化数据
        initDate();
        glideRequest = GlideApp.with(context).asBitmap().error(R.mipmap.found_vote_initial).placeholder(R.mipmap.found_vote_initial).circleCrop();
    }

    // 初始化isSelected的数据
    private void initDate() {
        if (list != null) {
            for (int i = 0; i < list.size(); i++) {
                getIsSelected().put(i, false);
            }
        }
    }

    @Override
    public int getCount() {
        return list.size();
    }

    @Override
    public Object getItem(int position) {
        return list.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder holder = null;
        if (convertView == null) {
            // 获得ViewHolder对象
            holder = new ViewHolder();
            // 导入布局并赋值给convertview
            convertView = inflater.inflate(R.layout.item_node_car, null);
            //holder.tv = (TextView) convertView.findViewById(R.id.item_tv);
            holder.cb = (AppCompatCheckBox) convertView.findViewById(R.id.checkbox);
            holder.tv_name = (TextView) convertView.findViewById(R.id.tv_name);
            holder.tv_address = (TextView) convertView.findViewById(R.id.tv_address);
            holder.tv_id = (TextView) convertView.findViewById(R.id.tv_id);
            holder.tv_zb = (TextView) convertView.findViewById(R.id.tv_zb);
            holder.ivIcon = convertView.findViewById(R.id.iv_icon);
            // 为view设置标签
            convertView.setTag(holder);
        } else {
            // 取出holder
            holder = (ViewHolder) convertView.getTag();
        }
        // 设置list中TextView的显示
        //   holder.tv.setText(list.get(position));
        // 根据isSelected来设置checkbox的选中状况
        if (getIsSelected().get(position) != null) {
            holder.cb.setChecked(getIsSelected().get(position));
        }
        VoteListBean.DataBean.ResultBean.ProducersBean producersBean = list.get(position);

        holder.tv_zb.setText(NumberiUtil.numberFormat(Double.parseDouble(producersBean.getVoterate()) * 100 + "", 5) + "%");
        holder.tv_name.setText(producersBean.getNickname());
        holder.tv_address.setText(AppUtlis.getLoc(context, producersBean.getLocation() + ""));
        int id = producersBean.getIndex() + 1;
        holder.tv_id.setText("NO." + id);//12
        AppCompatImageView iv = holder.ivIcon;
        glideRequest.load(R.mipmap.found_vote_initial).into(iv);
        iv.setTag(R.string.error_tag_empty, position + "");
        if (iv.getTag(R.string.error_tag_empty) != null && iv.getTag(R.string.error_tag_empty).equals(position + "")) {
            String url = producersBean.getUrl();
            GetDynanicUrl.getData(url, context, new NodeDotJsonViewData() {
                @Override
                public void onGetNodeDotJsonData(NodeInfoBean t) {
                    if (t == null || t.getOrg() == null || t.getOrg().getBranding() == null) {
                        return;
                    }
                    String imgUrl = t.getOrg().getBranding().getLogo_256();
                    iv.setTag(R.string.ownerpublicKey, imgUrl);
                    if (iv.getTag(R.string.ownerpublicKey) != null && iv.getTag(R.string.ownerpublicKey).equals(imgUrl)) {
                        glideRequest.load(imgUrl).into(iv);
                    } else {
                        GlideApp.with(context).clear(iv);
                        iv.setImageResource(R.mipmap.found_vote_initial);
                    }
                }
            });
        }
        return convertView;
    }

    public static HashMap<Integer, Boolean> getIsSelected() {
        return isSelected;
    }

    public static void delSelected(int i) {
        MyAdapter.isSelected.remove(i);
    }

    public void setIsSelected(HashMap<Integer, Boolean> isSelected) {
        MyAdapter.isSelected = isSelected;
    }

    public class ViewHolder {
        TextView tv_name;
        TextView tv_address;
        TextView tv_id;
        AppCompatImageView ivIcon;


        CheckBox cb;
        TextView tv_zb;

        public CheckBox getCb() {
            return cb;
        }

        public void setCb(CheckBox cb) {
            this.cb = cb;
        }

    }
}
