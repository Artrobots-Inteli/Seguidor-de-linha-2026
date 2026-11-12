#!/usr/bin/env python3
import argparse
import fcntl
import os
import select
import struct
import sys
import termios
import time

TIOCMBIC = 0x5417
TIOCMBIS = 0x5416
TIOCM_DTR = 0x002
TIOCM_RTS = 0x004

BAUD = {
    300: termios.B300,
    600: termios.B600,
    1200: termios.B1200,
    2400: termios.B2400,
    4800: termios.B4800,
    9600: termios.B9600,
    19200: termios.B19200,
    38400: termios.B38400,
    57600: termios.B57600,
    115200: termios.B115200,
    230400: termios.B230400,
    460800: termios.B460800,
    500000: getattr(termios, "B500000", termios.B460800),
    921600: getattr(termios, "B921600", termios.B460800),
    1000000: getattr(termios, "B1000000", termios.B921600),
}


def open_port(port: str, baud: int, reset: bool) -> int:
    if baud not in BAUD:
        raise SystemExit(f"baudrate não suportado: {baud}")

    fd = os.open(port, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)

    attrs = termios.tcgetattr(fd)
    iflag, oflag, cflag, lflag, _ispeed, _ospeed, cc = attrs
    iflag = 0
    oflag = 0
    lflag = 0
    cflag = termios.CS8 | termios.CREAD | termios.CLOCAL
    cc = list(cc)
    cc[termios.VMIN] = 0
    cc[termios.VTIME] = 0
    termios.tcsetattr(
        fd,
        termios.TCSANOW,
        [iflag, oflag, cflag, lflag, BAUD[baud], BAUD[baud], cc],
    )
    termios.tcflush(fd, termios.TCIOFLUSH)

    if reset:
        # força o reset do ESP32: asserta DTR+RTS e depois solta,
        # deixando o chip em modo de execução (IO0 alto, EN alto)
        fcntl.ioctl(fd, TIOCMBIS, struct.pack("I", TIOCM_DTR | TIOCM_RTS))
        time.sleep(0.2)
        fcntl.ioctl(fd, TIOCMBIC, struct.pack("I", TIOCM_DTR | TIOCM_RTS))
    else:
        fcntl.ioctl(fd, TIOCMBIC, struct.pack("I", TIOCM_DTR | TIOCM_RTS))

    time.sleep(0.15)
    termios.tcflush(fd, termios.TCIOFLUSH)

    return fd


def main() -> int:
    ap = argparse.ArgumentParser(description="Monitor serial com controle de DTR/RTS")
    ap.add_argument("port")
    ap.add_argument("-b", "--baud", type=int, default=115200)
    ap.add_argument("-t", "--timestamp", action="store_true")
    ap.add_argument("--no-reset", action="store_true")
    args = ap.parse_args()

    try:
        fd = open_port(args.port, args.baud, reset=not args.no_reset)
    except OSError as exc:
        print(f"erro ao abrir {args.port}: {exc}", file=sys.stderr)
        return 1

    sys.stderr.write(f">> monitorando {args.port} @ {args.baud} baud (Ctrl+C para sair)\n")
    sys.stderr.flush()

    buf = b""
    try:
        while True:
            ready, _, _ = select.select([fd], [], [], 1.0)
            if not ready:
                continue
            try:
                data = os.read(fd, 4096)
            except BlockingIOError:
                continue
            except OSError:
                break
            if not data:
                continue
            buf += data
            while b"\n" in buf:
                line, buf = buf.split(b"\n", 1)
                text = line.decode(errors="replace").rstrip("\r")
                if args.timestamp:
                    stamp = time.strftime("%H:%M:%S")
                    sys.stdout.write(f"[{stamp}] {text}\n")
                else:
                    sys.stdout.write(f"{text}\n")
                sys.stdout.flush()
    except KeyboardInterrupt:
        pass
    finally:
        if buf:
            sys.stdout.write(buf.decode(errors="replace") + "\n")
            sys.stdout.flush()
        os.close(fd)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
