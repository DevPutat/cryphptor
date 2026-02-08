#!/bin/bash

EXT_NAME="xor_decrypt"

docker run --rm -v "$(pwd)":/work php:7.4-cli bash -c "
docker-php-source extract
php /usr/src/php/ext/ext_skel.php --ext ${EXT_NAME} --dir /work
echo '✓ Расширение сгенерировано'
"

echo "Файлы расширения находятся в: $(pwd)/${EXT_NAME}"
ls -la "${EXT_NAME}/"
