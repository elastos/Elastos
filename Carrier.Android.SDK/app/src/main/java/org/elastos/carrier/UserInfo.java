/*
 * Copyright (c) 2018 Elastos Foundation
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

package org.elastos.carrier;

/**
 * A class representing the Carrier user information.
 *
 * In carrier SDK, self and each friend also is Carrier user, and has
 * same user attributes.
 */
public class UserInfo {

	/**
	 * Carrier User ID max length.
	 */
	public static final int MAX_ID_LEN = 63;

	/**
	 * Carrier user name max length.
	 */
	public static final int MAX_USER_NAME_LEN = 63;

	/**
	 * Carrier user description max length.
	 */
	public static final int MAX_USER_DESCRIPTION_LEN = 127;

	/**
	 * Carrier user gender max length.
	 */
	public static final int MAX_GENDER_LEN = 31;

	/**
	 * Carrier user phone number max length.
	 */
	public static final int MAX_PHONE_LEN = 31;

	/**
	 * Carrier user email address max length.
	 */
	public static final int MAX_EMAIL_LEN = 127;

	/**
	 * Carrier user region max length.
	 */
	public static final int MAX_REGION_LEN = 127;

	private String userId;
	private String name;
	private String description;
	private boolean hasAvatar;
	private String gender;
	private String phone;
	private String email;
	private String region;

	protected UserInfo() {}

	/**
	 * Set user ID.
	 *
	 * This function only be called in JNI.
	 *
	 * @param
	 * 		userId		The user ID to set
	 */
	void setUserId(String userId) {
		this.userId = userId;
	}

	/**
	 * Get user ID
	 *
	 * @return
	 * 		The user ID
	 */
	public String getUserId() {
		return userId;
	}

	/**
	 * Set nickname, also as display name.
	 *
	 * @param
	 * 		name		The nickname to set
	 *
	 * @throws IllegalArgumentException illegal exception.
	 */
	public void setName(String name)  {
		if (name == null || name.length() > MAX_USER_NAME_LEN)
			throw new IllegalArgumentException("Invalid name length, expected (0,(0," +
				MAX_USER_NAME_LEN + "]");
		this.name = name;
	}

	/**
	 * Get nickname, also as display name.
	 *
	 * @return
	 * 		The nickname.
	 */
	public String getName() {
		return name;
	}

	/**
	 User's brief description, also as what's up.
	 */
	/**
	 * Set user's brief description, also as what's up.
	 *
	 * @param
	 * 		description		the brief description to set
	 *
	 * @throws IllegalArgumentException illegal exception.
	 */
	public void setDescription(String description) {
		if (description == null || description.length() > MAX_USER_DESCRIPTION_LEN)
			throw new IllegalArgumentException("Invalid description length, expected (0,(0," +
				MAX_USER_DESCRIPTION_LEN + "]");
		this.description = description;
	}

	/**
	 * Get user's description, also as what's up
	 *
	 * @return
	 * 		The brief description of user
	 */
	public String getDescription() {
		return description;
	}

	/**
	 * Set user to have avatar.
	 *
	 * @param
	 * 		hasAvatar	Has avatar or not
	 */
	public void setHasAvatar(boolean hasAvatar) {
		this.hasAvatar = hasAvatar;
	}

	/**
	 * Check user have avatar.
	 *
	 * @return
	 * 		True if have avatar, otherwise is not
	 */
	public boolean hasAvatar() {
		return hasAvatar;
	}

	/**
	 * Set user's gender.
	 *
	 * @param
	 * 		gender		The gender to set
	 *
	 * @throws IllegalArgumentException illegal exception.
	 */
	public void setGender(String gender) {
		if (gender == null || gender.length() > MAX_GENDER_LEN)
			throw new IllegalArgumentException("Invalid gender length, expected (0, (0," +
				MAX_GENDER_LEN + "]");
		this.gender = gender;
	}

	/**
	 * Get user's gender.
	 *
	 * @return
	 * 		The user's gender.
	 */
	public String getGender() {
		return gender;
	}

	/**
	 * Set usr's phone number.
	 *
	 * @param
	 * 		phone		The phone number to set
	 *
	 * @throws IllegalArgumentException illegal exception.
	 */
	public void setPhone(String phone) {
		if (phone == null || phone.length() > MAX_PHONE_LEN)
			throw new IllegalArgumentException("Invalid phone length, expected (0, (0," +
				MAX_PHONE_LEN + "]");
		this.phone = phone;
	}

	/**
	 * Get user's phone number.
	 *
	 * @return
	 * 		The phone number of user
	 */
	public String getPhone() {
		return phone;
	}

	/**
	 *	Set user's email address.
	 *
	 * @param
	 * 		email		The email address to set
	 *
	 * @throws IllegalArgumentException illegal exception.
	 */
	public void setEmail(String email) {
		if (email == null || email.length() > MAX_EMAIL_LEN)
			throw new IllegalArgumentException("Invalid email length, expected (0, (0," +
				MAX_EMAIL_LEN + "]");
		this.email = email;
	}

	/**
	 * Get user's email address.
	 *
	 * @return
	 * 		The email address
	 */
	public String getEmail() {
		return email;
	}

	/**
	 * Set user's region
	 *
	 * @param
	 * 		region		The region to set
	 *
	 * @throws IllegalArgumentException illegal exception.
	 */
	public void setRegion(String region) {
		if (region == null || region.length() > MAX_REGION_LEN )
			throw new IllegalArgumentException("Invalid region length, expected (0, (0," +
				MAX_REGION_LEN + "]");
		this.region = region;
	}

	/**
	 * Get user's region.
	 *
	 * @return
	 * 		The region.
	 */
	public String getRegion() {
		return region;
	}

	/**
	 * Get debug description of current UserInfo object.
	 *
	 * @return
	 * 		The debug description of UserInfo object.
	 */
	@Override
	public String toString() {
		return String.format("UserInfo[id:%s, name:%s, description:%s, hasAvatar:%s," +
				"gender:%s, phone:%s, email:%s, region:%s", userId, name, description,
				hasAvatar, gender, phone, email, region);
	}
}
