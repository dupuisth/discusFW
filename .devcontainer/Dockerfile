ARG DOCKER_TAG=v5.4.1
FROM espressif/idf:${DOCKER_TAG}

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8

RUN apt-get update -y && apt-get install udev dfu-util libssl-dev python3-dev pkg-config libglib2.0-dev libevent-dev clang-format -y

RUN echo "source /opt/esp/idf/export.sh > /dev/null 2>&1" >> ~/.bashrc

WORKDIR /opt

# ESP-MATTER setup
# RUN git clone --depth 1 --branch release/v1.5 https://github.com/espressif/esp-matter.git /opt/esp-matter
# WORKDIR /opt/esp-matter
# RUN git submodule update --init --depth 1
# WORKDIR /opt/esp-matter/connectedhomeip/connectedhomeip
# RUN ./scripts/checkout_submodules.py --platform esp32 linux --shallow
# RUN echo "source /opt/esp-matter/export.sh > /dev/null 2>&1" >> ~/.bashrc
# RUN echo "export IDF_CCACHE_ENABLE=1" >> ~/.bashrc

ENTRYPOINT [ "/opt/esp/entrypoint.sh" ]

CMD ["/bin/bash", "-c"]