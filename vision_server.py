"simple server sending detected triangles on connection"

import SocketServer
import json

from vision import get_triangles, YELLOW, RED

class VisionServer(SocketServer.ThreadingTCPServer):
    "server class"
    allow_reuse_address = True

class VisionHandler(SocketServer.BaseRequestHandler):
    "request handler sending data for triangle"
    def handle(self):
        try:
            triangle = next(get_triangles())

            if triangle is None:
                response = []
            else:
                xpos = int(triangle.xpos * 1000)
                ypos = int(triangle.ypos * 1000)
                color = 'x'

                if triangle.color == YELLOW:
                    color = 'y'
                elif triangle.color == RED:
                    color = 'r'
                else:
                    color = 'x'

                response = [xpos, ypos, color]


            self.request.sendall(json.dumps(response).strip())
        except Exception:
            self.request.sendall(json.dumps([]).strip())

SERVER = VisionServer(('0.0.0.0', 6666), VisionHandler)
SERVER.serve_forever()
