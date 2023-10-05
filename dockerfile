# Use the Debian base image
FROM debian:11

# Set the working directory inside the container
WORKDIR /app

# Copy the scripts folder to the container
COPY . /app/
COPY . /app/

# Install dos2unix and convert scripts line endings
RUN apt update && \
    apt install -y dos2unix && \
    dos2unix /app/scripts/install_deps.sh && \
    dos2unix /app/scripts/install_mediaservices.sh && \
    rm -rf /var/lib/apt/lists/*

# Install dependencies
RUN apt update && \
    apt install -y build-essential autoconf libtool pkg-config cmake apt-transport-https git wget && \
    rm -rf /var/lib/apt/lists/*

# Change scripts permissions and execute them
RUN chmod +x /app/scripts/install_deps.sh && \
    chmod +x /app/scripts/install_mediaservices.sh && \
    ls -la /app && \
    /app/scripts/install_deps.sh && \
    /app/scripts/install_mediaservices.sh

# Set the entry point to run any desired command or start the application
# ENTRYPOINT [ "command-to-run" ]

CMD ["/app/cms_server/cms_server.cc"]
