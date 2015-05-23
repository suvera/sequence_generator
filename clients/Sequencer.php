<?php
/**
 * $Id: Sequencer.php rev 5/16/15 rnarmala $
 *
 * @author rnarmala
 */
define('SEQUENCER_DEFAULT_PORT', 5088);
define('SEQUENCER_DEFAULT_READ_BYTES', 2048);

class SequencerException extends Exception {}

class Sequencer {
    /**
     * @var string
     */
    private $_host;

    /**
     * @var string|int
     */
    private $_port = SEQUENCER_DEFAULT_PORT;

    /**
     * @var bool
     */
    private $_connected = false;

    /**
     * @var resource
     */
    private $_conn;

    /**
     * Constructor
     *
     * @param string $host
     * @param int $port
     */
    public function __construct($host, $port = SEQUENCER_DEFAULT_PORT) {
        $this->_host = $host;
        $this->_port = $port;

        $this->_connect();
    }

    /**
     * destructor
     */
    public function __destruct() {
        $this->close();
    }

    /**
     * close
     */
    public function close() {
        if ($this->_conn) {
            socket_close($this->_conn);
        }
    }

    /**
     * _connect
     *
     * @return bool
     * @throws SequencerException
     */
    private function _connect() {
        if ($this->_connected) {
            return true;
        }

        $this->_conn = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
        if ($this->_conn === false) {
            throw new SequencerException("socket_create() failed: Reason: " . socket_strerror(socket_last_error()) . "");
        }

        $result = socket_connect($this->_conn, $this->_host, $this->_port);
        if ($result === false) {
            throw new SequencerException("socket_connect() failed. Reason: ($result) " . socket_strerror(socket_last_error($this->_conn)) . "");
        }

        $this->_connected = true;

        return $this->_connected;
    }

    /**
     * get next value of the Sequence i.e. seq++
     *
     *   - NULL means no sequence does no exists
     *
     * @param string $key
     * @return string|null
     * @throws SequencerException
     */
    public function nextSequence($key) {
        $in = 'op=next&key=' . $key;

        $resp = $this->_call($in);
        
        return ($resp === false) ? NULL : $resp['data'];
    }

    /**
     * get current value of a Sequence
     *
     *   - NULL means no sequence does no exists
     *
     * @param string $key
     * @return string|null
     * @throws SequencerException
     */
    public function getSequence($key) {
        $in = 'op=get&key=' . $key;

        $resp = $this->_call($in);

        return ($resp === false) ? NULL : $resp['data'];
    }
    
    /**
     * call the API
     *
     * @param string $in - Command input
     * @return bool
     * @throws SequencerException
     */
    private function _call($in) {
        if (socket_write($this->_conn, $in, strlen($in)) === false) {
            throw new SequencerException("socket_connect() failed. Reason: " . socket_strerror(socket_last_error($this->_conn)) . "");
        }

        while ($out = socket_read($this->_conn, SEQUENCER_DEFAULT_READ_BYTES)) {
            $resp = json_decode($out, true);

            if ($resp['success']) {
                return $resp;
            }

            throw new SequencerException("_call() API failed. Reason: " . $resp['error'] . "", isset($resp['code']) ? $resp['code'] : 0);
        }

        return false;
    }

    /**
     * set new or update existing Sequence
     *
     * @param string $key
     * @param int $value
     * @return bool
     * @throws SequencerException
     */
    public function setSequence($key, $value = 0) {
        $in = 'op=set&key=' . $key . '&value=' . $value;

        $resp = $this->_call($in);
        
        return (bool) $resp;
    }
    
    
    /**
     * reset existing Sequence to zero
     *
     * @param string $key
     * @return bool
     * @throws SequencerException
     */
    public function resetSequence($key) {
        $in = 'op=reset&key=' . $key;

        $resp = $this->_call($in);
        
        return (bool) $resp;
    }
    
    
    /**
     * create new Sequence
     *
     * @param string $key
     * @param int $value
     * @return bool
     * @throws SequencerException
     */
    public function createSequence($key, $value = 0) {
        $in = 'op=create&key=' . $key . '&value=' . $value;

        $resp = $this->_call($in);
        
        return (bool) $resp;
    }

    /**
     * TODO:
     * remove a Sequence
     *
     * @param string $key
     * @return bool
     * @throws SequencerException
     */
    public function removeSequence($key) {
        $in = 'op=remove&key=' . $key;

        $resp = $this->_call($in);
        
        return (bool) $resp;
    }

    /**
     * get UUID
     *
     * @return string|null
     * @throws SequencerException
     */
    public function getUUID() {
        $in = 'op=uuid';
        
        $resp = $this->_call($in);
        
        return ($resp === false) ? NULL : $resp['data'];
    }
}

