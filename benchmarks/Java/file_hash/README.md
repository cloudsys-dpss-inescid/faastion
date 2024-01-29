#Introduction

Running make inside the FileHash directory does the follow:

1) The file_generator.py is utilized in order to generate a test_file needed for hashing.
   Length and width of the test file in terms of character can be set in the MakeFile, where
   first arg is the number of lines in the file and second is the number of characters in a line. 

2) The generated test file is then used by the FileHash.java to generate a SHA-256 hash using java.security library. 
   This output is then redirected to another file. 
