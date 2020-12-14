#!/usr/bin/env python3

from http.server import HTTPServer
from http.server import BaseHTTPRequestHandler
import socket

server_addr = ('127.0.0.1', 8080)
target_addr = ('iot.hongo.wide.ad.jp', 10030)

class MyHandler(BaseHTTPRequestHandler):
  def do_GET(self):
    if self.path == '/data.csv':
      self.send_response(200)
      self.end_headers()

      with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client:
        client.connect(target_addr)

        client.send(b'GET\r\n')

        response = b''
        while chunk := client.recv(4096):
          response += chunk

      print(f'[*] receive data ({len(response)} bytes)')

      self.wfile.write(response)

    elif self.path in ['/', '/index.html']:
      self.send_response(200)
      self.end_headers()

      with open('index.html') as f:
        self.wfile.write(f.read().encode())

    else:
      self.send_response(404)
      self.end_headers()

def main():
  try:
    with HTTPServer(server_addr, MyHandler) as server:
      print(f'[*] listen on {server_addr[0]}:{server_addr[1]}')
      server.serve_forever()
  except KeyboardInterrupt:
    print()
    print('[*] exit')

if __name__ == '__main__':
  main()
