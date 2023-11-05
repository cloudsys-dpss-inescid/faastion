public class ImageManipulation {

        static{
                System.loadLibrary("image");
        }

        public native int manipulation(String progam, String input, String output);

        public static void main(String[] args){
		String program = "ImageManipulation";		
		String file_location = System.getenv("INPUT_IMAGE");
                String out_file = "manip_scooter.jpeg";
                new ImageManipulation().manipulation(program,file_location,out_file);
        }

}
