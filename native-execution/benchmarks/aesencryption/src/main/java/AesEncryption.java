public class AesEncryption{

        static{
                System.loadLibrary("encrypt");
        }

        public native int aes();

        public static void main(String[] args){
                new AesEncryption().aes();
        }

}
