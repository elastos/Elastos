package org.elastos.wallet.ela.net;

import org.elastos.wallet.ela.bean.GetdePositcoinBean;
import org.elastos.wallet.ela.ui.crvote.bean.CRDePositcoinBean;
import org.elastos.wallet.ela.ui.main.entity.ServerListEntity;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.NodeInfoBean;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.utils.Constant;

import java.util.Map;

import io.reactivex.Observable;
import retrofit2.http.FieldMap;
import retrofit2.http.FormUrlEncoded;
import retrofit2.http.GET;
import retrofit2.http.Headers;
import retrofit2.http.POST;
import retrofit2.http.Url;

public interface ApiServer {
    @FormUrlEncoded
    @POST("api/dposnoderpc/check/listproducer")
    @Headers("Content-Type:application/x-www-form-urlencoded; charset=utf-8")
    Observable<VoteListBean> votelistbean(@FieldMap Map<String, String> map);

    @FormUrlEncoded
    @POST("api/dposnoderpc/check/getdepositcoin")
    @Headers("Content-Type:application/x-www-form-urlencoded; charset=utf-8")
    Observable<GetdePositcoinBean> getdepositcoin(@FieldMap Map<String, String> map);

    // Observable<ResponseBody> getUrlJson();获得String类型用这个
    @GET
    Observable<NodeInfoBean> getUrlJson(@Url String url);//不同baseurl用@Url @GET

    @GET(Constant.SERVERLIST)
    Observable<ServerListEntity> getServerList();//不同baseurl用@Url
    @FormUrlEncoded
    @POST("api/dposnoderpc/check/getdepositcoin")
    @Headers("Content-Type:application/x-www-form-urlencoded; charset=utf-8")
    Observable<CRDePositcoinBean> getCRDepositcoin(@FieldMap Map<String, String> map);
}
