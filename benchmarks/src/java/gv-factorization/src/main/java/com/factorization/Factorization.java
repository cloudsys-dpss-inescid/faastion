package com.factorization;

import java.util.HashMap;
import java.util.Map;


@SuppressWarnings("unused")
public class Factorization {

    public static HashMap<String, Object> main(Map<String, Object> input) {
	int number = 100000;
        HashMap<String, Object> output = new HashMap<>();
        if (number > 0) {
            System.out.print("1 " + number + " ");
        }

        for (int i = 2; i <= Math.sqrt(number); i++) {
            if (number % i == 0) {
                System.out.print(i + " ");
                if (i != number / i) {
                    System.out.print((number / i) + " ");
                }
            }
        }
        return output;
    }

    public static void main(String[] args) {
        HashMap<String, Object> output = new HashMap<>();
        output = main(output);
    }
}
