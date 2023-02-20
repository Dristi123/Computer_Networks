package com.company;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.*;

public class Worker extends Thread {
    Socket socket;
    Socket filesocket;

    //ServerSocket ss;
    private String id;
    File dir;
    File dir2;
    Random r;
    String filepath = null;
    BufferedReader infromclient=null;
    DataOutputStream outtoclient=null;
    Thread filethread;
    Users user;
    FileOutputStream fs;
    List<String> resultspublic = new ArrayList<String>();
    List<String> resultsprivate = new ArrayList<String>();


    public Worker(Socket socket,Socket fs) throws IOException {
        this.socket = socket;
        this.filesocket=fs;
        this.infromclient = new BufferedReader(new InputStreamReader(this.socket.getInputStream()));
        this.outtoclient = new DataOutputStream(this.socket.getOutputStream());
        r=new Random();
    }
    public String getid() {
        return  id;
    }
    public void showfile(String id) {
        List<String> values = new ArrayList<String>();
        for (String item : Main.filemap.values()) {
            values.add(item);
        }

        for(int i=0;i<values.size();i++) {
            if(values.get(i).split("\\\\",3)[0].equals(id)) {
                String fid=null;
                for (Map.Entry<Integer, String> entry : Main.filemap.entrySet()) {
                    if (entry.getValue().equals(values.get(i))) {
                        //System.out.println(entry.getKey());
                        fid=entry.getKey().toString();
                    }
                }
                if(values.get(i).split("\\\\",3)[1].equals("Public Files")) {
                    resultspublic.add(values.get(i).split("\\\\",3)[2]+" (File ID:"+fid+")");
                }
                else resultsprivate.add(values.get(i).split("\\\\",3)[2]+" (File ID:"+fid+")");
            }
        }
    }
    public void run() {
        // buffers
        try {
            int flag=0;
            String msg = infromclient.readLine();
            id=msg;
            //System.out.println(id);
            for(int i=0;i<Main.loggedinids.size();i++) {
                if (Main.loggedinids.get(i).getid().equals(msg)) {
                    flag=1;
                }
            }
            if(flag==1) {
                outtoclient.writeBytes("You are already logged in from another client.Please log out first" + '\n');
                outtoclient.flush();
            }
            else {
                int f=0;
                for(int i=0;i<Main.atleastonceloggedin.size();i++) {
                    if(Main.atleastonceloggedin.get(i).getid().equals(id)) {
                        user=Main.atleastonceloggedin.get(i);
                        f=1;
                        break;
                    }
                }
                if(f==0) {
                    user=new Users(id);
                    Main.atleastonceloggedin.add(user);
                }
                //Users user=new Users(id);
                Main.loggedinids.add(user);

                dir =new File(id+"\\Public Files");
                dir2=new File(id+"\\Private Files");
                if(!dir.exists()) {
                    dir.mkdirs();
                }
                if(!dir2.exists()) {
                    dir2.mkdirs();
                }
                outtoclient.writeBytes("You have successfully logged in"+'\n');
                outtoclient.flush();
                while(true) {
                    outtoclient.writeBytes("What do you want to do?" + '\n');
                    outtoclient.writeBytes("1.Look up student list" + '\n');
                    outtoclient.writeBytes("2.Upload a file" + '\n');
                    outtoclient.writeBytes("3.Look up all your files" + '\n');
                    outtoclient.writeBytes("4.Look up public files of other students" + '\n');
                    outtoclient.writeBytes("5.Download a file" + '\n');
                    outtoclient.writeBytes("6.See unread messages" + '\n');
                    outtoclient.writeBytes("7.Request for a file" + '\n');
                    outtoclient.writeBytes("8.Log Out" + '\n');
                    String choice = infromclient.readLine();
                    //System.out.println(choice);
                    if (choice.equals("1")) {
                        outtoclient.writeBytes("IDs of currently logged-in students:" + '\n');
                        for (int i = 0; i < Main.loggedinids.size(); i++) {
                            outtoclient.writeBytes(Main.loggedinids.get(i).getid() + '\n');
                        }
                        outtoclient.writeBytes("IDs of students who logged-in atleast once:" + '\n');
                        for (int i = 0; i < Main.atleastonceloggedin.size(); i++) {
                            outtoclient.writeBytes(Main.atleastonceloggedin.get(i).getid() + '\n');
                        }
                     }
                    else if (choice.equals("2")) {
                        int rid=0;

                        outtoclient.writeBytes("Is it a requested file?" + '\n');
                        outtoclient.writeBytes("1.Yes or 2.No" + '\n');
                        String option = infromclient.readLine();
                        if (option.equals("1")) {
                            outtoclient.writeBytes("Provide Request ID" + '\n');
                            rid = Integer.valueOf(infromclient.readLine());
                            filepath = id + "\\Public Files";
                        }
                        else {
                            outtoclient.writeBytes("1.Public File? or 2.Private File?" + '\n');
                            String filechoice = infromclient.readLine();
                            //String filepath = null;
                            if (filechoice.equals("1")) filepath = id + "\\Public Files";
                            else filepath = id + "\\Private Files";
                        }
                        outtoclient.writeBytes("Please enter the name of the file" + '\n');
                        String name = infromclient.readLine();
                        outtoclient.writeBytes("Send Size " + name + '\n');
                        String strRecv = infromclient.readLine();
                        int filesize = Integer.parseInt(strRecv);
                        if ((filesize + Main.CURRENT_BUFFER_SIZE) > Main.MAX_BUFFER_SIZE) {
                            outtoclient.writeBytes("Buffer Overflow!Can't Send File."+'\n');
                        }
                        else
                            {
                            int chunk=r.nextInt(Main.MAX_CHUNK_SIZE-Main.MIN_CHUNK_SIZE)+Main.MIN_CHUNK_SIZE;
                            filepath = filepath + "\\" + name;
                            Main.i = Main.i + 1;
                            int fileid = Main.i;
                            fs=new FileOutputStream(filepath);
                            outtoclient.writeBytes("Send File " + chunk+" "+ name + '\n');
                            filethread=new Filereceiveserver(filesocket, filepath,rid,fileid,fs);
                            filethread.start();
                            //System.out.println("File saved successfully!baire");
                            }

                    }
                    else if(choice.equals("3")) {
                        showfile(id);
                        outtoclient.writeBytes("Your Public Files are:"+'\n');
                        if(resultspublic.isEmpty()) outtoclient.writeBytes("You dont have any public files"+"\n");
                        else {
                            for(int i=0;i<resultspublic.size();i++) {
                                outtoclient.writeBytes(resultspublic.get(i)+'\n');
                            }
                        }
                        outtoclient.writeBytes("Your Private Files are:"+'\n');
                        if(resultsprivate.isEmpty()) outtoclient.writeBytes("You dont have any private files"+"\n");
                        else {
                            for(int i=0;i<resultsprivate.size();i++) {
                                outtoclient.writeBytes(resultsprivate.get(i)+'\n');
                            }
                        }
                        resultsprivate.clear();
                        resultspublic.clear();
                    }
                    else if(choice.equals("4")) {
                        outtoclient.writeBytes("Enter the student ID" + '\n');
                        String id1 = infromclient.readLine();
                        int fl = 0;
                        for (int i = 0; i < Main.atleastonceloggedin.size(); i++) {
                            if (Main.atleastonceloggedin.get(i).getid().equals(id1)) {
                                fl = 1;
                                break;
                            }
                        }
                        if (fl == 0) outtoclient.writeBytes("Not a Valid ID" + '\n');
                        else {
                            showfile(id1);
                            outtoclient.writeBytes("Student ID: " + id1 + '\n');
                            outtoclient.writeBytes("Public Files:" + '\n');
                            if (resultspublic.isEmpty()) outtoclient.writeBytes("No File" + '\n');
                            else {
                                for (int j = 0; j < resultspublic.size(); j++) {
                                    outtoclient.writeBytes(resultspublic.get(j) + '\n');
                                }
                            }
                            resultspublic.clear();
                            resultsprivate.clear();
                        }
                    }
                    else if (choice.equals("5")) {
                        outtoclient.writeBytes("Please enter the file ID of your desired file"+'\n');
                        String dwldid=infromclient.readLine();
                        String dwdpath=Main.filemap.get(Integer.valueOf(dwldid));
                        String name=dwdpath.split("\\\\",3)[2];
                        File file=new File(dwdpath);
                        long filesize=file.length();
                        outtoclient.writeBytes("Download "+filesize+" "+Main.MAX_CHUNK_SIZE+" "+name+'\n');
                        Thread filethread2=new Filesendserver(filesocket,file);
                        filethread2.start();
                    }
                    else if (choice.equals("6")) {
                        outtoclient.writeBytes("Your unread messages:"+"\n");
                        if(user.messages.isEmpty()) outtoclient.writeBytes("No unread messages"+'\n');
                        else {
                            for (int i=0;i<user.messages.size();i++) {
                                outtoclient.writeBytes(user.messages.get(i)+'\n');
                            }
                        }
                        user.messages.clear();
                    }
                    else if (choice.equals("7")) {
                        outtoclient.writeBytes("Please provide a short description of the requested file"+'\n');
                        String reqdescription=infromclient.readLine();
                        Main.reqid=Main.reqid+1;
                        String finalreqdescription=id+"\\"+reqdescription;
                        Main.reqmap.put(Main.reqid,finalreqdescription);
                        for(int i=0;i<Main.atleastonceloggedin.size();i++) {
                            if(Main.atleastonceloggedin.get(i).getid().equals(id)) continue;
                            Main.atleastonceloggedin.get(i).messages.add(reqdescription+" (Request ID:"+Main.reqid+")");
                        }
                        outtoclient.writeBytes("Your request has been forwarded"+'\n');
                    }
                    else if(choice.split(" ")[0].equals("Timeout")) {
                        filethread.stop();
                        Main.CURRENT_BUFFER_SIZE=Main.CURRENT_BUFFER_SIZE-Integer.valueOf(choice.split(" ")[1]);
                        fs.close();
                        File file=new File(filepath);
                        file.delete();
                        outtoclient.writeBytes("TimeOut!Transmission aborted"+'\n');
                    }
                    else {
                        Main.loggedinids.remove(user);
                        outtoclient.writeBytes("You have logged out" + '\n');
                        break;
                    }
                }
            }
        }
        catch (IOException |NullPointerException e) {
            //e.printStackTrace();
            if(e instanceof IOException) {
                //System.out.println("heree");
                Main.loggedinids.remove(user);
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}
