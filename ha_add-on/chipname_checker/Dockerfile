FROM python:3

ADD listenToMesh.py /
RUN chmod a+x /listenToMesh.py

RUN pip freeze > requirements.txt && \
    pip install requests

CMD [ "python3", "./listenToMesh.py" ]