//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;

import java.io.UnsupportedEncodingException;
import java.security.InvalidKeyException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;

public class Crypto {
	public final static String ALGORITHM_MD4 = "MD4";
	public final static String ALGORITHM_MD5 = "MD5";
	public final static String ALGORITHM_SHA1A = "SHA1A";
	public final static String ALGORITHM_SHA224A = "SHA224A";
	public final static String ALGORITHM_SHA256A = "SHA256A";
	public final static String ALGORITHM_SHA384 = "SHA384";
	public final static String ALGORITHM_SHA512 = "SHA512";
	
	public final static String ALGORITHM_HMAC_MD4 = "HmacMD4";
	public final static String ALGORITHM_HMAC_MD5 = "HmacMD5";
	public final static String ALGORITHM_HMAC_SHA1A = "HmacSHA1A";
	public final static String ALGORITHM_HMAC_SHA224A = "HmacSHA224A";
	public final static String ALGORITHM_HMAC_SHA256A = "HmacSHA256A";
	public final static String ALGORITHM_HMAC_SHA384 = "HmacSHA384";
	public final static String ALGORITHM_HMAC_SHA512 = "HmacSHA512";
	
	public static int GetDigestLength( String algorithm ) {
		int length = 0;
		try {
			MessageDigest digest = MessageDigest.getInstance( algorithm );
			length = digest.getDigestLength();
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		return length;
	}
	
	public static byte[] CalculateDigest( String algorithm, byte[] data) {
		byte[] result = null;
		try {
			MessageDigest digest = MessageDigest.getInstance( algorithm );
			result = digest.digest( data );
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		return result;
	}

	public static byte[] CalculateHMAC( String algorithm, byte[] key, byte[] data ) {
		byte[] result = null;
		try {
			Mac hmac = Mac.getInstance(algorithm); 
			hmac.init(new SecretKeySpec(key, "RAW")); 
			result = hmac.doFinal(data); 
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		return result;
	}
}
