#!/bin/bash

#docker build -t ubuntu-tgbot .

docker run -it --rm -v $(pwd):/app ubuntu-tgbot    bash -c "cd /app/build && cmake .. && make -j $(nproc)"
