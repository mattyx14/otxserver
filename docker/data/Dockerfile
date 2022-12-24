FROM webdevops/php-nginx:8.2-alpine

COPY ./ /tmp/otserver/
COPY ./docker/data/start.sh /opt/docker/provision/entrypoint.d/99-start.sh
