import com.criteo.vips.VipsContext;
import com.criteo.vips.VipsException;
import com.criteo.vips.VipsImage;
import com.criteo.vips.enums.VipsImageFormat;
import java.nio.file.Files;
import java.nio.file.StandardOpenOption;
import java.io.File;

import java.awt.Dimension;
import java.io.IOException;

public class SimpleExample {

    public static byte[] downloadBytes(String path) throws IOException {
            return Files.readAllBytes(new File(path).toPath());
    }

    public static void uploadBytes(byte[] bytes, String path) throws IOException {
            Files.write(new File(path).toPath(), bytes, StandardOpenOption.CREATE);
    }

    public static byte[] resize(byte[] bytes, float ratio) {
        try {
            VipsImage image = new VipsImage(bytes, bytes.length);
            int width = (int) (image.getWidth() * ratio);
            int height =(int) (image.getHeight() * ratio);
            image.thumbnailImage(new Dimension(width, height), true);
            bytes = image.writeToArray(VipsImageFormat.PNG, false);
            image.release();
            return bytes;
        } catch (VipsException e) {
            e.printStackTrace();
            return null;
        }
    }

    public static void main(String[] args) throws IOException {
        String iiPath = "snap.png";
        String oiPath = "snap-thumbnail.png";
        uploadBytes(resize(downloadBytes(iiPath), 0.25f), oiPath);
    }
}
