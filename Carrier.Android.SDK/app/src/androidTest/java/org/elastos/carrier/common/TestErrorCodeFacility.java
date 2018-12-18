package org.elastos.carrier.common;

public enum TestErrorCodeFacility {
    GENERAL, SYS, HTTP, MQTT, ICE, DHT;

    public static TestErrorCodeFacility valueOf(int facility) {
        switch (facility) {
            case 1:
            default:
                return GENERAL;
            case 2:
                return SYS;
            case 3:
                return HTTP;
            case 4:
                return MQTT;
            case 5:
                return ICE;
            case 6:
                return DHT;
        }
    }


    public int value() {
        switch (this) {
            case GENERAL:
            default:
                return 1;
            case SYS:
                return 2;
            case HTTP:
                return 3;
            case MQTT:
                return 4;
            case ICE:
                return 5;
            case DHT:
                return 6;
        }
    }
}
