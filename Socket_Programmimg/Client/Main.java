package com.company;

import java.io.*;
import java.net.Socket;

public class Main {

    public static void main(String[] args) throws Exception {
        // write your code here
        Socket clientSocket=new Socket("localhost",6666);
        Socket fileSocket=new Socket("localhost",5000);
        System.out.println("Please Log in with your ID");
        send s = new send(clientSocket);
        Thread ts = new Thread(s);
        ts.start();
        receive r = new receive(clientSocket,fileSocket);
        Thread tr = new Thread(r);
        tr.start();
    }
}


