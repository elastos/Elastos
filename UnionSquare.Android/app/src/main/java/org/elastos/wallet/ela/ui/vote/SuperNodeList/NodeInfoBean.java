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

package org.elastos.wallet.ela.ui.vote.SuperNodeList;

public class NodeInfoBean {
    /**
     * arbiter_public_key :
     * org : {"candidate_name":"","candidate_info":{"en":"English  introductionEnglish  introductionEnglish  introductionEnglish  introductionEnglish  introductionEnglish  introductionEnglish  introductionEnglish  introduction","zh":"中文简介中文简介中文简介中文简介中文简介中文简介中文简介中文简介中文简介中文简介中文简介","ja":""},"website":"","code_of_conduct":"","ownership_disclosure":"","email":"","branding":{"logo_256":"http://photocdn.sohu.com/20120128/Img333056814.jpg","logo_1024":"","logo_svg":""},"location":{"name":"","country":"","latitude":0,"longitude":0},"social":{"twitter":"","youtube":"","facebook":"","github":"","reddit":"","keybase":"","telegram":"","wechat":""}}
     */

    private String arbiter_public_key;
    private OrgBean org;

    public String getArbiter_public_key() {
        return arbiter_public_key;
    }

    public void setArbiter_public_key(String arbiter_public_key) {
        this.arbiter_public_key = arbiter_public_key;
    }

    public OrgBean getOrg() {
        return org;
    }

    public void setOrg(OrgBean org) {
        this.org = org;
    }

    public static class OrgBean {
        /**
         * candidate_name :
         * candidate_info : {"en":"English  introductionEnglish  introductionEnglish  introductionEnglish  introductionEnglish  introductionEnglish  introductionEnglish  introductionEnglish  introduction","zh":"中文简介中文简介中文简介中文简介中文简介中文简介中文简介中文简介中文简介中文简介中文简介","ja":""}
         * website :
         * code_of_conduct :
         * ownership_disclosure :
         * email :
         * branding : {"logo_256":"http://photocdn.sohu.com/20120128/Img333056814.jpg","logo_1024":"","logo_svg":""}
         * location : {"name":"","country":"","latitude":0,"longitude":0}
         * social : {"twitter":"","youtube":"","facebook":"","github":"","reddit":"","keybase":"","telegram":"","wechat":""}
         */

        private String candidate_name;
        private CandidateInfoBean candidate_info;
        private String website;
        private String code_of_conduct;
        private String ownership_disclosure;
        private String email;
        private BrandingBean branding;
        private LocationBean location;
        private SocialBean social;

        public String getCandidate_name() {
            return candidate_name;
        }

        public void setCandidate_name(String candidate_name) {
            this.candidate_name = candidate_name;
        }

        public CandidateInfoBean getCandidate_info() {
            return candidate_info;
        }

        public void setCandidate_info(CandidateInfoBean candidate_info) {
            this.candidate_info = candidate_info;
        }

        public String getWebsite() {
            return website;
        }

        public void setWebsite(String website) {
            this.website = website;
        }

        public String getCode_of_conduct() {
            return code_of_conduct;
        }

        public void setCode_of_conduct(String code_of_conduct) {
            this.code_of_conduct = code_of_conduct;
        }

        public String getOwnership_disclosure() {
            return ownership_disclosure;
        }

        public void setOwnership_disclosure(String ownership_disclosure) {
            this.ownership_disclosure = ownership_disclosure;
        }

        public String getEmail() {
            return email;
        }

        public void setEmail(String email) {
            this.email = email;
        }

        public BrandingBean getBranding() {
            return branding;
        }

        public void setBranding(BrandingBean branding) {
            this.branding = branding;
        }

        public LocationBean getLocation() {
            return location;
        }

        public void setLocation(LocationBean location) {
            this.location = location;
        }

        public SocialBean getSocial() {
            return social;
        }

        public void setSocial(SocialBean social) {
            this.social = social;
        }

        public static class CandidateInfoBean {
            /**
             * en : English  introductionEnglish  introductionEnglish  introductionEnglish  introductionEnglish  introductionEnglish  introductionEnglish  introductionEnglish  introduction
             * zh : 中文简介中文简介中文简介中文简介中文简介中文简介中文简介中文简介中文简介中文简介中文简介
             * ja :
             */

            private String en;
            private String zh;
            private String ja;

            public String getEn() {
                return en;
            }

            public void setEn(String en) {
                this.en = en;
            }

            public String getZh() {
                return zh;
            }

            public void setZh(String zh) {
                this.zh = zh;
            }

            public String getJa() {
                return ja;
            }

            public void setJa(String ja) {
                this.ja = ja;
            }
        }

        public static class BrandingBean {
            /**
             * logo_256 : http://photocdn.sohu.com/20120128/Img333056814.jpg
             * logo_1024 :
             * logo_svg :
             */

            private String logo_256;
            private String logo_1024;
            private String logo_svg;

            public String getLogo_256() {
                return logo_256;
            }

            public void setLogo_256(String logo_256) {
                this.logo_256 = logo_256;
            }

            public String getLogo_1024() {
                return logo_1024;
            }

            public void setLogo_1024(String logo_1024) {
                this.logo_1024 = logo_1024;
            }

            public String getLogo_svg() {
                return logo_svg;
            }

            public void setLogo_svg(String logo_svg) {
                this.logo_svg = logo_svg;
            }
        }

        public static class LocationBean {
            /**
             * name :
             * country :
             * latitude : 0
             * longitude : 0
             */

            private String name;
            private String country;
            private int latitude;
            private int longitude;

            public String getName() {
                return name;
            }

            public void setName(String name) {
                this.name = name;
            }

            public String getCountry() {
                return country;
            }

            public void setCountry(String country) {
                this.country = country;
            }

            public int getLatitude() {
                return latitude;
            }

            public void setLatitude(int latitude) {
                this.latitude = latitude;
            }

            public int getLongitude() {
                return longitude;
            }

            public void setLongitude(int longitude) {
                this.longitude = longitude;
            }
        }

        public static class SocialBean {
            /**
             * twitter :
             * youtube :
             * facebook :
             * github :
             * reddit :
             * keybase :
             * telegram :
             * wechat :
             */

            private String twitter;
            private String youtube;
            private String facebook;
            private String github;
            private String reddit;
            private String keybase;
            private String telegram;
            private String wechat;

            public String getTwitter() {
                return twitter;
            }

            public void setTwitter(String twitter) {
                this.twitter = twitter;
            }

            public String getYoutube() {
                return youtube;
            }

            public void setYoutube(String youtube) {
                this.youtube = youtube;
            }

            public String getFacebook() {
                return facebook;
            }

            public void setFacebook(String facebook) {
                this.facebook = facebook;
            }

            public String getGithub() {
                return github;
            }

            public void setGithub(String github) {
                this.github = github;
            }

            public String getReddit() {
                return reddit;
            }

            public void setReddit(String reddit) {
                this.reddit = reddit;
            }

            public String getKeybase() {
                return keybase;
            }

            public void setKeybase(String keybase) {
                this.keybase = keybase;
            }

            public String getTelegram() {
                return telegram;
            }

            public void setTelegram(String telegram) {
                this.telegram = telegram;
            }

            public String getWechat() {
                return wechat;
            }

            public void setWechat(String wechat) {
                this.wechat = wechat;
            }
        }
    }
}
