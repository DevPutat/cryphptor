--TEST--
Check if xor_decrypt is loaded
--SKIPIF--
<?php
if (!extension_loaded('xor_decrypt')) {
	echo 'skip';
}
?>
--FILE--
<?php
echo 'The extension "xor_decrypt" is available';
?>
--EXPECT--
The extension "xor_decrypt" is available
