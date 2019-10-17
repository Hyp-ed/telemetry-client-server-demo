import socket
import selectors

HOST = socket.gethostname()
PORT = 9090
HEADER_LENGTH = 8

list_of_clients = []

def accept(sock):
    client_socket, address = sock.accept()
    print(f'Connection from {address}')

    # add client to our list of client sockets
    list_of_clients.append(client_socket)

    # tell socket manager what to do when we receive new info on our client socket
    socket_manager.register(client_socket, selectors.EVENT_READ, handle_client_socket_data)

# reading our protocol and extracting the payload
def receive_message(client_socket):
    try:
        # receive header data specifying length of payload
        message_header = client_socket.recv(HEADER_LENGTH)

        # received no data (received 0 bytes), client probs closed connection
        if not len(message_header):
            raise

        # convert header data into length of payload
        payload_length = int(message_header.decode('utf-8').strip())

        # return an object of message header and payload
        return {'header': message_header, 'payload': client_socket.recv(payload_length)}

    # client closed connection probs
    except:
        return False

def handle_client_socket_data(client_socket):
    # get message header and payload
    message = receive_message(client_socket)

    if not message:
        print(f'Client socket at {client_socket.getpeername()} closed connection')
        socket_manager.unregister(client_socket)
        list_of_clients.remove(client_socket)
        client_socket.close()
        return

    print(f'Received message from {client_socket.getpeername()}: {message["payload"].decode("utf-8")}')

    # broadcast message to other clients
    for client in list_of_clients:

        # don't send to sender
        if client != client_socket:
            client.send(message['header'] + message['payload'])

if __name__ == "__main__":
    # create socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # reuse addresses for server socket
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    # tell OS we want to bind our server socket
    server_socket.bind((HOST, PORT))

    # tell server socket to start listening
    server_socket.listen()

    print(f'Server now listening for connections on {HOST}:{PORT}')

    socket_manager = selectors.DefaultSelector()
    socket_manager.register(server_socket, selectors.EVENT_READ, accept)

    while True:
        # socket manager knows theres stuff to be dealt with
        events = socket_manager.select()

        for unread_socket, _ in events:
            callback = unread_socket.data
            callback(unread_socket.fileobj)
