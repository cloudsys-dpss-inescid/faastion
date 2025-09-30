package com.factorization;

import java.util.HashMap;
import java.util.Map;


@SuppressWarnings("unused")
public class Factorization {

    private static final int MAX_FACTORS = 500;

    public static HashMap<String, Object> main(Map<String, Object> input) {
        HashMap<String, Object> output = new HashMap<>();

        int[] num_factors = new int[MAX_FACTORS];
        int count = 0;
	    int number = 10000000;

        for (int i = 1; i <= number; i++) {
            if (count == MAX_FACTORS) {
                break;
            }
            if ((number % i) == 0) {
                num_factors[count++] = i;
            }
        }

        return output;
    }

    public static void main(String[] args) {
        HashMap<String, Object> output = new HashMap<>();
        output = main(output);
    }
}
