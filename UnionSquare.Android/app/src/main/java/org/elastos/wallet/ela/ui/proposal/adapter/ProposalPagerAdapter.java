/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

package org.elastos.wallet.ela.ui.proposal.adapter;

import android.content.Context;
import android.support.v4.view.PagerAdapter;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.view.ViewGroup;

import org.elastos.wallet.ela.ui.Assets.adapter.WalletListRecAdapetr;

import java.util.List;

public class ProposalPagerAdapter extends PagerAdapter {
    private final Context context;
    private List<String> titles;

    public ProposalPagerAdapter(List<String> titles, Context context) {
        this.context = context;
        this.titles = titles;
    }

    @Override
    public int getCount() {
        return titles.size();
    }

    //判断是否是否为同一张图片，这里返回方法中的两个参数做比较就可以
    @Override
    public boolean isViewFromObject(View view, Object object) {
        return view == object;
    }

    //设置viewpage内部东西的方法，如果viewpage内没有子空间滑动产生不了动画效果
    @Override
    public Object instantiateItem(ViewGroup container, int position) {
        RecyclerView re = getRecycleView(context, position);
        container.addView(re);
        //最后要返回的是控件本身
        return re;
    }

    public void setOnItemOnclickListner(OnItemClickListner onItemOnclickListner) {
        this.onItemOnclickListner = onItemOnclickListner;
    }

    private OnItemClickListner onItemOnclickListner;

    public interface OnItemClickListner {
        void onItemClick(View v, int position);
    }

    private RecyclerView getRecycleView(Context context, int position) {
        RecyclerView rv = new RecyclerView(context);
        //ProposalRecAdapetr adapter = new ProposalRecAdapetr(context,  position);
        rv.setLayoutManager(new LinearLayoutManager(context, LinearLayoutManager.VERTICAL, false));
       // rv.setAdapter(adapter);
       // adapter.setCommonRvListener(onItemOnclickListner);
        return rv;


    }

    //因为它默认是看三张图片，第四张图片的时候就会报错，还有就是不要返回父类的作用
    @Override
    public void destroyItem(ViewGroup container, int position, Object object) {
        container.removeView((View) object);
        //         super.destroyItem(container, position, object);
    }

    //目的是展示title上的文字，
    @Override
    public CharSequence getPageTitle(int position) {

        return titles.get(position);
    }
}
