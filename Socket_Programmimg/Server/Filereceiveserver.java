package com.company;

import javax.naming.Context;
import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;

public class Filereceiveserver extends Thread{
    Socket socket;
    FileOutputStream fos=null;
    BufferedOutputStream bos=null;
    InputStream is=null;
    int requesterid=0;
    int filesize=0;
    int fid;
    String path;



    public Filereceiveserver(Socket socket, String path,int id,int fid,FileOutputStream fs) throws IOException {
        this.socket=socket;
        this.fos = fs;
        bos = new BufferedOutputStream(fos);
        is = socket.getInputStream();
        requesterid=id;
        this.fid=fid;
        this.path=path;

    }
    public  void run() {
        try {
            byte[] contents = new byte[Main.MAX_CHUNK_SIZE];
            int flag=0;
            BufferedReader fromSender = new BufferedReader(new InputStreamReader(this.socket.getInputStream()));
            PrintWriter toSender = new PrintWriter(this.socket.getOutputStream(), true);
            String strRecv = fromSender.readLine();
            filesize=Integer.parseInt(strRecv);
            byte[] fullcontents=new byte[filesize];
            //System.out.println(filesize);
            int bytesRead = 0;
            int total=0;
            int fullindex=0;
            Main.CURRENT_BUFFER_SIZE=Main.CURRENT_BUFFER_SIZE+filesize;
            while(total!=filesize)
            {
                bytesRead=is.read(contents);
                //System.out.println(bytesRead);
                total+=bytesRead;
                for(int i=0;i<bytesRead;i++) {
                    fullcontents[fullindex]=contents[i];
                    fullindex++;
                }
                //Thread.sleep(50000);
                toSender.println("Chunk received");
                toSender.flush();
            }
            if(fullindex==filesize) {
                bos.write(fullcontents, 0, fullindex);
                toSender.println("File Upload Successful");
                Main.filemap.put(fid,path);
            }
            else {
                toSender.println("Error!!");
                filedelete();
            }
            System.out.println("File saved Successfully");
            if(requesterid!=0 && fullindex==filesize) {
                String requester=Main.reqmap.get(requesterid).split("\\\\",2)[0];
                for(int i=0;i<Main.atleastonceloggedin.size();i++) {
                    if(Main.atleastonceloggedin.get(i).getid().equals(requester)) {
                        Main.atleastonceloggedin.get(i).messages.add("Your requested file (File ID:"+requesterid+") has been uploaded");
                    }
                }

            }
            bos.flush();
            fos.close();
            bos.close();
            Main.CURRENT_BUFFER_SIZE=Main.CURRENT_BUFFER_SIZE-filesize;

        }
        catch(Exception e)
        {
            System.err.println("Could not transfer file.");
            try {
                filedelete();
            } catch (IOException ioException) {
                ioException.printStackTrace();
            }
            Main.CURRENT_BUFFER_SIZE=Main.CURRENT_BUFFER_SIZE-filesize;
        }
    }
    public void filedelete() throws IOException {
        fos.close();
        File f=new File(path);
        f.delete();
    }
}
