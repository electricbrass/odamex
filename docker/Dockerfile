# build stage for compiling odamex
FROM alpine:3.23 AS build

WORKDIR /build
ENV DEBIAN_FRONTEND=noninteractive

# Base build deps (Alpine equivalents)
RUN apk add --no-cache \
    bash \
    git \
    gcc \
    g++ \
    musl-dev \
    cmake \
    make \
    patch \
    zlib-dev \
    zstd-dev \
    ca-certificates \
    linux-headers \
    tzdata

# Use bash (needed for arrays / shopt / etc.)
SHELL ["/bin/bash", "-c"]

ARG REPO_URL
ARG REPO_TAG

# Clone repository
RUN true \
    && test -n "$REPO_URL" && test -n "$REPO_TAG" \
    && git clone --depth 1 --branch "$REPO_TAG" "$REPO_URL" odamex

WORKDIR /build/odamex

# Apply patches
RUN mkdir /patches
COPY docker-files/patches /patches

RUN true \
    && shopt -s nullglob \
    && for p in /patches/*.patch; do patch -p1 < "$p"; done

# Submodules
RUN git submodule init \
    && git submodule update -r

# Build setup
RUN mkdir build

WORKDIR /build/odamex/build

RUN true \
    && cmake -W no-dev \
        -D CMAKE_BUILD_TYPE=Release \
        -D CMAKE_CXX_COMPILER=g++ \
        -D CMAKE_C_COMPILER=gcc \
        -D BUILD_CLIENT=0 \
        -D BUILD_LAUNCHER=0 \
        -D BUILD_TESTS=0 \
        -D CMAKE_C_FLAGS="-w" \
        -D CMAKE_CXX_FLAGS="-w" \
        .. \
    && cmake --build . --parallel 4 --target odasrv

# Install
ENV INSTALL_DIR=/usr/local/games/odamex
COPY docker-files/odamex-server.sh /usr/local/bin/odamex-server

RUN true \
    && COPY_PATTERNS=(\
        server/odamex.wad \
        server/odasrv \
    ) \
    && mkdir -p "$INSTALL_DIR" \
    && cp "${COPY_PATTERNS[@]}" "$INSTALL_DIR/" \
    && chmod a+x /usr/local/bin/odamex-server \
    && sed -i "s|INSTALL_DIR|${INSTALL_DIR}|" /usr/local/bin/odamex-server


# runtime stage
FROM alpine:3.23

# runtime deps
RUN apk add --no-cache \
    bash \
    tini \
    gosu \
    ca-certificates

COPY --from=build /usr/local /usr/local/

ENV ODAMEX_UID= \
    ODAMEX_GID=

COPY ./docker-files/entrypoint.sh /
RUN chmod +x /entrypoint.sh

ENTRYPOINT [ "tini", "--", "/entrypoint.sh" ]