FROM ubuntu:18.04

# Avoid warnings by switching to noninteractive
ENV DEBIAN_FRONTEND=noninteractive
ENV NVIDIA_VISIBLE_DEVICES \
    ${NVIDIA_VISIBLE_DEVICES:-all}
ENV NVIDIA_DRIVER_CAPABILITIES \
    ${NVIDIA_DRIVER_CAPABILITIES:+$NVIDIA_DRIVER_CAPABILITIES,}graphics

ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN apt-get update \
    && apt-get -y upgrade \
    && apt-get -y install --no-install-recommends apt-utils dialog 2>&1 \
    && apt-get install -y libgl1-mesa-glx \
    && apt-get install -y libgl1-mesa-dri \
    && apt-get -y install clang \
    && apt-get -y install doxygen \
    && apt-get -y install graphviz \
    && apt-get -y install git \
    && apt-get -y install make astyle \
    && apt-get -y install python3-pyqt5 \
    && apt-get -y install avrdude  avr-libc binutils-avr gcc-avr \
    && apt-get -y install python3 \
    && apt-get -y install python3-pip \
    && pip3 install --upgrade pip \
    && pip3 install crcmod \
    && pip3 install matplotlib \
    && pip3 install pyserial \
    && pip3 install PyQt5-stubs \
    && pip3 install pylint \
    && groupadd --gid $USER_GID $USERNAME \
    && useradd -s /bin/bash --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && apt-get install -y sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME\
    && chmod 0440 /etc/sudoers.d/$USERNAME \
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/*

RUN usermod -a -G dialout $USERNAME

# Switch back to dialog for any ad-hoc use of apt-get
ENV DEBIAN_FRONTEND=dialog