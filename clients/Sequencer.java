import java.io.*;
import java.net.*;

class SequencerException extends Exception {
}

public class Sequencer {
    public static final int SEQUENCER_DEFAULT_PORT = 5088;
    
    public int port;
    
    public String host;
    
    Socket conn = null;
    
    OutputStreamWriter writer = null;
    
    BufferedReader reader = null;
    
    public Sequencer(String host) {
        this.host = host;
        this.port = SEQUENCER_DEFAULT_PORT;
        
        this.connect();
    }
    
    public Sequencer(String host, int port) {
        this.host = host;
        this.port = port;
        
        this.connect();
    }
    
    protected void connect() {
        try {
            conn = new Socket(host, port);
            writer = new OutputStreamWriter(conn.getOutputStream(), "UTF-8");
            reader = new BufferedReader(new InputStreamReader(conn.getInputStream()));
        } catch (UnknownHostException e) {
            System.err.println("Could not connect to host: " + host);
            System.exit(1);
        } catch (IOException e) {
            System.err.println("Could not get I/O for the connection to: " + host);
            System.exit(1);
        }
    }
    
    protected void close() {
        if (writer) {
            writer.close();
        }
        
        if (reader) {
            reader.close();
        }
        
        if (conn) {
            conn.close();
        }
    }
    
    protected String call(String cmd) throws SequencerException {
        try {
            writer.write(cmd, 0, cmd.length());
        } catch (Exception ex) {
            throw new SequencerException(ex);
        }
        
        String resp;
        while ((resp = reader.readLine()) != null) {
            return resp;
        }
        
        return null;
    }
}
