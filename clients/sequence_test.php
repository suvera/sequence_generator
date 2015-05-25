<?php
/**
 * $Id: sequence_test.php rev 5/16/15 rnarmala $
 *
 * @author rnarmala
 */
require_once(__DIR__ . '/Sequencer.php');

$key = 'key_' . mt_rand(1, 150);

echo "Using key '$key' ...\n";
$time = microtime(true);
$obj = new Sequencer('127.0.0.1');
echo "Sequence Connection: " . number_format(round(microtime(true) - $time, 8), 8) . "\n";

$time = microtime(true);
try {
    echo 'createSequence: ' . $obj->createSequence($key, 100) . "\n";
} catch(Exception $ex) {
    echo "$key " . $ex->getMessage() . "\n";
}
echo "Sequence Create: " . number_format(round(microtime(true) - $time, 8), 8) . "\n";

$limit = 100000;

$time = microtime(true);
for ($i = 0; $i < $limit; $i++) {
    $seq = $obj->nextSequence($key);
    //echo 'nextSequence: ' . $seq . "\n";
}
echo "Sequence Time (Calls: $limit): " . number_format(round(microtime(true) - $time, 8), 8) . "\n";


$time = microtime(true);
for ($i = 0; $i < $limit; $i++) {
    $seq = $obj->getUUID();
    //echo 'getUUID: ' . $seq . "\n";
}
echo "UUID Time (Calls: $limit): " .number_format(round(microtime(true) - $time, 8), 8) . "\n";

