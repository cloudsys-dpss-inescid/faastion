package com.classify;

import java.util.concurrent.ThreadLocalRandom;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;
import java.util.HashMap;
import java.util.Map;

import javax.imageio.ImageIO;

public class Classify {

    public static InceptionImageClassifier classifier = null;
    public static String TMP_IMG_PATH = String.format("/tmp/img-%d.jpg", ThreadLocalRandom.current().nextInt(0, 1024 + 1));

    public static byte[] fromInputStream(InputStream is) throws Exception {
        ByteArrayOutputStream buffer = new ByteArrayOutputStream();
        int nRead;
        byte[] data = new byte[16384];

        while ((nRead = is.read(data, 0, data.length)) != -1) {
            buffer.write(data, 0, nRead);
        }

        return buffer.toByteArray();
    }

    public static byte[] downloadBytes(String url) {
        try {
            URLConnection conn = new URL(url).openConnection();
            InputStream is = conn.getInputStream();
            byte[] bytes = fromInputStream(is); 
            is.close();
            return bytes;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
    
    public static void downloadIfNecessary(String fileName, String fileURL) throws FileNotFoundException, IOException {
    	if (!new File(fileName).exists()) {
            File file = new File(fileName);
            try (FileOutputStream stream = new FileOutputStream(file)) {
                stream.write(downloadBytes(fileURL));
                file.setWritable(false);
                file.setReadable(true);
                file.setExecutable(true);
            } 
        }
    }
    
    public static HashMap<String, Object> main(Map<String, Object> args) {
        HashMap<String, Object> output = new HashMap<>();
        try {
           	if (classifier == null) {
                classifier = new InceptionImageClassifier();
                downloadIfNecessary("/tmp/tensorflow_inception_graph.pb", (String)args.get("model_url"));
                downloadIfNecessary("/tmp/imagenet_comp_graph_label_strings.txt", (String)args.get("labels_url"));
    			classifier.load_model(new FileInputStream("/tmp/tensorflow_inception_graph.pb"));
    			classifier.load_labels(new FileInputStream(("/tmp/imagenet_comp_graph_label_strings.txt")));
            }
           	
            try (FileOutputStream stream = new FileOutputStream(TMP_IMG_PATH)) {
                stream.write(downloadBytes((String)args.get("image_url")));
            }

			output.put("prediction", classifier.predict_image(ImageIO.read(new FileInputStream(TMP_IMG_PATH))));
		} catch (Throwable e) {
			output.put("exception", e.getMessage());
			e.printStackTrace();
		}

        return output;
    }
    
    public static void main(String[] args) throws Exception {
    	HashMap<String, Object> output = new HashMap<>();
    	output.put("model_url", "http://127.0.0.1:8000/tensorflow_inception_graph.pb");
    	output.put("labels_url", "http://127.0.0.1:8000/imagenet_comp_graph_label_strings.txt");
    	output.put("image_url", "http://127.0.0.1:8000/eagle.jpg");
        System.out.println(main(output));
    }
    

}
