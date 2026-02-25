<?php
/**
 * XOR Decrypt Helper
 * 
 * Вспомогательные функции для XOR-шифрования.
 * Основное расширение загружается автоматически через php.ini.
 * 
 * Использование:
 *   1. Установите XOR_DECRYPT_KEY в environment
 *   2. Все .php файлы будут расшифровываться автоматически
 */

/**
 * XOR encrypt/decrypt function (symmetric)
 */
if (!function_exists('xor_crypt_string')) {
function xor_crypt_string(string $data, string $key): string
{
    $result = '';
    $keyLen = strlen($key);
    $dataLen = strlen($data);
    
    for ($i = 0; $i < $dataLen; $i++) {
        $result .= $data[$i] ^ $key[$i % $keyLen];
    }
    
    return $result;
}
}

/**
 * Decrypt data
 */
if (!function_exists('xor_decrypt_string')) {
function xor_decrypt_string(string $data, string $key): string
{
    return xor_crypt_string($data, $key);
}
}

/**
 * Encrypt data
 */
if (!function_exists('xor_encrypt_string')) {
function xor_encrypt_string(string $data, string $key): string
{
    return xor_crypt_string($data, $key);
}
}
