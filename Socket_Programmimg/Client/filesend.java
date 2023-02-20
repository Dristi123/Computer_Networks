package com.company;

import java.io.*;
import java.net.Socket;
import java.net.SocketTimeoutException;

public class filesend implements Runnable {
    Socket clientSocket;
    Socket textsocket;
    String nameoffile;
    int chunk;

    public filesend(Socket s, Socket s1,String filename,int chunk) throws Exception {
        this.clientSocket = s;
        this.textsocket=s1;
        this.nameoffile=filename;
        this.chunk=chunk;
        this.clientSocket.setSoTimeout(30000);
    }

    @Override
    public void run() {
        try {
            int flag = 0;
            PrintWriter toReceiver = new PrintWriter(clientSocket.getOutputStream(), true);
            PrintWriter toReceiver1 = new PrintWriter(textsocket.getOutputStream(), true);
            BufferedReader fromReceiver = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
            File file = new File(nameoffile);
            FileInputStream fis = new FileInputStream(file);
            BufferedInputStream bis = new BufferedInputStream(fis);
            OutputStream os = clientSocket.getOutputStream();
            byte[] contents;
            long fileLength = file.length();
            toReceiver.println(String.valueOf(fileLength));
            toReceiver.flush();
            long current = 0;

            while (current != fileLength) {
                int size = chunk;

                if (fileLength - current >= size)
                    current += size;
                else {
                    size = (int) (fileLength - current);
                    current = fileLength;
                }
                contents = new byte[size];
                bis.read(contents, 0, size);
                os.write(contents);
                try {
                    String confirmation = fromReceiver.readLine();
                }
                catch (SocketTimeoutException e) {
                    flag=1;
                    break;
                }

            }
            os.flush();
            if (flag == 1) {
                toReceiver1.println("Timeout "+fileLength);
                toReceiver.flush();
            } else
                {
                String str=fromReceiver.readLine();
                System.out.println(str);
                //toReceiver.println("Success");
                //System.out.println("File sent successfully!");
                }
        }
        catch(Exception e)
        {
            System.err.println("Could not transfer file.");
        }
    }
}
