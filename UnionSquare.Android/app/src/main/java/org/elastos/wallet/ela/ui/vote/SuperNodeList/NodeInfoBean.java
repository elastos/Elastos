package org.elastos.wallet.ela.ui.vote.SuperNodeList;

public class NodeInfoBean {
    /**
     * org : {"website":"","social":{"youtube":"","twitter":"","github":"","facebook":"","reddit":"","wechat":"","telegram":"","keybase":""},"branding":{"logo_256":"http://52.80.54.227:9000/nodelogo256.png","logo_1024":"http://52.80.54.227:9000/nodelogo1024.png","logo_svg":"http://52.80.54.227:9000/nodelogovsg.png"},"location":{"country":"中国","latitude":0,"name":"知春路","longitude":0},"candidate_name":"","ownership_disclosure":"","code_of_conduct":"","email":""}
     * arbiter_public_key :
     */
    private OrgEntity org;
    private String arbiter_public_key;

    public void setOrg(OrgEntity org) {
        this.org = org;
    }

    public void setArbiter_public_key(String arbiter_public_key) {
        this.arbiter_public_key = arbiter_public_key;
    }

    public OrgEntity getOrg() {
        return org;
    }

    public String getArbiter_public_key() {
        return arbiter_public_key;
    }

    public class OrgEntity {
        /**
         * website :
         * social : {"youtube":"","twitter":"","github":"","facebook":"","reddit":"","wechat":"","telegram":"","keybase":""}
         * branding : {"logo_256":"http://52.80.54.227:9000/nodelogo256.png","logo_1024":"http://52.80.54.227:9000/nodelogo1024.png","logo_svg":"http://52.80.54.227:9000/nodelogovsg.png"}
         * location : {"country":"中国","latitude":0,"name":"知春路","longitude":0}
         * candidate_name :
         * ownership_disclosure :
         * code_of_conduct :
         * email :
         */
        private String website;
        private SocialEntity social;
        private BrandingEntity branding;
        private LocationEntity location;
        private String candidate_name;
        private String ownership_disclosure;
        private String code_of_conduct;
        private String email;

        public void setWebsite(String website) {
            this.website = website;
        }

        public void setSocial(SocialEntity social) {
            this.social = social;
        }

        public void setBranding(BrandingEntity branding) {
            this.branding = branding;
        }

        public void setLocation(LocationEntity location) {
            this.location = location;
        }

        public void setCandidate_name(String candidate_name) {
            this.candidate_name = candidate_name;
        }

        public void setOwnership_disclosure(String ownership_disclosure) {
            this.ownership_disclosure = ownership_disclosure;
        }

        public void setCode_of_conduct(String code_of_conduct) {
            this.code_of_conduct = code_of_conduct;
        }

        public void setEmail(String email) {
            this.email = email;
        }

        public String getWebsite() {
            return website;
        }

        public SocialEntity getSocial() {
            return social;
        }

        public BrandingEntity getBranding() {
            return branding;
        }

        public LocationEntity getLocation() {
            return location;
        }

        public String getCandidate_name() {
            return candidate_name;
        }

        public String getOwnership_disclosure() {
            return ownership_disclosure;
        }

        public String getCode_of_conduct() {
            return code_of_conduct;
        }

        public String getEmail() {
            return email;
        }

        public class SocialEntity {
            /**
             * youtube :
             * twitter :
             * github :
             * facebook :
             * reddit :
             * wechat :
             * telegram :
             * keybase :
             */
            private String youtube;
            private String twitter;
            private String github;
            private String facebook;
            private String reddit;
            private String wechat;
            private String telegram;
            private String keybase;

            public void setYoutube(String youtube) {
                this.youtube = youtube;
            }

            public void setTwitter(String twitter) {
                this.twitter = twitter;
            }

            public void setGithub(String github) {
                this.github = github;
            }

            public void setFacebook(String facebook) {
                this.facebook = facebook;
            }

            public void setReddit(String reddit) {
                this.reddit = reddit;
            }

            public void setWechat(String wechat) {
                this.wechat = wechat;
            }

            public void setTelegram(String telegram) {
                this.telegram = telegram;
            }

            public void setKeybase(String keybase) {
                this.keybase = keybase;
            }

            public String getYoutube() {
                return youtube;
            }

            public String getTwitter() {
                return twitter;
            }

            public String getGithub() {
                return github;
            }

            public String getFacebook() {
                return facebook;
            }

            public String getReddit() {
                return reddit;
            }

            public String getWechat() {
                return wechat;
            }

            public String getTelegram() {
                return telegram;
            }

            public String getKeybase() {
                return keybase;
            }

            @Override
            public String toString() {
                return "SocialEntity{" +
                        "youtube='" + youtube + '\'' +
                        ", twitter='" + twitter + '\'' +
                        ", github='" + github + '\'' +
                        ", facebook='" + facebook + '\'' +
                        ", reddit='" + reddit + '\'' +
                        ", wechat='" + wechat + '\'' +
                        ", telegram='" + telegram + '\'' +
                        ", keybase='" + keybase + '\'' +
                        '}';
            }
        }

        public class BrandingEntity {
            /**
             * logo_256 : http://52.80.54.227:9000/nodelogo256.png
             * logo_1024 : http://52.80.54.227:9000/nodelogo1024.png
             * logo_svg : http://52.80.54.227:9000/nodelogovsg.png
             */
            private String logo_256;
            private String logo_1024;
            private String logo_svg;

            public void setLogo_256(String logo_256) {
                this.logo_256 = logo_256;
            }

            public void setLogo_1024(String logo_1024) {
                this.logo_1024 = logo_1024;
            }

            public void setLogo_svg(String logo_svg) {
                this.logo_svg = logo_svg;
            }

            public String getLogo_256() {
                return logo_256;
            }

            public String getLogo_1024() {
                return logo_1024;
            }

            public String getLogo_svg() {
                return logo_svg;
            }

            @Override
            public String toString() {
                return "BrandingEntity{" +
                        "logo_256='" + logo_256 + '\'' +
                        ", logo_1024='" + logo_1024 + '\'' +
                        ", logo_svg='" + logo_svg + '\'' +
                        '}';
            }
        }

        public class LocationEntity {
            /**
             * country : 中国
             * latitude : 0
             * name : 知春路
             * longitude : 0
             */
            private String country;
            private int latitude;
            private String name;
            private int longitude;

            public void setCountry(String country) {
                this.country = country;
            }

            public void setLatitude(int latitude) {
                this.latitude = latitude;
            }

            public void setName(String name) {
                this.name = name;
            }

            public void setLongitude(int longitude) {
                this.longitude = longitude;
            }

            public String getCountry() {
                return country;
            }

            public int getLatitude() {
                return latitude;
            }

            public String getName() {
                return name;
            }

            public int getLongitude() {
                return longitude;
            }
        }

        @Override
        public String toString() {
            return "OrgEntity{" +
                    "website='" + website + '\'' +
                    ", social=" + social +
                    ", branding=" + branding +
                    ", location=" + location +
                    ", candidate_name='" + candidate_name + '\'' +
                    ", ownership_disclosure='" + ownership_disclosure + '\'' +
                    ", code_of_conduct='" + code_of_conduct + '\'' +
                    ", email='" + email + '\'' +
                    '}';
        }
    }

    @Override
    public String toString() {
        return "NodeInfoBean{" +
                "org=" + org +
                ", arbiter_public_key='" + arbiter_public_key + '\'' +
                '}';
    }
}
