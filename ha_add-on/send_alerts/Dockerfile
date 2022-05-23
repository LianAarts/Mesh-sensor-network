FROM python:3

ADD listenToMesh.py /
RUN chmod a+x /send_alerts.py

RUN pip freeze > requirements.txt && \
    pip install requests

CMD [ "python3", "./send_alerts.py" ]