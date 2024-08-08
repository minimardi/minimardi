FROM ubuntu:bionic as builder
RUN apt-get update && \
	apt-get install -y build-essential git cmake autoconf libtool pkg-config
RUN git clone https://github.com/minimardi/minimardi /minimardi_docker
WORKDIR /minimardi_docker/src
RUN make

FROM builder as botBuilder
RUN apt-get update &&\
    apt-get install -y python3-venv
RUN git clone https://github.com/careless25/lichess-bot.git /lcbot
WORKDIR /lcbot
RUN python3 -m venv .venv &&\
    . .venv/bin/activate &&\
    pip3 install wheel &&\
    pip3 install -r requirements.txt

FROM ubuntu:bionic as lcbot
RUN apt-get update &&\
    apt-get install -y python3 &&\
    apt-get install -y vim &&\
    apt-get clean all
COPY --from=builder /minimardi_docker/src/minimardi /lcbot/engines/minimardi
COPY --from=builder /minimardi_docker/src/Book.txt /lcbot/engines/Book.txt
COPY --from=builder /minimardi_docker/src/Book.txt /lcbot/Book.txt
COPY --from=botBuilder /lcbot /lcbot
WORKDIR /lcbot
