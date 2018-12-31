package com.ansca.util;

import java.io.FileInputStream;
import java.io.IOException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateException;
import java.util.Enumeration;

/**
 * javap -classpath bin -protected com.ansca.util.ListKeyStore -s

public class com.ansca.util.ListKeyStore extends java.lang.Object{
public com.ansca.util.ListKeyStore();
  Signature: ()V
protected int isKeyStoreValid(java.lang.String, java.lang.String);
  Signature: (Ljava/lang/String;Ljava/lang/String;)I
public static java.lang.String[] listAliases(java.lang.String, java.lang.String);
  Signature: (Ljava/lang/String;Ljava/lang/String;)[Ljava/lang/String;
}

 * @author eherrman
 *
 */

public class ListKeyStore {

	/**
	 * Check keystore for validity
	 * @param filename
	 * @param password
	 * @return
	 */
	protected static int isKeyStoreValid( String filename, String password )
	{
		int result = 0;
		
		try {
		    FileInputStream stream = new FileInputStream(filename);
		    KeyStore keystore = KeyStore.getInstance("JKS");
	
		    keystore.load( stream, password.toCharArray() );

		} catch ( IOException ie ) {
			result = 1;
		} catch ( NoSuchAlgorithmException ne ) {
			result = 2;
		} catch ( CertificateException ce ) {
			result = 3;
		} catch ( KeyStoreException ke ) {
			result = 4;
		}
		
		return result;
	}

	protected static String[] listAliases( String filename, String password )
	{
		String [] result = null;
		
		try {
		    FileInputStream stream = new FileInputStream(filename);
		    KeyStore keystore = KeyStore.getInstance("JKS");
	
		    keystore.load( stream, password.toCharArray() );

		    Enumeration<String> aliases = keystore.aliases();
		    
		    result = new String[keystore.size()];
		    for ( int i = 0; i < keystore.size(); i++ ) {
		    	result[i] = aliases.nextElement();
		    }
		} catch ( Exception e ) {
		}
		
		return result;
	}

	/**
	 * @param args
	 */
//	public static void main(String[] args) {
//		String[] aliases = listAliases( args[0], args[1] );
//		
//		for ( String alias : aliases ) {
//			System.out.println( alias );
//		}
//	}
}
