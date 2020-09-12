#!/bin/sh

NET_ROOT=$(pwd)
NET_BUILD_DIR=${NET_ROOT}/build

rm -rf ${NET_BUILD_DIR}
mkdir -p ${NET_BUILD_DIR}
mkdir -p ${NET_BUILD_DIR}/third_party
mkdir -p ${NET_BUILD_DIR}/release

#编译mbedtls
tar -xf ${NET_ROOT}/third_party/v2.24.0.tar.gz -C ${NET_BUILD_DIR}/third_party
cd ${NET_BUILD_DIR}/third_party/mbedtls-2.24.0
cmake .
make lib
MBEDTLS_RELEASE_DIR=${NET_BUILD_DIR}/release/mbedtls
mkdir -p ${MBEDTLS_RELEASE_DIR}
mkdir -p ${MBEDTLS_RELEASE_DIR}/lib
mkdir -p ${MBEDTLS_RELEASE_DIR}/include
cp library/*.a ${MBEDTLS_RELEASE_DIR}/lib
cp -r include/mbedtls ${MBEDTLS_RELEASE_DIR}/include