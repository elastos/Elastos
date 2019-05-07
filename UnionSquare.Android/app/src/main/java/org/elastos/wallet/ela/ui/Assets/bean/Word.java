package org.elastos.wallet.ela.ui.Assets.bean;

public class Word {

    private String word;
    private boolean hide;

    public Word(String word) {
        this.word = word;
    }

    public Word(String word, boolean hide) {
        this.word = word;
        this.hide = hide;
    }

    public String getWord() {
        return word;
    }

    public void setWord(String word) {
        this.word = word;
    }

    public boolean isHide() {
        return hide;
    }

    public void setHide(boolean hide) {
        this.hide = hide;
    }

    @Override
    public String toString() {
        return "Word{" +
                "word='" + word + '\'' +
                '}';
    }

    @Override
    public boolean equals(Object obj) {
        return ((Word) obj).word.equals(this.word);
    }
}