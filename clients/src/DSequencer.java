import java.io.*;
import java.net.*;
import javax.json.Json;
import javax.json.JsonObject;
import javax.json.JsonReader;


public class DSequencer {
    public static final int SEQUENCER_DEFAULT_PORT = 5088;
    
    public static final int SEQUENCER_DEFAULT_READ_BYTES = 2048;
    
    public int port;
    
    public String host;
    
    Socket conn = null;
    
    PrintWriter writer = null;
    
    BufferedReader reader = null;
    
    public DSequencer(String host) {
        this.host = host;
        this.port = SEQUENCER_DEFAULT_PORT;
        
        this.connect();
    }
    
    public DSequencer(String host, int port) {
        this.host = host;
        this.port = port;
        
        this.connect();
    }
    
    protected void connect() {
        try {
            conn = new Socket(host, port);
            writer = new PrintWriter(conn.getOutputStream(), true);
            reader = new BufferedReader(new InputStreamReader(conn.getInputStream()));
        } catch (UnknownHostException e) {
            System.err.println("Could not connect to host: " + host);
            System.exit(1);
        } catch (IOException e) {
            System.err.println("Could not get I/O for the connection to: " + host);
            System.exit(1);
        }
    }
    
    protected void close() throws IOException {
        if (writer != null) {
            writer.close();
        }
        
        if (reader != null) {
            reader.close();
        }
        
        if (conn != null) {
            conn.close();
        }
    }
    
    protected JsonObject call(String cmd) throws IOException,DSequencerException {
    	//System.out.println("sending data22: " + cmd);
    	
    	writer.println(cmd);
    	
        String resp;
        
        while ((resp = reader.readLine()) != null) {
        	
        	JsonReader reader = Json.createReader(new StringReader(resp));
        	
        	JsonObject jObj = reader.readObject();
        	
        	if (jObj.getInt("success") == 1) {
        		return jObj;
        	}
        	
        	int code = 0;
        	if (jObj.containsKey("code")) {
        		code = jObj.getInt("code");
        	}
        	
        	String reason = "";
        	if (jObj.getString("error") != null) {
        		reason = jObj.getString("error");
        	}
            
        	throw new DSequencerException("_call() API failed. Reason: " + reason, code);
        }
        
        return null;
    }

    /**
     * get next value of the Sequence i.e. seq++
     *
     *   - null means no sequence does no exists
     *
     * @param String key
     * @return String|null
     * @throws DSequencerException 
     * @throws IOException 
     * @throws SequencerException
     */
    public String nextSequence(String key) throws IOException, DSequencerException {
        String in = "op=next&key=" + key;

        JsonObject resp = this.call(in);
        
        return (resp == null) ? null : resp.getString("data");
    }

    /**
     * get current value of a Sequence
     *
     *   - null means no sequence does no exists
     *
     * @param String key
     * @return String|null
     * @throws DSequencerException 
     * @throws IOException 
     * @throws SequencerException
     */
    public String getSequence(String key) throws IOException, DSequencerException {
        String in = "op=get&key=" + key;

        JsonObject resp = this.call(in);

        return (resp == null) ? null : resp.getString("data");
    }
    
    /**
     * set new or update existing Sequence
     *
     * @param String key
     * @param int value
     * @return boolean
     * @throws DSequencerException 
     * @throws IOException 
     * @throws SequencerException
     */
    public boolean setSequence(String key, int value) throws IOException, DSequencerException {
        String in = "op=set&key=" + key + "&value=" + value;

        JsonObject resp = this.call(in);
        
        return !(resp == null);
    }
    
    
    /**
     * reset existing Sequence to zero
     *
     * @param String key
     * @return boolean
     * @throws DSequencerException 
     * @throws IOException 
     * @throws SequencerException
     */
    public boolean resetSequence(String key) throws IOException, DSequencerException {
        String in = "op=reset&key=" + key;

        JsonObject resp = this.call(in);
        
        return !(resp == null);
    }
    
    
    /**
     * create new Sequence
     *
     * @param String key
     * @param int value
     * @return boolean
     * @throws DSequencerException 
     * @throws IOException 
     * @throws SequencerException
     */
    public boolean createSequence(String key, int value) throws IOException, DSequencerException {
        String in = "op=create&key=" + key + "&value=" + value;

        JsonObject resp = this.call(in);
        
        return !(resp == null);
    }

    /**
     * remove a Sequence
     *
     * @param String key
     * @return boolean
     * @throws DSequencerException 
     * @throws IOException 
     */
    public boolean removeSequence(String key) throws IOException, DSequencerException {
    	String in = "op=remove&key=" + key;

        JsonObject resp = this.call(in);
        
        return !(resp == null);
    }

    /**
     * get UUID
     *
     * @return String|null
     * @throws DSequencerException 
     * @throws IOException 
     * @throws SequencerException
     */
    public String getUUID() throws IOException, DSequencerException {
        String in = "op=uuid";
        
        JsonObject resp = this.call(in);
        
        return (resp == null) ? null : resp.getString("data");
    }
}
