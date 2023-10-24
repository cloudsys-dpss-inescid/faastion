import random
import sys
import string 

def sentence_generator(length):
    return ''.join(random.choice(string.ascii_letters) for _ in range(length))

#file_name = "hashing.txt"
#with open(file_name, "w") as file:
for _ in range(int(sys.argv[1])):
    print(sentence_generator(int(sys.argv[2])))
    

