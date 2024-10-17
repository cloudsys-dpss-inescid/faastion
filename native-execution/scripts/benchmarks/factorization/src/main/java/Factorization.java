import java.util.Random;

public class Factorization{

	static{
			System.loadLibrary("factorization");
	}

	public native int factor(int num);

	public static void main(String[] args){
		Factorization app = new Factorization();
		Random random = new Random();

		// Generate and process 10 random integers between 1 and 1000
		for (int i = 0; i < 10; i++) {
				int number = random.nextInt(1000) + 1; // Generate a number between 1 and 1000
				
				// Call the native method to factorize the number and get the count of factors
				int factorCount = app.factor(number);
				System.out.println("Random number: " + number + ", Number of factors: " + factorCount);
		}
	}
}
