package com.company;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

class send implements Runnable {
    public Socket clientSocket;

    public send(Socket s) throws Exception {
        this.clientSocket = s;
    }

    @Override
    public void run() {
        try {
            BufferedReader inFromUser = new BufferedReader(new InputStreamReader(System.in));
            String choice = null;
            PrintWriter outToServer = new PrintWriter(clientSocket.getOutputStream(), true);
            String id=null;
            id=inFromUser.readLine();
            outToServer.println(id);
            while (true) {
                choice = inFromUser.readLine();
                outToServer.println(choice);
                //System.out.println("hereee");
            }
        } catch (IOException e) {
            e.printStackTrace();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
