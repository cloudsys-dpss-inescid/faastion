public class ZipCompression{

	static{
		System.loadLibrary("compress");
	}

	public native int compress(String input, String output);

	public static void main(String[] args){
		String file_location = System.getenv("INPUT_DIRECTORY");
		String out_file = "test.zip";
		new ZipCompression().compress(file_location,out_file);
	}

}
