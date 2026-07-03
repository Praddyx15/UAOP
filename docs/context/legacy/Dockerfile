FROM osrf/ros:humble-desktop

# Install prerequisites
RUN apt-get update && apt-get install -y \
    python3-pip \
    python3-colcon-common-extensions \
    git \
    wget \
    curl \
    tmux \
    iputils-ping \
    nano \
    && rm -rf /var/lib/apt/lists/*

# Install specific PX4 & Ardupilot SITL dependencies
RUN pip3 install --upgrade pip
RUN pip3 install \
    MAVProxy \
    numpy \
    toml \
    pyserial \
    empy==3.3.4 \
    jinja2 \
    rosdep

# Setup ROS bridge
RUN apt-get update && apt-get install -y \
    ros-humble-rosbridge-suite \
    ros-humble-micro-ros-agent \
    && rm -rf /var/lib/apt/lists/*

# Set up working directory
WORKDIR /uaop_workspace

# We will mount ros2_ws and other configurations here via docker-compose
CMD ["/bin/bash", "-c", "source /opt/ros/humble/setup.bash && exec bash"]
