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

package org.elastos.wallet.ela.net;

import org.elastos.wallet.ela.bean.GetdePositcoinBean;
import org.elastos.wallet.ela.bean.ImageBean;
import org.elastos.wallet.ela.ui.committee.bean.CtDetailBean;
import org.elastos.wallet.ela.ui.committee.bean.CtListBean;
import org.elastos.wallet.ela.ui.committee.bean.PastCtBean;
import org.elastos.wallet.ela.ui.crvote.bean.CRDePositcoinBean;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.did.entity.GetJwtRespondBean;
import org.elastos.wallet.ela.ui.did.entity.SaveJwtRespondBean;
import org.elastos.wallet.ela.ui.did.entity.WebBackEntity;
import org.elastos.wallet.ela.ui.main.entity.ServerListEntity;
import org.elastos.wallet.ela.ui.proposal.bean.ProposalDetailEntity;
import org.elastos.wallet.ela.ui.proposal.bean.ProposalSearchEntity;
import org.elastos.wallet.ela.ui.proposal.bean.SuggestBean;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.NodeInfoBean;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;

import java.util.Map;

import io.reactivex.Observable;
import retrofit2.http.Body;
import retrofit2.http.FieldMap;
import retrofit2.http.FormUrlEncoded;
import retrofit2.http.GET;
import retrofit2.http.Headers;
import retrofit2.http.POST;
import retrofit2.http.Path;
import retrofit2.http.QueryMap;
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


    @POST
    Observable<WebBackEntity> postData(@Url String url, @Body Map map);//不同baseurl用@Url @GET

    @GET("api/dposNodeRPC/getProducerNodesList")
    Observable<ServerListEntity> getServerList();//不同baseurl用@Url

    @FormUrlEncoded
    @POST("api/dposnoderpc/check/listcrcandidates")
    @Headers("Content-Type:application/x-www-form-urlencoded; charset=utf-8")
    Observable<CRListBean> getCRlist(@FieldMap Map<String, Object> map);

    @FormUrlEncoded
    @POST("api/dposnoderpc/check/getcrdepositcoin")
    @Headers("Content-Type:application/x-www-form-urlencoded; charset=utf-8")
    Observable<CRDePositcoinBean> getCRDepositcoin(@FieldMap Map<String, String> map);

    @FormUrlEncoded
    @POST("api/dposnoderpc/check/getimage")
    @Headers("Content-Type:application/x-www-form-urlencoded; charset=utf-8")
    Observable<ImageBean> getImageUrl(@FieldMap Map<String, String> map);

    @FormUrlEncoded
    @POST("api/dposnoderpc/check/jwtsave")
    @Headers("Content-Type:application/x-www-form-urlencoded; charset=utf-8")
    Observable<SaveJwtRespondBean> jwtSave(@FieldMap Map<String, String> map);

    @FormUrlEncoded
    @POST("api/dposnoderpc/check/jwtget")
    @Headers("Content-Type:application/x-www-form-urlencoded; charset=utf-8")
    Observable<GetJwtRespondBean> jwtGet(@FieldMap Map<String, String> map);

    @GET("/api/suggestion/get_suggestion/{id}")
    Observable<SuggestBean> getSuggestion(@Path("id") String id);

    @GET("/api/cvote/all_search")
    Observable<ProposalSearchEntity> proposalSearch(@QueryMap Map<String, Object> map);

    @GET("/api/council/term")
    Observable<PastCtBean> getCouncilTerm();

    @GET("/api/council/list/{id}")
    Observable<CtListBean> getCouncilList(@Path("id") String id);

    @GET("/api/council/information/{did}/{id}")
    Observable<CtDetailBean> getCouncilInfo(@Path("id") String id, @Path("did") String did);

    @GET("/api/council/information/{did}")
    Observable<CtDetailBean> getCurrentCouncilInfo(@Path("did") String did);

    @GET("/api/cvote/get_proposal/{id}")
    Observable<ProposalDetailEntity> getProposalDetail(@Path("id") int id);
}
