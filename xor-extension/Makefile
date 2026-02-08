EXT_NAME := xor_decrypt
PHP_VERSION_MAJOR := 8.4
PHP_VERSION_MINOR := 7.4
DIST= "$(PWD)/php."

MINOR_PATH="$(DIST)$(PHP_VERSION_MINOR)"
MAJOR_PATH="$(DIST)$(PHP_VERSION_MINOR)"

EXT_PATH="$(EXT_NAME)/modules/$(EXT_NAME).so"

MINOR_EXT_PATH="$(MINOR_PATH)/$(EXT_PATH)"
MAJOR_EXT_PATH="$(MIJOR_PATH)/$(EXT_PATH)"

.PHONY: generate generate-minor build build-minor test test-minor docker docker-minor clean

generate:
	docker run --rm -v "$(MAJOR_PATH)":/work php:$(PHP_VERSION_MAJOR)-cli bash -c "\
		docker-php-source extract && \
		php /usr/src/php/ext/ext_skel.php --ext $(EXT_NAME) --dir /work && \
		echo '✓ Сгенерировано'"

generate-minor:
	docker run --rm -v "$(MINOR_PATH)":/work php:$(PHP_VERSION_MINOR)-cli bash -c "\
		docker-php-source extract && \
		php /usr/src/php/ext/ext_skel.php --ext $(EXT_NAME) --dir /work && \
		echo '✓ Сгенерировано'"

build:
	docker run --rm -v "$(MAJOR_PATH)/$(EXT_NAME)":/ext php:$(PHP_VERSION_MAJOR)-cli bash -c "\
		docker-php-source extract && \
		cd /ext && \
		phpize && \
		./configure && \
		make && \
		echo '✓ Собрано: /ext/modules/$(EXT_NAME).so'"

build-minor:
	docker run --rm -v "$(MINOR_PATH)/$(EXT_NAME)":/ext php:$(PHP_VERSION_MINOR)-cli bash -c "\
		docker-php-source extract && \
		cd /ext && \
		phpize && \
		./configure && \
		make && \
		echo '✓ Собрано: /ext/modules/$(EXT_NAME).so'"

test:
	docker run --rm -v "$(MAJOR_PATH)":/ext php:$(PHP_VERSION_MAJOR)-cli php \
		-d extension=/ext/modules/$(EXT_NAME).so \
		-r "echo 'Тест: ' . function_exists('confirm_$(EXT_NAME)_compiled') ? 'OK' : 'FAIL';"

test-minor:
	docker run --rm -v "$(MINOR_PATH)":/ext php:$(PHP_VERSION_MINOR)-cli php \
		-d extension=/ext/modules/$(EXT_NAME).so \
		-r "echo 'Тест: ' . function_exists('confirm_$(EXT_NAME)_compiled') ? 'OK' : 'FAIL';"

docker:
	docker build -t cryphptor-xor-8.4:latest -f ./Dockerfile.php8.4 .

docker-minor:
	docker build -t cryphptor-xor-7.4:latest -f ./Dockerfile.php7.4 .

clean:
	rm -rf php.*
