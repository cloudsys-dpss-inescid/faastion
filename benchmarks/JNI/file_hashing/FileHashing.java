import java.io.File;
import java.io.FileNotFoundException;
import java.util.Scanner;

public class FileHashing{

	static{
		System.loadLibrary("hash");
	}

	public native String hashing(String line);

	public static void main(String[] args){
		String line;
		int transitions = 0;
		String filePath = "./hashing.txt";
		try{
			File file = new File(filePath);
			Scanner reader = new Scanner(file);
			while(reader.hasNextLine()){
				System.out.println(transitions);
				transitions++;
				String data = reader.nextLine();
				long start = System.nanoTime();
				String hash = new file_hashing().hashing(data);
				long end = System.nanoTime();
				System.out.println(((double)end - start) / 1000000000);
			}
			reader.close();
		}catch(FileNotFoundException e){
			e.printStackTrace();
		}
	}
}
