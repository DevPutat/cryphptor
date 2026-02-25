#!/bin/bash
# Docker entrypoint script for Cryphptor XOR Extension
# 
# Usage:
#   docker run -e XOR_DECRYPT_KEY="your_secret_key" -v /path/to/laravel:/var/www cryphptor-xor-8.4
#
# Environment variables:
#   XOR_DECRYPT_KEY  - The XOR decryption key (required for running encrypted apps)
#   APP_DIR          - The application directory (default: /var/www)

set -e

APP_DIR="${APP_DIR:-/var/www}"

# Check if XOR_DECRYPT_KEY is provided
if [ -n "$XOR_DECRYPT_KEY" ]; then
    echo "Configuring XOR decryption with provided key..."
    
    # Export the key for PHP processes
    export XOR_DECRYPT_KEY
    
    echo "✓ XOR decryption configured"
    echo "  Key length: ${#XOR_DECRYPT_KEY} characters"
    echo "  All .php files will be decrypted on-the-fly"
else
    echo "⚠ Warning: XOR_DECRYPT_KEY environment variable is not set."
    echo "  Encrypted PHP files will not be decrypted properly."
    echo "  Set XOR_DECRYPT_KEY to run encrypted applications."
    echo ""
fi

# Change to app directory
cd "$APP_DIR"

# Execute the command passed to docker
exec "$@"
