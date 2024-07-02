package com.classify;

import org.tensorflow.Graph;
import org.tensorflow.Session;
import org.tensorflow.Tensor;

import java.awt.Image;
import java.awt.image.BufferedImage;
import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class InceptionImageClassifier implements AutoCloseable {

    private Graph graph = new Graph();
    private List<String> labels = new ArrayList<>();

    private static BufferedImage resizeImage(BufferedImage img, int imgWidth, int imgHeight) {
        if(img.getWidth() != imgWidth || img.getHeight() != imgHeight) {
            Image newImg = img.getScaledInstance(imgWidth, imgHeight, Image.SCALE_SMOOTH);
            BufferedImage newBufferedImg = new BufferedImage(newImg.getWidth(null),
                    newImg.getHeight(null),
                    BufferedImage.TYPE_INT_RGB);
            newBufferedImg.getGraphics().drawImage(newImg, 0, 0, null);
            return newBufferedImg;
        }
        return img;
    }
    
    private static byte[] getBytes(InputStream is) throws IOException {
        ByteArrayOutputStream mem = new ByteArrayOutputStream();
        byte[] buffer = new byte[1024];
        int len = 0;
        while((len = is.read(buffer, 0, 1024)) > 0){
            mem.write(buffer, 0, len);
        }
        return mem.toByteArray();
    }
    
    public void load_model(InputStream inputStream) throws IOException {
        byte[] bytes = getBytes(inputStream);
        graph.importGraphDef(bytes);
    }

    public void load_labels(InputStream inputStream) {
        labels.clear();
        try(BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream))) {
            String line;
            while((line = reader.readLine()) != null) {
                labels.add(line);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public String predict_image(BufferedImage image) {
        return predict_image(image, 224, 224);
    }

    public String predict_image(BufferedImage image, int imgWidth, int imgHeight) {
        int argmax = 0;

        image = resizeImage(image, imgWidth, imgHeight);

        Tensor<Float> imageTensor = TensorUtils.getImageTensorNormalized(image, imgWidth, imgHeight);

        try (Session sess = new Session(graph);
             Tensor<Float> result =
                     sess.runner().feed("input", imageTensor)
                             .fetch("output").run().get(0).expect(Float.class)) {
            final long[] rshape = result.shape();
            if (result.numDimensions() != 2 || rshape[0] != 1) {
                imageTensor.close();
                throw new RuntimeException(
                        String.format(
                                "Expected model to produce a [1 N] shaped tensor where N is the number of labels, instead it produced one with shape %s",
                                Arrays.toString(rshape)));
            }
            int nlabels = (int) rshape[1];
            float[] predicted = result.copyTo(new float[1][nlabels])[0];

            float max = predicted[0];
            for(int i=1; i < nlabels; ++i) {
                if(max < predicted[i]) {
                    max = predicted[i];
                    argmax = i;
                }
            }
        } catch(Exception e) {
            imageTensor.close();
            e.printStackTrace();
        }

        imageTensor.close();

        if(argmax < labels.size()) {
            return labels.get(argmax);
        }

        return "unknown";
    }

    @Override
    public void close() throws Exception {
        if(graph != null) {
            graph.close();
            graph = null;
        }
    }
}
