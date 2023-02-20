package com.company;

import java.io.*;
import java.net.Socket;

class receive implements Runnable {
    public static Socket clientSocket;
    public static Socket clientfilesocket;

    public receive(Socket s,Socket fs) throws Exception {
        this.clientSocket = s;
        this.clientfilesocket=fs;
    }

    @Override
    public void run() {
        try {
            String modifiedSentence;
            BufferedReader inFromServer = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
            while (true) {
                modifiedSentence = inFromServer.readLine();

                if(modifiedSentence.equals("You are already logged in from another client.Please log out first")||modifiedSentence.equals("You have logged out")) {
                    System.out.println(modifiedSentence);
                    clientSocket.close();
                    System.exit(0);
                }
                else if(modifiedSentence.split(" ",4)[0].equals("Send") && modifiedSentence.split(" ",4)[1].equals("File")) {
                        //Socket clientfilesocket=new Socket("localhost",5000);
                        filesend fs=new filesend(clientfilesocket,clientSocket,modifiedSentence.split(" ",4)[3],Integer.valueOf(modifiedSentence.split(" ",4)[2]));
                        Thread tfs= new Thread(fs);
                        tfs.start();
                    }
                else if(modifiedSentence.split(" ",3)[0].equals("Send") && modifiedSentence.split(" ",3)[1].equals("Size")) {
                    PrintWriter outToServer = new PrintWriter(clientSocket.getOutputStream(), true);
                    File file=new File(modifiedSentence.split(" ",3)[2]);
                    long fileLength = file.length();
                    outToServer.println(String.valueOf(fileLength));
                }
                else if(modifiedSentence.split(" ",4)[0].equals("Download")) {
                    String filepath="Downloads"+"\\"+modifiedSentence.split(" ",4)[3];
                    int filesize=Integer.valueOf(modifiedSentence.split(" ",4)[1]);
                    int chunksize=Integer.valueOf(modifiedSentence.split(" ",4)[2]);
                    Thread tfr=new filereceive(clientfilesocket,filepath,filesize,chunksize);
                    tfr.start();
                }
                else System.out.println(modifiedSentence);
                }

        } catch (IOException e) {
            e.printStackTrace();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
