# Stage 1: Download all dependencies
FROM ubuntu:20.04 AS dependencies

RUN apt-get update && apt-get install -y --no-install-recommends cmake

RUN apt-get update && apt-get install -y --no-install-recommends git

RUN apt-get install -y --no-install-recommends \
	build-essential ca-certificates curl zip unzip tar pkg-config

RUN apt-get install -y --no-install-recommends libluajit-5.1-dev unzip

WORKDIR /opt
RUN git clone https://github.com/microsoft/vcpkg
RUN ./vcpkg/bootstrap-vcpkg.sh

WORKDIR /opt/vcpkg
COPY vcpkg.json /opt/vcpkg/
RUN /opt/vcpkg/vcpkg --feature-flags=binarycaching,manifests,versions install

# Stage 2: create build
FROM dependencies AS build

COPY cmake /srv/cmake
COPY CMakeLists.txt vcpkg.json /srv/
COPY src /srv/src

WORKDIR /srv/build

RUN cmake -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake .. && make -j`nproc`

# Stage 3: load data and execute
FROM build

COPY --from=build /srv/build/bin/otxsr /bin/otxsr
COPY LICENSE *.sql key.pem /otxsr/
COPY data /otxsr/data
COPY config.lua.dist /otxsr/config.lua

WORKDIR /otxsr

RUN sed -i '/mysqlHost = .*$/c\mysqlHost = "mysql"' config.lua \
  && sed -i '/mysqlUser = .*$/c\mysqlUser = "otserver"' config.lua \
  && sed -i '/mysqlPass = .*$/c\mysqlPass = "otserver"' config.lua \
  && sed -i '/mysqlDatabase = .*$/c\mysqlDatabase = "otserver"' config.lua \
  && unzip -o data/world/world.zip -d data/world/

VOLUME /otxsr

ENTRYPOINT ["/bin/otxsr"]

