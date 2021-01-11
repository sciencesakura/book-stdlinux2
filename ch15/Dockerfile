FROM ubuntu:20.04
RUN apt-get update \
  && apt-get -y install xinetd \
  && apt-get clean
COPY xinetd.d/* /etc/xinetd.d/
EXPOSE 13
ENTRYPOINT ["xinetd", "-dontfork"]
