package main

import (
	"fmt"
	"os"
	"path/filepath"
)

func main() {
	// Получаем путь к директории проекта
	projectDir, err := os.Getwd()
	if err != nil {
		fmt.Printf("Ошибка получения текущей директории: %v\n", err)
		os.Exit(1)
	}

	// Определяем путь к директории с расширением
	phpExtDir := filepath.Join(projectDir, "internal", "phpext")

	// Проверяем, что директория существует
	if _, err := os.Stat(phpExtDir); os.IsNotExist(err) {
		fmt.Printf("Директория с расширением не найдена: %s\n", phpExtDir)
		os.Exit(1)
	}

	// Создаем директорию для сборки
	buildDir := filepath.Join(projectDir, "dist", "phpext")
	if err := os.MkdirAll(buildDir, 0755); err != nil {
		fmt.Printf("Ошибка создания директории сборки: %v\n", err)
		os.Exit(1)
	}

	// Копируем файлы расширения в директорию сборки
	src := filepath.Join(phpExtDir, "extention_impl.c")
	dst := filepath.Join(buildDir, "extention.c")
	if err := copyFile(src, dst); err != nil {
		fmt.Printf("Ошибка копирования файла extention_impl.c: %v\n", err)
		os.Exit(1)
	}

	// Создаем config.m4 файл для PHP extension
	configM4 := `PHP_ARG_ENABLE(cryphptor, whether to enable cryphptor support,
[  --enable-cryphptor          Enable cryphptor support])

if test "$PHP_CRYPHPTOR" != "no"; then
  PHP_NEW_EXTENSION(cryphptor, cryphptor.c extention.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
  PHP_ADD_LIBRARY(ssl, 1, CRYPHPTOR_SHARED_LIBADD)
  PHP_ADD_LIBRARY(crypto, 1, CRYPHPTOR_SHARED_LIBADD)
  PHP_SUBST(CRYPHPTOR_SHARED_LIBADD)
fi`

	if err := os.WriteFile(filepath.Join(buildDir, "config.m4"), []byte(configM4), 0644); err != nil {
		fmt.Printf("Ошибка создания config.m4: %v\n", err)
		os.Exit(1)
	}

	// Создаем php_cryphptor.h файл
	phpHeader := `#ifndef PHP_CRYPHPTOR_H
#define PHP_CRYPHPTOR_H

extern zend_module_entry cryphptor_module_entry;
#define phpext_cryphptor_ptr &cryphptor_module_entry

#define PHP_CRYPHPTOR_VERSION "0.1.0"

#ifdef PHP_WIN32
#	define PHP_CRYPHPTOR_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_CRYPHPTOR_API __attribute__ ((visibility("default")))
#else
#	define PHP_CRYPHPTOR_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#if defined(ZTS) && defined(COMPILE_DL_CRYPHPTOR)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif`

	if err := os.WriteFile(filepath.Join(buildDir, "php_cryphptor.h"), []byte(phpHeader), 0644); err != nil {
		fmt.Printf("Ошибка создания php_cryphptor.h: %v\n", err)
		os.Exit(1)
	}

	// Создаем cryphptor.c файл (основной файл расширения)
	cryphptorC := `#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_cryphptor.h"

// Объявление функции
PHP_FUNCTION(cryphptor_decrypt);

// Таблица аргументов функции
ZEND_BEGIN_ARG_INFO_EX(arginfo_cryphptor_decrypt, 0, 0, 2)
	ZEND_ARG_INFO(0, encrypted_data)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

// Таблица функций расширения
const zend_function_entry cryphptor_functions[] = {
	PHP_FE(cryphptor_decrypt, arginfo_cryphptor_decrypt)
	PHP_FE_END
};

// Информация о модуле
zend_module_entry cryphptor_module_entry = {
	STANDARD_MODULE_HEADER,
	"cryphptor",
	cryphptor_functions,
	NULL, // PHP_MINIT
	NULL, // PHP_MSHUTDOWN
	NULL, // PHP_RINIT
	NULL, // PHP_RSHUTDOWN
	NULL, // PHP_MINFO
	PHP_CRYPHPTOR_VERSION,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_CRYPHPTOR
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(cryphptor)
#endif
`

	if err := os.WriteFile(filepath.Join(buildDir, "cryphptor.c"), []byte(cryphptorC), 0644); err != nil {
		fmt.Printf("Ошибка создания cryphptor.c: %v\n", err)
		os.Exit(1)
	}

	fmt.Printf("PHP расширение подготовлено в директории: %s\n", buildDir)
	fmt.Println("Для сборки расширения выполните следующие команды в директории сборки:")
	fmt.Println("  phpize")
	fmt.Println("  ./configure")
	fmt.Println("  make")
}

// Функция для копирования файлов
func copyFile(src, dst string) error {
	data, err := os.ReadFile(src)
	if err != nil {
		return err
	}
	return os.WriteFile(dst, data, 0644)
}