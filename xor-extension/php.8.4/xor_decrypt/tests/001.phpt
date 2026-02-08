--TEST--
Check if xor_decrypt is loaded
--EXTENSIONS--
xor_decrypt
--FILE--
<?php
echo 'The extension "xor_decrypt" is available';
?>
--EXPECT--
The extension "xor_decrypt" is available
