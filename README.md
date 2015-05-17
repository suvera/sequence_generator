# Sequence Generator

Sequence Generator allows to generate Integer Sequences and UUIDs in a Distributed environment.

# Installation

This is designed for Linux platform only, other platforms may not work!

## Pre-Requisites

1. libuuid
  - *sudo apt-get install uuid uuid-dev*
3. C++11

## Steps to Install

1. git clone this repo.
  - *git clone --recursive https://github.com/suvera/sequence_generator.git*
  - *cd sequence_generator*
  - *git submodule update --init --recursive*

2. Default installation. 
  - *make install*

3. Install with custom configuration. Please check "sequencer.ini" for example configuration.
  - make config=/path/to/ini/seq.ini install

3. Install into specific folder
  - make prefix=/path/to/custom install

4. Install into specific folder with custom configuration
  - make prefix=/path/to/custom config=/path/to/ini/seq.ini install


**Start the sequencer daemon**

- /usr/local/bin/sequencer start

**Stop the sequencer**

- /usr/local/bin/sequencer stop


# How to use it

There are some example clients written under 'clients' folder

**PHP**
```
require_once(__DIR__ . '/Sequencer.php');

$key = 'page_usage_hits';

$obj = new Sequencer('127.0.0.1');
// OR with port
// $obj = new Sequencer('127.0.0.1', 5088);


// one time create, no need to create again and again
try {
  $obj->createSequence($key, 0);
} catch (SequencerException $ex) {
  // will throw an erro if sequence already exists
}

// Now use it in your Application whenever needed.


// print 1
$seq = $obj->nextSequence($key);
echo 'nextSequence: ' . $seq . "\n";

// print 2
$seq = $obj->nextSequence($key);
echo 'nextSequence: ' . $seq . "\n";

// print 3
$seq = $obj->nextSequence($key);
echo 'nextSequence: ' . $seq . "\n";


// Get UUID
$uuid = $obj->getUUID();
echo 'UUID: ' . $uuid . "\n";

```







