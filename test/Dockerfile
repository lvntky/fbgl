FROM ubuntu:22.04

# Install required packages
RUN apt-get update && apt-get install -y \
    build-essential \
    xvfb \
    imagemagick \
    && rm -rf /var/lib/apt/lists/*

# Create app directory
WORKDIR /app

# Copy your library and any dependencies
COPY . /app

# Make sure binaries are executable
RUN chmod +x /app/*

# Accept program name as argument and run with virtual framebuffer
ENTRYPOINT ["xvfb-run", "-a"]