
public class HelloWorld {

	static{
			System.loadLibrary("helloworld");
	}

	public native void helloworld();

	public static void main(String[] args){
		HelloWorld app = new HelloWorld();
		app.helloworld();
		}
	
}
