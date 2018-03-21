package org.elastos.carrier.common;

public class TestErrorCode {
    private int errorCode;

    public TestErrorCode(int facility, int code) {
        errorCode = (0x80000000 | facility << 24 | ((code & 0x80000000) >> 8 | (code & 0x7fffffff)));
    }

    public TestErrorCode(TestErrorCodeFacility facility, int code) {
        errorCode = (0x80000000 | facility.value() << 24 | ((code & 0x80000000) >> 8 | (code & 0x7fffffff)));
    }

    public static TestErrorCode generalError(int code) {
        return new TestErrorCode(TestErrorCodeFacility.GENERAL, code);
    }

    public static TestErrorCode sysError(int code) {
        return new TestErrorCode(TestErrorCodeFacility.SYS, code);
    }

    public static TestErrorCode httpError(int code) {
        return new TestErrorCode(TestErrorCodeFacility.HTTP, code);
    }

    public static TestErrorCode mqttError(int code) {
        return new TestErrorCode(TestErrorCodeFacility.MQTT, code);
    }

    public static TestErrorCode iceError(int code) {
        return new TestErrorCode(TestErrorCodeFacility.ICE, code);
    }

    public int getErrorCode() {
        return errorCode;
    }
}
