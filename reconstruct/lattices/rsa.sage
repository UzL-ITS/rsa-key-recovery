import json
import sys
import socket
import time
import random
from concurrent.futures.process import ProcessPoolExecutor


receive_buffer = str()
pbars_global = []
processed_candidate_counter = 0
stop = False
no_data_ctr = 0
no_data_ctr_max = 60
total_process_time = 0


def accept_wrapper(sock):
    conn, addr = sock.accept()  # Should be ready to read
    conn.setblocking(True)
    return conn, addr


def reconstruct_done_method(future):
    global processed_candidate_counter
    global total_process_time

    result, pbar, length, time, processed_candidates = future.result()

    total_process_time += time

    if len(result) > 0:
        print("p = " + str(result[0] * 2 ^ length + pbar))
    else:
        processed_candidate_counter += processed_candidates
        print(str(processed_candidate_counter)
              + " Candidates have been processed. Total process time: " + str(total_process_time))


def service_connection(sock, n, p_len, length, c, executor):
    global receive_buffer
    global pbars_global
    global no_data_ctr
    global no_data_ctr_max
    global stop

    recv_data = sock.recv(4096)  # Should be ready to read
    if recv_data:
        receive_buffer += recv_data.decode('utf-8')
        pbars = receive_buffer.split(';')

        if pbars[-1] == '':
            pbars = pbars[:-1]

        if receive_buffer[-1] == ';':
            receive_buffer = str()
        else:
            receive_buffer = pbars[-1]
            pbars = pbars[:-1]

        pbars_global.extend(pbars)

        if len(pbars_global) > 1000:
            pbars_thread = []
            for pbar in pbars_global:
                pbars_thread.append(int(pbar, 10))
            pbars_global.clear()
            r = executor.submit(reconstruct, n, p_len, pbars_thread, length, c)
            r.add_done_callback(reconstruct_done_method)
    else:
        if no_data_ctr >= no_data_ctr_max:
            stop = True
        no_data_ctr += 1
        time.sleep(1)


def start_server(host, port, n, p_len, length, number_of_workers):
    lsock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    lsock.bind((host, port))
    lsock.listen()
    print("listening on", (host, port))
    lsock.setblocking(True)

    executor = ProcessPoolExecutor(max_workers=number_of_workers)

    c = inverse_mod(2 ^ length, n)

    conn, addr = accept_wrapper(lsock)
    try:
        while not stop:
            service_connection(conn, n, p_len, length, c, executor)
    except KeyboardInterrupt:
        print("caught keyboard interrupt, exiting")
    finally:
        print("Closing socket")
        lsock.close()

    executor.shutdown(wait=True)


def send_msg(host, port, p):
    HOST = host
    PORT = port

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        s.sendall(p.to_bytes(len(bin(p)[2:]), 'big'))


def reconstruct(n, p_len, pbars, length, c=None):
    start = time.process_time()

    result_return = []
    pbar_return = 0

    if c == None:
        c = inverse_mod(2 ^ length, n)
    F.<x> = PolynomialRing(Zmod(n), implementation='NTL')

    for pbar in pbars:

        f = x + pbar * c
        result = f.small_roots(X=2 ^ (p_len - length) - 1, beta=0.4)

        if len(result) > 0:
            result_return = result
            pbar_return = pbar

    end = time.process_time()
    processed_candidates = len(pbars)

    return result_return, pbar_return, length, (end - start), processed_candidates


def load_parameters(file):
    with open(file, 'r') as f:
        config = json.load(f)

    return config


def test_reconstruct(length):
    q = random_prime(2 ^ (length + 1) - 1, lbound=2 ^ length)
    p = random_prime(2 ^ (length + 1) - 1, lbound=2 ^ length)
    n = p * q
    p_len = len(bin(p)[2:])
    for i in range(round(p_len / 2), p_len):
        r = reconstruct(n, p_len, p % (2 ^ i), i)
        if len(r) > 0:
            return i


def test_server(n, p, p_len, p_number_of_known_bits, iterations):
    xs = []
    for i in range(0, iterations):
        p_prime = random_prime(2 ^ (p_len + 1) - 1, lbound=2 ^ p_len)
        xs.append(p_prime)
    xs[random.randint(0, len(xs) - 1)] = p
    xs = [p]
    ys = [x % (2 ^ (p_number_of_known_bits)) for x in xs]
    for y in ys:
        print(y)
        send_msg("localhost", 8888, int(y))


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage:")
        print("Either: sage rsa.sage --run [CONFIG]")
        print("Or: sage rsa.sage --test [CONFIG] [NUMBER OF ITERATIONS]")
        exit(0)

    config = load_parameters(sys.argv[2])
    print('Config:')
    print(json.dumps(config, indent=2) + '\n')

    if "--run" in sys.argv:
        # WARNING: This line is important. Otherwise we get a segfault. DO NOT REMOVE!!!!111
        F.<x> = PolynomialRing(Zmod(config['n']), implementation='NTL')
        start_server(config['host'], 8888, config['n'], config['p_len'], config['p_number_of_known_bits'],
                     config['number_of_workers'])

        print("\nTotal process time: " + total_process_time + "s\n")

    if "--test" in sys.argv:
        iterations = int(sys.argv[3])
        test_server(config['n'], config['p'], config['p_len'], config['p_number_of_known_bits'], iterations)
