package com.classify;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;

import javax.imageio.ImageIO;

import com.google.gson.JsonObject;

public class Classify {

    public static InceptionImageClassifier classifier = null;

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
    
    public static JsonObject main(JsonObject args) {
    	JsonObject response = new JsonObject();

        try {
           	if (classifier == null) {
                classifier = new InceptionImageClassifier();
                downloadIfNecessary("/tmp/tensorflow_inception_graph.pb", args.getAsJsonPrimitive("model_url").getAsString());
                downloadIfNecessary("/tmp/imagenet_comp_graph_label_strings.txt", args.getAsJsonPrimitive("labels_url").getAsString());
    			classifier.load_model(new FileInputStream("/tmp/tensorflow_inception_graph.pb"));
    			classifier.load_labels(new FileInputStream(("/tmp/imagenet_comp_graph_label_strings.txt")));
            }
           	
            try (FileOutputStream stream = new FileOutputStream("image.jpg")) {
                stream.write(downloadBytes(args.getAsJsonPrimitive("image_url").getAsString()));
            } catch (Exception e) {
                 response.addProperty("exception", e.getMessage());
                 e.printStackTrace();
            }

			response.addProperty("prediction", classifier.predict_image(ImageIO.read(new FileInputStream("image.jpg"))));
		} catch (Exception e) {
			response.addProperty("exception", e.getMessage());
			e.printStackTrace();
		}
        
        return response;
    }
    
    public static void main(String[] args) throws Exception {
        JsonObject arguments = new JsonObject();
        arguments.addProperty("model_url", "http://127.0.0.1:8000/tensorflow_inception_graph.pb");
        arguments.addProperty("labels_url", "http://127.0.0.1:8000/imagenet_comp_graph_label_strings.txt");
        arguments.addProperty("image_url", "http://127.0.0.1:8000/eagle.jpg");
        System.out.println(main(arguments));
    }
    

}
