package com.company;

import java.io.*;
import java.net.Socket;

public class Filesendserver extends Thread {
    Socket clientSocket2;
    File f;
    PrintWriter pw;
    int max_chunk;
    //DataOutputStream outtoclient=null;
    public Filesendserver(Socket s, File file) throws Exception {
        this.clientSocket2 = s;
        //this.nameoffile=filename;
        f=file;
        pw=new PrintWriter(this.clientSocket2.getOutputStream(),true);
        //outtoclient=out;
        max_chunk=Main.MAX_CHUNK_SIZE;
    }

    @Override
    public void run() {
        try {
            FileInputStream fis = new FileInputStream(f);
            BufferedInputStream bis = new BufferedInputStream(fis);
            OutputStream os = clientSocket2.getOutputStream();
            byte[] contents;
            long fileLength = f.length();
            //outToServer.println(String.valueOf(fileLength));

            long current = 0;

            while(current!=fileLength){
                int size = max_chunk;
                if(fileLength - current >= size)
                    current += size;
                else{
                    size = (int)(fileLength - current);
                    current = fileLength;
                }
                contents = new byte[size];
                bis.read(contents, 0, size);
                os.write(contents);
            }
            os.flush();
            Thread.sleep(500);
            pw.println("File Downlaod Successful");
            pw.flush();
            //outtoclient.writeBytes("Down Complete"+'\n');
            System.out.println("File sent successfully!");
        }
        catch(Exception e)
        {
            System.err.println("Could not transfer file.");
        }
    }
}