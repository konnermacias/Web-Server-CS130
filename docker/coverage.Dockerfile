### Build/run coverage report ###
FROM server_gitrdone:base as builder

COPY . /usr/src/project
WORKDIR /usr/src/project/build_coverage

RUN cmake -DCMAKE_BUILD_TYPE=Coverage ..
RUN mkdir static
RUN mkdir static_files1
RUN mkdir memes_r_us

RUN cp -r ../memes_r_us ./
RUN cp -r ../static ./

RUN make coverage
