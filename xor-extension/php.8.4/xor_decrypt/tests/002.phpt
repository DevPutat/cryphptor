--TEST--
test1() Basic test
--EXTENSIONS--
xor_decrypt
--FILE--
<?php
$ret = test1();

var_dump($ret);
?>
--EXPECT--
The extension xor_decrypt is loaded and working!
NULL
