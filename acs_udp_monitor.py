#!/usr/bin/env python3
"""Muestra en vivo la telemetría ACS recibida desde TO Lab por UDP.

Solo usa la biblioteca estándar de Python. Espera el paquete ACS con MID 0x08D0:
encabezado de 16 bytes, SampleCounter uint32 y seis float32 (aceleración en g,
giro en grados/s), en little endian.
"""

import argparse
from collections import deque
from datetime import datetime
import socket
import struct
import sys
import time


def number(value: str) -> int:
    return int(value, 0)


def arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bind", default="0.0.0.0", help="IP local donde escuchar (por defecto, todas)")
    parser.add_argument("--port", type=int, default=2234, help="Puerto UDP de TO Lab (2234)")
    parser.add_argument("--mid", type=number, default=0x08D0, help="MID de ACS (0x08D0)")
    parser.add_argument("--offset", type=int, default=16, help="Byte inicial de SampleCounter (16)")
    parser.add_argument("--endian", choices=("little", "big"), default="little")
    parser.add_argument("--refresh", type=float, default=10.0, help="Actualizaciones de pantalla por segundo (10)")
    parser.add_argument("--lines", action="store_true", help="Imprimir cada paquete ACS en una línea")
    parser.add_argument("--raw", action="store_true", help="Con --lines, incluir todos los bytes en hex")
    parser.add_argument("--duration", type=float, help="Terminar después de esta cantidad de segundos")
    args = parser.parse_args()
    if not 1 <= args.port <= 65535 or args.offset < 0 or args.refresh <= 0:
        parser.error("port debe estar entre 1 y 65535; offset >= 0; refresh > 0")
    if args.raw and not args.lines:
        parser.error("--raw requiere --lines")
    if args.duration is not None and args.duration <= 0:
        parser.error("duration debe ser > 0")
    return args


def main() -> int:
    args = arguments()
    format_data = "<I6f" if args.endian == "little" else ">I6f"
    data_size = struct.calcsize(format_data)
    received_at = deque()
    total_udp = acs_packets = other_packets = short_packets = jumps = 0
    last_counter = last_packet_at = last_values = None
    started_at = time.monotonic()
    deadline = started_at + args.duration if args.duration is not None else None
    next_refresh = started_at + 1 / args.refresh
    last_width = 0

    try:
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
            sock.bind((args.bind, args.port))
            print(f"Escuchando UDP {args.bind}:{args.port}, ACS MID 0x{args.mid:04X}. Ctrl+C para salir.")

            while True:
                now = time.monotonic()
                if deadline is not None and now >= deadline:
                    break
                wait_until = min(next_refresh, deadline) if deadline is not None else next_refresh
                sock.settimeout(max(0.001, wait_until - now))

                try:
                    packet, source = sock.recvfrom(4096)
                except socket.timeout:
                    packet = None

                if packet is not None:
                    total_udp += 1
                    mid = int.from_bytes(packet[:2], "big") if len(packet) >= 2 else None
                    if mid != args.mid:
                        other_packets += 1
                    elif len(packet) < args.offset + data_size:
                        short_packets += 1
                    else:
                        now = time.monotonic()
                        counter, ax, ay, az, gx, gy, gz = struct.unpack_from(
                            format_data, packet, args.offset
                        )
                        last_values = (ax, ay, az, gx, gy, gz)
                        acs_packets += 1
                        received_at.append(now)
                        last_packet_at = now

                        if last_counter is not None:
                            delta = (counter - last_counter) & 0xFFFFFFFF
                            if 1 < delta < 0x80000000:
                                jumps += delta - 1
                        last_counter = counter

                        if args.lines:
                            stamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
                            line = (f"{stamp} {source[0]} MID=0x{mid:04X} "
                                    f"SampleCounter={counter} "
                                    f"accel[g]=({ax:+.3f}, {ay:+.3f}, {az:+.3f}) "
                                    f"gyro[deg/s]=({gx:+.2f}, {gy:+.2f}, {gz:+.2f}) "
                                    f"bytes={len(packet)}")
                            if args.raw:
                                line += f" raw={packet.hex(' ')}"
                            print(line)

                now = time.monotonic()
                if now >= next_refresh:
                    while received_at and received_at[0] <= now - 1:
                        received_at.popleft()
                    if not args.lines:
                        elapsed = min(1.0, max(0.001, now - started_at))
                        rate = len(received_at) / elapsed
                        age = "--" if last_packet_at is None else f"{now - last_packet_at:.2f}s"
                        value = "--" if last_counter is None else str(last_counter)
                        line = (f"ACS={value} | {rate:5.1f} pkt/s | recibidos={acs_packets} "
                                f"| saltos={jumps} | otros MID={other_packets} | edad={age}")
                        if last_values is not None:
                            ax, ay, az, gx, gy, gz = last_values
                            line += (f" | accel[g]=({ax:+.3f}, {ay:+.3f}, {az:+.3f})"
                                     f" | gyro[deg/s]=({gx:+.2f}, {gy:+.2f}, {gz:+.2f})")
                        if short_packets:
                            line += f" | cortos={short_packets}"
                        if sys.stdout.isatty():
                            print("\r" + line.ljust(last_width), end="", flush=True)
                            last_width = len(line)
                        else:
                            print(line, flush=True)
                    next_refresh = now + 1 / args.refresh
    except KeyboardInterrupt:
        pass
    except OSError as error:
        print(f"Error de red: {error}", file=sys.stderr)
        return 1

    if not args.lines and sys.stdout.isatty():
        print()
    elapsed = time.monotonic() - started_at
    print(f"Resumen: ACS={acs_packets}, total UDP={total_udp}, "
          f"promedio={acs_packets / elapsed:.1f} pkt/s, saltos={jumps}, cortos={short_packets}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
