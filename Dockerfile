FROM alpine:latest

RUN apk add clang clang-analyzer clang-libs clang-extra-tools cmake git samurai g++ llvm14 llvm14-libs llvm14-test-utils compiler-rt
RUN adduser -D build

USER build
WORKDIR /home/build/ccl

ADD --chown=build . .

ENTRYPOINT ["cmake/docker-entrypoint.sh"]
