#!/bin/bash

DIR="$(pwd)"
NGINX_VERSION="1.27.4"

echo "Nginx version $NGINX_VERSION"
echo "Directory: $DIR"

mkdir -p $DIR/buildnginx/modules/ngx_module_vts/
wget -q "https://nginx.org/download/nginx-${NGINX_VERSION}.tar.gz"
tar -xzf nginx-${NGINX_VERSION}.tar.gz
mv nginx-${NGINX_VERSION}/* $DIR/buildnginx/
mv src  $DIR/buildnginx/modules/ngx_module_sts/
mv config  $DIR/buildnginx/modules/ngx_module_sts/
cd $DIR/buildnginx
./configure --with-cc-opt='-g -O2 -fstack-protector-strong -Wformat -Werror=format-security' --with-pcre --with-stream --with-http_ssl_module --with-http_stub_status_module --add-module=./modules/ngx_module_sts/
make -j12
