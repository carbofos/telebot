FROM ubuntu:20.04
# FROM debian:bullseye
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    g++ \
    cmake \
    git \
    curl \
    wget \
    libcurl4-openssl-dev \
    libssl-dev \
    zlib1g-dev \
    libboost-all-dev \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt
RUN git clone https://github.com/reo7sp/tgbot-cpp.git
WORKDIR /opt/tgbot-cpp
RUN mkdir build && cd build && \
    cmake .. && \
    make -j $(nproc) && \
    make install

WORKDIR /app/build

#CMD ["/bin/bash"]
CMD ["cmake", "..", "&&", "make"]
