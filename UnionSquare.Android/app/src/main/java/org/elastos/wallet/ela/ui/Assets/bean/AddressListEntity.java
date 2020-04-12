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

package org.elastos.wallet.ela.ui.Assets.bean;

import java.util.List;

public class AddressListEntity {
    /**
     * Addresses : ["ES1P3Tmf3qvJAv4NvS7vKp8Mr3QsG5WJyB","EZEDLbxX39NiVzEVhzuzZUbmmJRUhuGvdy","EbGBsajf2hpu4HomoP1nRhMFjFLBtZPjMA","EfKPzoQM8btgt1wyA4Cuwvnqnu2E2JAX5a","EfUMnEFn2B3EBNLvgX8n68YtfYKYxxPRrP","ESbHQRpqNC3G1aFozZhXzMHPVPLyLdodpY","EJUp5HArFr15Mut9dCeo97gWtkHG8GNb62","EW6887cVvyZUEyNeGFyv5X5dkGkJAixham","EUpfswAE1PE53X7Ga6ZnGoUUDiSayagXxK","ELvYZuu5bSPnP1Y6UcMsCbRPjLBdRa2MJ7","EbipmqezKXqBtvNB1iNeMon8hGyxNB8whW","EdnKuquFJXHH7sstSjUbGGz5Ne4xS3Z5wo","ERjku39nERxPQVbiaNwK4kpxchkUYHZDRx","EY3Jfgr8WWdyv29FtFcdsEth9x3Bg7ytb3","EM1uTGyasAHAdrDDmyDc1hLXr4ryfksohu","EZm94tq8kN2YjBHfqpkmhJrmL7Knhq15tL","EKYauzw7xKRWEsxsUYkeU3bNeJdvo6p6xt","ERSGToThQRj1VqR2GQoja6wWQ3fQzF2FTW","EdiywPWeecw2CJoRssbqCvqJLJPFRgh5qb","ENUUrqccJDFNpM6fvwvUGyh76cupQaZtDa"]
     * MaxCount : 215
     */

    private int MaxCount;
    private List<String> Addresses;

    public int getMaxCount() {
        return MaxCount;
    }

    public void setMaxCount(int MaxCount) {
        this.MaxCount = MaxCount;
    }

    public List<String> getAddresses() {
        return Addresses;
    }

    public void setAddresses(List<String> Addresses) {
        this.Addresses = Addresses;
    }
}
