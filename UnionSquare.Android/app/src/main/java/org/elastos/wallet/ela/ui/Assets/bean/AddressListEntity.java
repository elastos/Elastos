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
