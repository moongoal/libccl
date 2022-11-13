FROM alpine:latest

RUN apk add clang clang-analyzer clang-libs clang-extra-tools cmake git samurai g++ llvm13 llvm13-libs llvm13-test-utils compiler-rt
RUN adduser -D build

USER build
WORKDIR /home/build/ccl

ADD --chown=build . .

ENTRYPOINT ["cmake/docker-entrypoint.sh"]
