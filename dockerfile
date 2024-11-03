FROM ubuntu:20.04

WORKDIR /app

RUN apt-get update && apt-get install -y valgrind lldb make g++ && apt-get clean && rm -rf /var/lib/apt/lists/*

COPY . /app

RUN make
# ENTRYPOINT ["make", "re"]