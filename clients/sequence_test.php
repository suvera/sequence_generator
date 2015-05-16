<?php
/**
 * $Id: sequence_test.php rev 5/16/15 rnarmala $
 *
 * @author rnarmala
 */
require_once(__DIR__ . '/Sequencer.php');

$time = microtime(true);
$obj = new Sequencer('127.0.0.1');
echo 'setSequence: ' . $obj->setSequence('rama', 100) . "\n";
echo "Sequence Connection: " . number_format(round(microtime(true) - $time, 8), 8) . "\n";

$limit = 100000;

$time = microtime(true);
for ($i = 0; $i < $limit; $i++) {
    $seq = $obj->nextSequence('rama');
    //echo 'nextSequence: ' . $seq . "\n";
}
echo "Sequence Time: " . number_format(round(microtime(true) - $time, 8), 8) . "\n";

/*
$time = microtime(true);
for ($i = 0; $i < $limit; $i++) {
    $seq = $obj->getUUID();
    //echo 'getUUID: ' . $seq . "\n";
}
echo "UUID Time: " .number_format(round(microtime(true) - $time, 8), 8) . "\n";
*/
