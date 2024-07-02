package com.filehashing;

import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import javax.xml.bind.DatatypeConverter;
import java.util.Map;
import java.util.HashMap;

public class FileHashing {
	
	public static byte[] downloadBytes(String url) {
    	try {
    		URLConnection conn = new URL(url).openConnection();
			InputStream is = conn.getInputStream();
			byte[] bytes = is.readAllBytes();
			is.close();
			return bytes;
		} catch (IOException e) {
			e.printStackTrace();
			return null;
		}
    }

    public static HashMap<String, Object> main(Map<String, Object> args) {
        HashMap<String, Object> output = new HashMap<>();

        byte[] bytes = downloadBytes((String)args.get("url"));
        try {
			output.put("hash", DatatypeConverter.printHexBinary(MessageDigest.getInstance("MD5").digest(bytes)));
		} catch (NoSuchAlgorithmException e) {
			output.put("hash", e.getMessage());
			e.printStackTrace();
		}

        return output;
    }

    public static void main(String[] args) {
    	HashMap<String, Object> output = new HashMap<>();
    	output.put("url", "http://127.0.0.1:8000/snap.png");
    	output = main(output);
    	System.out.println(output);
    }
}
