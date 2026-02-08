--TEST--
xor_decrypt_test1() Basic test
--SKIPIF--
<?php
if (!extension_loaded('xor_decrypt')) {
	echo 'skip';
}
?>
--FILE--
<?php
$ret = xor_decrypt_test1();

var_dump($ret);
?>
--EXPECT--
The extension xor_decrypt is loaded and working!
NULL
