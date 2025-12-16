FROM ubuntu:24.04 AS build

RUN apt update && \
    apt install -yq build-essential pkg-config \
    libboost-dev libboost-filesystem-dev libboost-regex-dev \
    libboost-system-dev libboost-thread-dev libboost-iostreams-dev \
    libgmp3-dev libxml2-dev \
    sqlite3 libsqlite3-dev \
    libtcmalloc-minimal4 \
    liblua5.1-dev \
    libmysqlclient-dev \
    ccache \
    ca-certificates

COPY sources /usr/src/otxserver/sources/
WORKDIR /usr/src/otxserver/sources/

RUN make -j$(nproc)


FROM ubuntu:24.04

RUN apt update && \
    apt install -yq build-essential pkg-config \
    libboost-dev libboost-filesystem-dev libboost-regex-dev \
    libboost-system-dev libboost-thread-dev libboost-iostreams-dev \
    libgmp3-dev libxml2-dev \
    sqlite3 libsqlite3-dev \
    libtcmalloc-minimal4 \
    liblua5.1-dev \
    libmysqlclient-dev \
    ccache \
    ca-certificates

COPY --from=build /usr/src/otxserver/sources/theotxserver /bin/theotxserver

EXPOSE 7171 7172
WORKDIR /srv
VOLUME /srv
ENTRYPOINT ["/bin/theotxserver"]
