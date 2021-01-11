FROM ubuntu:20.04
RUN apt-get update \
  && apt-get -y install xinetd \
  && apt-get clean \
  && sed -i -E 's/(disable\s*=\s*)yes/\1no/' /etc/xinetd.d/daytime /etc/xinetd.d/echo
EXPOSE 7
EXPOSE 13
ENTRYPOINT ["xinetd", "-dontfork"]
