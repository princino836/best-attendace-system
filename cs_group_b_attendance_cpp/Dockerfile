FROM ubuntu:24.04
RUN apt-get update && apt-get install -y g++ cmake git libasio-dev libboost-all-dev && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY . .
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j2
RUN mkdir -p public
COPY index.html style.css script.js public/
EXPOSE 8080
CMD ["./build/server"]
