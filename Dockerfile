#
# Copyright 2021-2025 Software Radio Systems Limited
#
# This file is part of srsRAN
#
# srsRAN is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# srsRAN is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# A copy of the GNU Affero General Public License can be found in
# the LICENSE file in the top-level directory of this distribution
# and at http://www.gnu.org/licenses/.
#

# Build args
ARG OS_VERSION=24.04
ARG UHD_VERSION=4.7.0.0
ARG DPDK_VERSION=23.11.1
ARG MARCH=native
ARG NUM_JOBS=""

##################
# Stage 1: Build #
##################
FROM ubuntu:$OS_VERSION AS builder

# Adding the complete repo to the context, in /src folder
ADD . /src

# Install dependencies in a single RUN command to optimize layer caching
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get upgrade -y && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    libzmq3-dev ccache git clang && \
    rm -rf /var/lib/apt/lists/*

# Install srsRAN build dependencies
RUN /src/docker/scripts/install_dependencies.sh build && \
    /src/docker/scripts/install_uhd_dependencies.sh build && \
    /src/docker/scripts/install_dpdk_dependencies.sh build

ARG UHD_VERSION
ARG DPDK_VERSION
ARG MARCH
ARG NUM_JOBS

# Compile UHD/DPDK
RUN /src/docker/scripts/build_uhd.sh "${UHD_VERSION}" ${MARCH} ${NUM_JOBS} && \
    /src/docker/scripts/build_dpdk.sh "${DPDK_VERSION}" ${MARCH} ${NUM_JOBS}

# Compile srsRAN Project
ARG COMPILER=gcc
ARG EXTRA_CMAKE_ARGS="-DENABLE_EXPORT=ON -DENABLE_ZEROMQ=ON"
ENV UHD_DIR=/opt/uhd/${UHD_VERSION}
ENV DPDK_DIR=/opt/dpdk/${DPDK_VERSION}

RUN if [ -z "$NUM_JOBS" ]; then NUM_JOBS=$(nproc); fi \
    && /src/docker/scripts/builder.sh -c ${COMPILER} \
    -m "-j${NUM_JOBS} srscu srsdu srsdu_split_8 srsdu_split_7_2 gnb gnb_split_8 gnb_split_7_2 ru_emulator" \
    -DBUILD_TESTS=False -DENABLE_UHD=On -DENABLE_DPDK=On -DMARCH=${MARCH} -DCMAKE_INSTALL_PREFIX=/opt/srs \
    ${EXTRA_CMAKE_ARGS} /src \
    && mkdir -p /opt/srs/bin /opt/srs/share/srsran \
    && cp /src/build/apps/cu/srscu                 /opt/srs/bin/srscu            \
    && cp /src/build/apps/du/srsdu                 /opt/srs/bin/srsdu            \
    && cp /src/build/apps/du_split_8/srsdu         /opt/srs/bin/srsdu_split_8    \
    && cp /src/build/apps/du_split_7_2/srsdu       /opt/srs/bin/srsdu_split_7_2  \
    && cp /src/build/apps/gnb/gnb                  /opt/srs/bin/gnb              \
    && cp /src/build/apps/gnb_split_8/gnb          /opt/srs/bin/gnb_split_8      \
    && cp /src/build/apps/gnb_split_7_2/gnb        /opt/srs/bin/gnb_split_7_2    \
    && cp /src/build/apps/examples/ofh/ru_emulator /opt/srs/bin/ru_emulator      \
    && cp /src/configs/*.yml                       /opt/srs/share/srsran/

###############################
# Stage 2: turbostat builder #
###############################
FROM debian:bookworm-slim AS turbostat-builder

RUN apt-get update && apt-get install -y \
    git build-essential libcap-dev \
    && rm -rf /var/lib/apt/lists/*

RUN git clone --depth 1 https://github.com/torvalds/linux.git /tmp/linux
WORKDIR /tmp/linux/tools/power/x86/turbostat
RUN make

################
# Stage 3: Run #
################

FROM ubuntu:$OS_VERSION

ARG UHD_VERSION
ARG DPDK_VERSION

# Copy srsRAN binaries and libraries from the builder stage
COPY --from=builder /opt/uhd/${UHD_VERSION}   /opt/uhd/${UHD_VERSION}
COPY --from=builder /opt/dpdk/${DPDK_VERSION} /opt/dpdk/${DPDK_VERSION}
COPY --from=builder /opt/srs                  /usr/local

# Copy turbostat binary
COPY --from=turbostat-builder /tmp/linux/tools/power/x86/turbostat/turbostat /usr/local/bin/turbostat
RUN chmod +x /usr/local/bin/turbostat

# Copy the install dependency scripts
ADD docker/scripts/install_uhd_dependencies.sh  /usr/local/etc/install_uhd_dependencies.sh
ADD docker/scripts/install_dpdk_dependencies.sh /usr/local/etc/install_dpdk_dependencies.sh
ADD docker/scripts/install_dependencies.sh      /usr/local/etc/install_srsran_dependencies.sh

# Set environment variables efficiently
ENV LD_LIBRARY_PATH="/opt/uhd/${UHD_VERSION}/lib:/opt/uhd/${UHD_VERSION}/lib/x86_64-linux-gnu:/opt/uhd/${UHD_VERSION}/lib/aarch64-linux-gnu:\
/opt/dpdk/${DPDK_VERSION}/lib:/opt/dpdk/${DPDK_VERSION}/lib/x86_64-linux-gnu:/opt/dpdk/${DPDK_VERSION}/lib/aarch64-linux-gnu"
ENV PATH="$PATH:/opt/uhd/${UHD_VERSION}/bin:/opt/dpdk/${DPDK_VERSION}/bin"

# Install runtime dependencies
RUN /usr/local/etc/install_srsran_dependencies.sh run && \
    /usr/local/etc/install_uhd_dependencies.sh run && \
    /usr/local/etc/install_dpdk_dependencies.sh run && \
    DEBIAN_FRONTEND=noninteractive apt-get update -y && apt-get install -y \
    curl ca-certificates libzmq3-dev iputils-ping iproute2 iperf3 libcap2 libpci3 && \
    apt-get autoremove -y && apt-get clean && rm -rf /var/lib/apt/lists/*
