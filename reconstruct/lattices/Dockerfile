from sagemath/sagemath

USER root
RUN apt update && apt install -y time
USER sage

RUN mkdir -p /home/sage/b64dec
COPY --chown=1000:1000 . /home/sage/b64dec
WORKDIR /home/sage/b64dec

ENTRYPOINT []
CMD /usr/bin/time -v sage rsa.sage --run config.json
