package com.demo_ni_osdcomms;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;

public class DemoObjects {

    public static final Random rng = new Random();

    public static String randomStr(int size) {
        byte[] bytes = new byte[size];
        rng.nextBytes(bytes);
        return new String(bytes);
    }

    public static class BigObj {
        public RInt8         field2     = new RInt8();
        public RString128    field5     = new RString128();
        public AList8        field8     = new AList8();
        public HMap8         field12    = new HMap8();

        public BigObj() { }
    }

    public static class RInt4 {
        public int field1 = rng.nextInt();
        public int field2 = rng.nextInt();
        public int field3 = rng.nextInt();
        public int field4 = rng.nextInt();

        public RInt4() { }
    }

    public static class RInt8 {
        public int field1 = rng.nextInt();
        public int field2 = rng.nextInt();
        public int field3 = rng.nextInt();
        public int field4 = rng.nextInt();
        public int field5 = rng.nextInt();
        public int field6 = rng.nextInt();
        public int field7 = rng.nextInt();
        public int field8 = rng.nextInt();

        public RInt8() { }
    }

    public static class RInt32 {
        public int field01 = rng.nextInt();
        public int field02 = rng.nextInt();
        public int field03 = rng.nextInt();
        public int field04 = rng.nextInt();
        public int field05 = rng.nextInt();
        public int field06 = rng.nextInt();
        public int field07 = rng.nextInt();
        public int field08 = rng.nextInt();
        public int field11 = rng.nextInt();
        public int field12 = rng.nextInt();
        public int field13 = rng.nextInt();
        public int field14 = rng.nextInt();
        public int field15 = rng.nextInt();
        public int field16 = rng.nextInt();
        public int field17 = rng.nextInt();
        public int field18 = rng.nextInt();
        public int field21 = rng.nextInt();
        public int field22 = rng.nextInt();
        public int field23 = rng.nextInt();
        public int field24 = rng.nextInt();
        public int field25 = rng.nextInt();
        public int field26 = rng.nextInt();
        public int field27 = rng.nextInt();
        public int field28 = rng.nextInt();
        public int field31 = rng.nextInt();
        public int field32 = rng.nextInt();
        public int field33 = rng.nextInt();
        public int field34 = rng.nextInt();
        public int field35 = rng.nextInt();
        public int field36 = rng.nextInt();
        public int field37 = rng.nextInt();
        public int field38 = rng.nextInt();

        public RInt32() { }
    }

    public static class RString64 {
        public String field1 = randomStr(64);

        public RString64() { }
    }

    public static class RString128 {
        public String field1 = randomStr(128);

        public RString128() { }
    }

    public static class RString256 {
        public String field1 = randomStr(256);

        public RString256() { }
    }

    public static class AList4 {
        public List<RInt4> field = new ArrayList<>(4);

        public AList4() {
            for (int i = 0; i < 4; i++) {
                field.add(new RInt4());
            }
        }
    }

    public static class AList8 {
        public List<RInt8> field = new ArrayList<>(8);

        public AList8() {
            for (int i = 0; i < 8; i++) {
                field.add(new RInt8());
            }
        }
    }

    public static class AList32 {
        public List<RInt32> field = new ArrayList<>(32);

        public AList32() {
            for (int i = 0; i < 32; i++) {
                field.add(new RInt32());
            }
        }
    }

    public static class AList64 {
        public List<RInt32> field = new ArrayList<>(64);

        public AList64() {
            for (int i = 0; i < 64; i++) {
                field.add(new RInt32());
            }
        }
    }

    public static class HMap4 {
        public Map<String, RInt4> field = new HashMap<>(4);

        public HMap4() {
            for (int i = 0; i < 4; i++) {
                field.put(randomStr(4), new RInt4());
            }
        }
    }

    public static class HMap8 {
        public Map<String, RInt4> field = new HashMap<>(8);

        public HMap8() {
            for (int i = 0; i < 8; i++) {
                field.put(randomStr(8), new RInt4());
            }
        }
    }

    public static class HMap32 {
        public Map<String, RInt4> field = new HashMap<>(32);

        public HMap32() {
            for (int i = 0; i < 32; i++) {
                field.put(randomStr(32), new RInt4());
            }
        }
    }

    public static class HMap64 {
        public Map<String, RInt4> field = new HashMap<>(64);

        public HMap64() {
            for (int i = 0; i < 64; i++) {
                field.put(randomStr(64), new RInt4());
            }
        }
    }
}
