package com.company;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class Main {
    public static List<Users> loggedinids=new ArrayList<>();
    public static List<Users> atleastonceloggedin=new ArrayList<>();
    public static HashMap<Integer,String> filemap= new HashMap<>();
    public static HashMap<Integer,String> reqmap= new HashMap<>();
    public static int MAX_BUFFER_SIZE=1000000000;
    public static int MIN_CHUNK_SIZE=1000;
    public static int MAX_CHUNK_SIZE=5000;
    public static int CURRENT_BUFFER_SIZE=0;
    public static int i=0;
    public static int reqid=0;
    public static void main(String[] args) throws IOException, ClassNotFoundException {
        ServerSocket welcomeSocket = new ServerSocket(6666);
        ServerSocket ss=new ServerSocket(5000);



        while(true) {
            System.out.println("Waiting for connection...");
            Socket socket = welcomeSocket.accept();
            System.out.println("Connection established");
            Socket filesocket=ss.accept();
            Thread worker = new Worker(socket,filesocket);
            worker.start();
        }

    }
}