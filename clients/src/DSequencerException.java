
@SuppressWarnings("serial")
public class DSequencerException extends Exception {
	protected int code = 0;
	
	public DSequencerException(String message, int code) {
		super(message);
		this.code = code;
	}
	
	public int getCode() {
		return code;
	}
}
