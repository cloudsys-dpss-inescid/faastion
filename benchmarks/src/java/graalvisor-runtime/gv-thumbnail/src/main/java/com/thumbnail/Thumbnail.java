package com.thumbnail;

import java.io.IOException;
import java.net.URL;
import java.util.Map;
import java.util.HashMap;

import java.util.Random;
import java.nio.file.Path;
import java.util.stream.Collectors;

import java.io.File;
import java.awt.Image;
import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;


public class Thumbnail {

    public static BufferedImage scale(BufferedImage source, double ratio) {
        if (source == null) {
            return null;
        }
        int width = (int)(source.getWidth() * ratio);
        int height = (int)(source.getHeight() * ratio);
        int imageType = BufferedImage.TYPE_INT_ARGB;
        BufferedImage bi = new BufferedImage(width, height, imageType);
        Image result = source.getScaledInstance(width, height, Image.SCALE_SMOOTH);
        bi.createGraphics().drawImage(result, 0, 0, null);
        return bi;
    }

    public static boolean resize(String url, double ratio) throws IOException {
        BufferedImage img = ImageIO.read(new URL(url));
        BufferedImage bimg = scale(img, ratio);
        String filePath = "/tmp/img-"
                + new Random().ints(8, 0, 11)
                    .mapToObj(n -> String.valueOf(n))
                    .collect(Collectors.joining())
                + ".png";
        ImageIO.write(bimg, "png", new File(filePath));
        return true;
    }

    public static HashMap<String, Object> main(Map<String, Object> input) {
        HashMap<String, Object> output = new HashMap<>();
        String url = "http://127.0.0.1:8000/snap.png";
        double ratio = 0.25;
        boolean success = false;
        try {
            success = resize(url, ratio);
        } catch (IOException e) {
            e.printStackTrace();
        }
        output.put("success", success);
        return output;
    }

    public static void main(String[] args) {
        HashMap<String, Object> output = new HashMap<>();
        output = main(output);
        System.out.println(output);
    }
}
