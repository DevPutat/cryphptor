EXT_NAME := xor_decrypt
PHP_VERSION_MAJOR := 8.4
PHP_VERSION_MINOR := 7.4
DIST= "$(PWD)/php."

.PHONY: generate generate-minor build build-minor test test-minor clean

generate:
	docker run --rm -v "$(DIST)$(PHP_VERSION_MAJOR)":/work php:$(PHP_VERSION_MAJOR)-cli bash -c "\
		docker-php-source extract && \
		php /usr/src/php/ext/ext_skel.php --ext $(EXT_NAME) --dir /work && \
		echo '✓ Сгенерировано'"

generate-minor:
	docker run --rm -v "$(DIST)$(PHP_VERSION_MINOR)":/work php:$(PHP_VERSION_MINOR)-cli bash -c "\
		docker-php-source extract && \
		php /usr/src/php/ext/ext_skel.php --ext $(EXT_NAME) --dir /work && \
		echo '✓ Сгенерировано'"

build:
	docker run --rm -v "$(DIST)$(PHP_VERSION_MAJOR)/$(EXT_NAME)":/ext php:$(PHP_VERSION_MAJOR)-cli bash -c "\
		docker-php-source extract && \
		cd /ext && \
		phpize && \
		./configure && \
		make && \
		echo '✓ Собрано: /ext/modules/$(EXT_NAME).so'"

build-minor:
	docker run --rm -v "$(DIST)$(PHP_VERSION_MINOR)/$(EXT_NAME)":/ext php:$(PHP_VERSION_MINOR)-cli bash -c "\
		docker-php-source extract && \
		cd /ext && \
		phpize && \
		./configure && \
		make && \
		echo '✓ Собрано: /ext/modules/$(EXT_NAME).so'"

test:
	docker run --rm -v "$(DIST)$(PHP_VERSION_MAJOR)":/ext php:$(PHP_VERSION_MAJOR)-cli php \
		-d extension=/ext/modules/$(EXT_NAME).so \
		-r "echo 'Тест: ' . function_exists('confirm_$(EXT_NAME)_compiled') ? 'OK' : 'FAIL';"

test-minor:
	docker run --rm -v "$(DIST)$(PHP_VERSION_MINOR)":/ext php:$(PHP_VERSION_MINOR)-cli php \
		-d extension=/ext/modules/$(EXT_NAME).so \
		-r "echo 'Тест: ' . function_exists('confirm_$(EXT_NAME)_compiled') ? 'OK' : 'FAIL';"

clean:
	rm -rf php.*
