package org.elastos.wallet.ela.bean;

import java.util.List;

/**
 * author: fangxiaogang
 * date: 2018/9/18
 */

public class Knowledge {






        private List<Children> children;
        private int courseId;
        private int id;
        private String name;
        private int order;
        private int parentChapterId;
        private int visible;
        public void setChildren(List<Children> children) {
            this.children = children;
        }
        public List<Children> getChildren() {
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

        public void setOrder(int order) {
            this.order = order;
        }
        public int getOrder() {
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



    public class Children {

        private List<String> children;
        private int courseId;
        private int id;
        private String name;
        private int order;
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

        public void setOrder(int order) {
            this.order = order;
        }
        public int getOrder() {
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



}
