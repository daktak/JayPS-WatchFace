FROM --platform=linux/amd64 python:3.11-slim

RUN apt-get update && apt-get install -y \
    curl \
    libfreetype6 \
    libpng16-16 \
    npm \
    && rm -rf /var/lib/apt/lists/*

RUN pip install pebble-tool

RUN pebble sdk install latest

WORKDIR /app

CMD ["pebble", "build"]
