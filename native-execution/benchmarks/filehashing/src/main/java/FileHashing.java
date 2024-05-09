import java.io.File;
import java.io.FileNotFoundException;
import java.util.Scanner;

public class FileHashing{

	static{
		System.loadLibrary("hash");
	}

	public native String hash(String line);

	public static void main(String[] args){
		String file_location = System.getenv("INPUT_FILE");
		try{
			File file = new File(file_location);
			Scanner reader = new Scanner(file);
			while(reader.hasNextLine()){
				String data = reader.nextLine();
				new FileHashing().hash(data);
			}
			reader.close();
		}catch(FileNotFoundException e){
			e.printStackTrace();
		}
	}
}
