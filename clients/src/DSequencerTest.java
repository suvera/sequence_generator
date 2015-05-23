import java.io.IOException;
public class DSequencerTest {

	/**
	 * @param args
	 * @throws DSequencerException 
	 * @throws IOException 
	 */
	public static void main(String[] args) throws IOException, DSequencerException {
		int limit = 100000;
		
		long startTime = System.currentTimeMillis();
		
		DSequencer seq = new DSequencer("127.0.0.1");
		
		long stopTime = System.currentTimeMillis();
	    long elapsedTime = stopTime - startTime;
	    System.out.println("Connection Time: " + ((elapsedTime+0.0000000)/1000) + " secs");
	    
	    
		String key = "key_" + (int) Math.floor(10 * Math.random());
		
		
		startTime = System.currentTimeMillis();
		try {
			seq.createSequence(key, 0);
		} catch (DSequencerException ex) {
			// this is okay
			System.out.println(ex.getMessage());
		}
		
		stopTime = System.currentTimeMillis();
	    elapsedTime = stopTime - startTime;
	    System.out.println("Sequence Key Creation: " + ((elapsedTime+0.0000000)/1000) + " secs");
		
		
		
		startTime = System.currentTimeMillis();
		
		@SuppressWarnings("unused")
		String data = "";
		for (int i = 0; i < limit; i++) {
			data = seq.nextSequence(key);
		}
		
		stopTime = System.currentTimeMillis();
	    elapsedTime = stopTime - startTime;
	    System.out.println("Sequence Time (Calls: " + limit + "): " + ((elapsedTime+0.0000000)/1000) + " secs");
	    
	    
	    startTime = System.currentTimeMillis();
		
		for (int i = 0; i < limit; i++) {
			data = seq.getUUID();
		}
		
		stopTime = System.currentTimeMillis();
	    elapsedTime = stopTime - startTime;
	    System.out.println("UUID Time (Calls: " + limit + "): " + ((elapsedTime+0.0000000)/1000) + " secs");
	}

}
