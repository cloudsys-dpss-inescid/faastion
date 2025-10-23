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
import java.awt.image.BufferedImage;

import org.graalvm.word.UnsignedWord;
import org.graalvm.nativeimage.c.function.CEntryPoint;
import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.c.type.CCharPointer;
import org.graalvm.nativeimage.c.type.CTypeConversion;

import com.fasterxml.jackson.jr.ob.JSON;

public class Classify {

    private static final String model_url = "http://127.0.0.1:8000/tensorflow_inception_graph.pb";
    private static final String labels_url = "http://127.0.0.1:8000/imagenet_comp_graph_label_strings.txt";
    private static final String image_url = "http://127.0.0.1:8000/eagle.jpg";

    private static InceptionImageClassifier classifier = null;
    private static BufferedImage image = null;
    public static String IMG_FILENAME = String.format("img-%d.jpg", ThreadLocalRandom.current().nextInt(0, 1024 + 1));

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
        String tmpDir = (String) args.get("tmpDir");
        String tmpImgPath = tmpDir + "/" + IMG_FILENAME;
        String modelPath = tmpDir + "/tensorflow_inception_graph.pb";
        String labelsPath = tmpDir + "/imagenet_comp_graph_label_strings.txt";
        HashMap<String, Object> output = new HashMap<>();
        try {
           	if (classifier == null) {
                classifier = new InceptionImageClassifier();
                downloadIfNecessary(modelPath, model_url);
                downloadIfNecessary(labelsPath, labels_url);
    			classifier.load_model(new FileInputStream(modelPath));
    			classifier.load_labels(new FileInputStream(labelsPath));
            }
           	
            try (FileOutputStream stream = new FileOutputStream(tmpImgPath)) {
                stream.write(downloadBytes(image_url));
            }

            // FIXME: ImageIO executes a method from libjavajpeg.so which fails to read image
            //   headers after the library is loaded on a different (parallel) sandbox. 
            //   This problem can be reproduced when co-locating multiple pku sandboxes
            //   in the same process.
            //
            //        Current workaround caches the BufferedImage in `image` to avoid calling
            //   the crashing method from libjavajpeg.so.
            //
            //        Source of the problem is likely related to the native image artifacts.
            if (image == null) {
                image = ImageIO.read(new FileInputStream(tmpImgPath));
            }

			output.put("prediction", classifier.predict_image(image));
        } catch (Throwable e) {
			output.put("exception", e.getMessage());
			e.printStackTrace();
		}

        System.out.println(output);

        return output;
    }
    
    public static void main(String[] args) throws Exception {
    	HashMap<String, Object> output = new HashMap<>();
        output.put("tmpDir", "/tmp/sandbox-0");
        main(output);
    }
    
    public static Map<String, Object> jsonToMap(String jsonString) {
        try {
            if (jsonString != null && !jsonString.isEmpty()) {
                return JSON.std.mapFrom(jsonString);
            }
        } catch (IOException e) {
            e.printStackTrace(System.err);
        }
        return new HashMap<>();
    }

    /* For c-API invocations. */
    @CEntryPoint(name = "entrypoint")
    public static void main(IsolateThread thread, CCharPointer fin, CCharPointer fout, UnsignedWord foutLen) {
        String input = CTypeConversion.toJavaString(fin);
        Map<String, Object> map = jsonToMap(input);
        String output = main(map).toString();

        int len = Math.min((int) foutLen.rawValue() - 1, output.length());
        if (len > 0) {
            CTypeConversion.toCString(output.substring(0, len), fout, foutLen);
        }
    }

}
