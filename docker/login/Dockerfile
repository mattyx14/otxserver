FROM python:latest

WORKDIR /app

COPY login/requirements.txt /app

RUN pip install -r requirements.txt

COPY login/login.py /app

CMD ["python", "login.py"]
