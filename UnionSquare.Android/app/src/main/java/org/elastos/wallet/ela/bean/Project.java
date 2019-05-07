package org.elastos.wallet.ela.bean;

import java.util.List;

/**
 * author: fangxiaogang
 * date: 2018/9/19
 */

public class Project {


        private List<String> children;
        private int courseId;
        private int id;
        private String name;
        private long order;
        private int parentChapterId;
        private int visible;
        public void setChildren(List<String> children) {
            this.children = children;
        }
        public List<String> getChildren() {
            return children;
        }

        public void setCourseId(int courseId) {
            this.courseId = courseId;
        }
        public int getCourseId() {
            return courseId;
        }

        public void setId(int id) {
            this.id = id;
        }
        public int getId() {
            return id;
        }

        public void setName(String name) {
            this.name = name;
        }
        public String getName() {
            return name;
        }

        public void setOrder(long order) {
            this.order = order;
        }
        public long getOrder() {
            return order;
        }

        public void setParentChapterId(int parentChapterId) {
            this.parentChapterId = parentChapterId;
        }
        public int getParentChapterId() {
            return parentChapterId;
        }

        public void setVisible(int visible) {
            this.visible = visible;
        }
        public int getVisible() {
            return visible;
        }

}
