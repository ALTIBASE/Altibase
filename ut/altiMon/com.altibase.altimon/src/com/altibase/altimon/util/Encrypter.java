/** 
 *  Copyright (c) 1999~2017, Altibase Corp. and/or its affiliates. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License, version 3,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
package com.altibase.altimon.util;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.security.spec.InvalidKeySpecException;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.CipherInputStream;
import javax.crypto.CipherOutputStream;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.PBEParameterSpec;

import org.apache.commons.codec.binary.Base64;

public class Encrypter 
{
    private static final Encrypter INSTANCE = new Encrypter();

    private final static String ENCRYPTION_SALT = "cndgjsrl"; // must be 8 bytes
    private final static String ENCRYPTION_PASSWORD = "dkfxlqpdltmdkagh";	
    private final static String ENCRYPTION_ALGORITHM = "PBEWithMD5AndDES";
    private final static int 	ENCRYPTION_ITERATION = 20;

    private Cipher ecipher;
    private Cipher dcipher;

    private final static String ENCODING_SCHEME = "UTF16";

    public static Encrypter getDefault()
    {
        return INSTANCE;
    }

    private Encrypter() {
        // Create an 8-byte initialization vector
        PBEKeySpec 			pbeKeySpec;
        PBEParameterSpec 	pbeParameterSpec;
        SecretKeyFactory 	secretKeyFactory;
        SecretKey 			secretKey;

        try {

            pbeParameterSpec = new PBEParameterSpec(ENCRYPTION_SALT.getBytes(), ENCRYPTION_ITERATION);

            pbeKeySpec = new PBEKeySpec(ENCRYPTION_PASSWORD.toCharArray());

            secretKeyFactory = SecretKeyFactory.getInstance(ENCRYPTION_ALGORITHM);

            secretKey = secretKeyFactory.generateSecret(pbeKeySpec);

            dcipher = Cipher.getInstance(ENCRYPTION_ALGORITHM);
            ecipher = Cipher.getInstance(ENCRYPTION_ALGORITHM);

            dcipher.init(Cipher.DECRYPT_MODE, secretKey, pbeParameterSpec);
            ecipher.init(Cipher.ENCRYPT_MODE, secretKey, pbeParameterSpec);

        } catch (java.security.InvalidAlgorithmParameterException e) {
            e.printStackTrace();
        } catch (javax.crypto.NoSuchPaddingException e) {
            e.printStackTrace();
        } catch (java.security.NoSuchAlgorithmException e) {
            e.printStackTrace();
        } catch (java.security.InvalidKeyException e) {
            e.printStackTrace();
        } catch (InvalidKeySpecException e) {
            e.printStackTrace();
        }
    }

    public void encrypt(InputStream in, OutputStream out) {

        byte[] buf = new byte[1024];

        try {
            // Bytes written to out will be encrypted
            out = new CipherOutputStream(out, ecipher);

            // Read in the clear text bytes and write to out to encrypt
            int numRead = 0;
            while ((numRead = in.read(buf)) >= 0) {
                out.write(buf, 0, numRead);
            }
            out.close();
        } catch (java.io.IOException e) {
        }
    }

    public void decrypt(InputStream in, OutputStream out) {

        byte[] buf = new byte[1024];

        try {
            // Bytes read from in will be decrypted
            in = new CipherInputStream(in, dcipher);

            // Read in the decrypted bytes and write the cleartext to out
            int numRead = 0;
            while ((numRead = in.read(buf)) >= 0) {
                out.write(buf, 0, numRead);
            }
            //out.close();
        } catch (java.io.IOException e) {
        }
    }

    public String encrypt(String plainText) {
        byte[] encrypted = null;
        byte[] encoded = null;
        String result = null;
        try {
            encoded = plainText.getBytes(ENCODING_SCHEME);
            encrypted = ecipher.doFinal(encoded);
            //result = new sun.misc.BASE64Encoder().encode(encrypted);
            result = Base64.encodeBase64String(encrypted);
        } catch (IllegalBlockSizeException e) {
            e.printStackTrace();
        } catch (BadPaddingException e) {
            e.printStackTrace();
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        } 

        return result;
    }

    public String decrypt(String encryptedText) {
        byte[] decoded = null;
        byte[] decrypted = null;
        String result = null;

        try {
            //decoded = new sun.misc.BASE64Decoder().decodeBuffer(encryptedText);
            decoded = Base64.decodeBase64(encryptedText);
            decrypted = dcipher.doFinal(decoded);
            result = new String(decrypted, ENCODING_SCHEME);
        } catch (IllegalBlockSizeException e) {
            e.printStackTrace();
        } catch (BadPaddingException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

        return result;
    }
}

