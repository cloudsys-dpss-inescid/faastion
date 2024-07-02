package org.graalvm.argo.dataset;

import java.util.HashSet;
import java.util.Set;

public class Owner {
    private final String ownerHash;
    private final Set<String> functions;
    private int invocations;

    public Owner(String hash) {
        this.ownerHash = hash;
        this.functions = new HashSet<>();
        this.invocations = 0;
    }

    public void addInvocations(int increment) {
        this.invocations += increment;
    }

    public void addFunction(String function) {
        this.functions.add(function);
    }

    public int getInvocations() {
        return this.invocations;
    }

    public int getFunctions() {
        return this.functions.size();
    }

    public String getOwnerHash() {
        return ownerHash;
    }

    @Override
    public String toString() {
        return "Owner{" +
                "ownerHash='" + ownerHash + '\'' +
                ", functions=" + functions.size() +
                ", invocations=" + invocations +
                '}';
    }
}
