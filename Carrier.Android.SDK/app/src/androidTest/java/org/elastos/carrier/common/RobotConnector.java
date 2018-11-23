package org.elastos.carrier.common;

import org.elastos.carrier.Log;
import org.elastos.carrier.robot.Robot;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.Socket;
import java.net.InetSocketAddress;

public class RobotConnector extends Socket {
	private static final String TAG = "RobotConnector";
	private static RobotConnector robotConnector = null;
	private BufferedReader mBufferedReader = null;
	private String mRobotId = null;
	private String mRobotAddress = null;

	public static RobotConnector getInstance() {
		if (robotConnector == null)
			robotConnector = new RobotConnector();

		return robotConnector;
	}

	private RobotConnector() {
		super();
	}

	public boolean connectToRobot() {
		String host = Robot.ROBOTHOST;
		String port = Robot.ROBOTPORT;

		try {
			InetSocketAddress address = new InetSocketAddress(host, Integer.parseInt(port));
			connect(address);
		}
		catch (IOException e) {
			e.printStackTrace();
		}

		if (isConnected()) {
			Log.i(TAG, String.format("Connected to robot (%s:%s)", host, port));

			String[] data = readAck();
			if (data != null && data.length == 3 && data[0].equals("ready")) {
				mRobotId = data[1];
				mRobotAddress = data[2];
				return true;
			}

			return false;
		}
		else {
			Log.w(TAG, String.format("Connect to robot (%s:%s) failed", host, port));
		}

		return isConnected();
	}

	public void disconnect() {
		try {
			if (isConnected())
				close();
		}
		catch (IOException e) {
			//DO nothing.
		}

		if (mBufferedReader != null) {
			try {
				mBufferedReader.close();
			}
			catch (IOException e){
				e.printStackTrace();
			}
		}

		Log.d(TAG, "Disconnected from robot");
	}

	public boolean writeCmd(String command) {
		if (!isConnected()) {
			Log.e(TAG, "Connection to robot is broken, write command error");
			return false;
		}

		try {
			getOutputStream().write((command + "\n").getBytes("utf-8"));
			getOutputStream().flush();
		}
		catch (IOException e) {
			Log.e(TAG, "Write command to robot failed with IO error");
			e.printStackTrace();
			return false;
		}

		return true;
	}

	public String[] readAck() {
		BufferedReader br;
		try {
			if (mBufferedReader == null) {
				InputStreamReader isr = new InputStreamReader(getInputStream());
				mBufferedReader = new BufferedReader(isr);
			}

			br = mBufferedReader;
			String revData = br.readLine();
			Log.d(TAG, "revData==========="+revData);
			return revData.split("\\s+");
		}
		catch (IOException e) {
			e.printStackTrace();
		}
		return null;
	}

	public String getNodeid() {
		return mRobotId;
	}

	public String getAddress() {
		return mRobotAddress;
	}
}
