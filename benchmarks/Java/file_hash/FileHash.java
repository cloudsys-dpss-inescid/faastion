import java.io.File;
import java.io.FileNotFoundException;
import java.math.BigInteger;
import java.util.Scanner;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class FileHash{
	
	public String encryptString (String input) throws NoSuchAlgorithmException {

		MessageDigest md = MessageDigest.getInstance("SHA-256");
		byte[] messageDigest = md.digest(input.getBytes());
		BigInteger big = new BigInteger(1,messageDigest);
		return big.toString(16);
	}	

	public static void main(String args[]) throws NoSuchAlgorithmException {
	String inputFilePath = "./test_file.txt";
	FileHash fh = new FileHash();
    File file = new File(inputFilePath);	
	try {
            Scanner scanner = new Scanner(file);
            while (scanner.hasNextLine()) {
                String line = scanner.nextLine();
				System.out.println(fh.encryptString(line));
            }
            scanner.close();
        } catch (FileNotFoundException e) {
            System.err.println("File not found");
            e.printStackTrace();
        }



	}

}
