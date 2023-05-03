/*
 * Copyright 2020-2021 the original author or authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package micronaut.benchmark.shopcart;

import io.micronaut.runtime.Micronaut;
import java.util.HashMap;
import java.util.Map;
import java.util.Collections;


public class Application {

    private static boolean INITIALIZED = false; 

    public static void main(String[] args) {
        Micronaut.run(Application.class);
    }

    public static Map<String, Object> main(Map<String, Object> input) {
        Map<String, Object> output = new HashMap<>();

        synchronized (Application.class) {
            if (!INITIALIZED) {
                Micronaut.build(new String[0]).properties(Collections.singletonMap("micronaut.server.port", "-1")).start();
                INITIALIZED = true;
                output.put("Log", "Hello from shopcart! (initialized)");
            } else {
                output.put("Log", "Hello from shopcart! (already initialized)");
            }
        }

        return output;
    } 
}
