package com.company;

import java.util.ArrayList;
import java.util.List;

public class Users {
    List<String> messages=new ArrayList<>();
    private String id;
    public Users(String id) {
        this.id=id;
    }
    public String getid() {
        return this.id;
    }
}
