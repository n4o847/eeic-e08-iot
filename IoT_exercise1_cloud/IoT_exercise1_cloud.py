#!/usr/bin/env python3

import socket
import threading
import re
import sys

server_addr = ('0.0.0.0', 10030)
data_file = './IoT_exercise1_received_data.csv'

def start_server():
  with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
    server.bind(server_addr)
    server.listen(5)
    print(f'[*] listen on {server_addr[0]}:{server_addr[1]}')
    while True:
      client, client_addr = server.accept()
      print(f'[*] connected from {client_addr[0]}:{client_addr[1]}')
      client_handler = threading.Thread(target=handle_client, args=(client,))
      client_handler.setDaemon(True)
      client_handler.start()

pattern = re.compile(r'^(\d+),(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}),(\d+),([01])\r\n$')

def handle_client(client):
  bufsize = 1024
  request = client.recv(bufsize)

  if request:
    print(f'[*] recv: {request}')
    request_str = request.decode()
    m = pattern.match(request_str)
    if m and m.group(0) == request_str:
      with open(data_file, mode='a') as f:
        f.write(request_str)
      response = b'OK\r\n'
      client.send(response)
      print(f'[*] send: {response}')
    elif request_str == 'GET\r\n':
      with open(data_file) as f:
        response = f.read().encode()
      client.send(response)
      client.close()
      print(f'[*] send data')
    else:
      response = b'ERROR\r\n'
      client.send(response)
      print(f'[*] send: {response}')

def main():
  try:
    start_server()
  except KeyboardInterrupt:
    print()
    print('[*] exit')
    sys.exit(0)

if __name__ == '__main__':
  main()
