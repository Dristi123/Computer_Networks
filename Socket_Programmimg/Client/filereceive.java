package com.company;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketTimeoutException;

public class filereceive extends Thread{
    Socket socket;
    FileOutputStream fos=null;
    BufferedOutputStream bos=null;
    InputStream is=null;
    int chunk_size;
    int filesize;
    BufferedReader infromserver;

    public filereceive(Socket socket, String path,int size,int cs) throws IOException {
        this.socket=socket;
        fos = new FileOutputStream(path);
        bos = new BufferedOutputStream(fos);
        is = socket.getInputStream();
        this.filesize=size;
        chunk_size=cs;
        infromserver=new BufferedReader(new InputStreamReader(this.socket.getInputStream()));
    }
    public  void run() {
        try {
            byte[] contents = new byte[chunk_size];
            int bytesRead = 0;
            int total=0;
            while(total!=filesize)
            {
                bytesRead=is.read(contents);
                total+=bytesRead;
                bos.write(contents, 0, bytesRead);
                //bos.flush();
            }
            bos.flush();
            fos.close();
            bos.close();
            String r=infromserver.readLine();
            System.out.println(r);
            //System.out.println("File Downlaod Successful");
        }
        catch(Exception e)
        {
            e.printStackTrace();
            System.err.println("Could not transfer file.");
        }
    }
}