#!/usr/bin/env python3
"""Envia comandos MCS a CI_LAB por UDP, usando solo Python estandar.

Formato verificado para cFS_lab/cpu1 (Raspberry, native_std, sin EDS):
MID 0x18D1; cabecera CCSDS de 8 bytes; payload float32 little-endian.
SWITCH: CC 0, 9 bytes. CLOSE: CC 1, 8 bytes. SET_DUTY: CC 2, 12 bytes.
Si cambian las estructuras o la plataforma, revisar este formato.

Ejemplos desde la Raspberry:
    python3 mcs_send.py switch 0
    python3 mcs_send.py duty 30
    python3 mcs_send.py switch 1
    python3 mcs_send.py close
    python3 mcs_send.py --dry-run duty 30
Desde otra PC: agregar --host IP_DE_LA_RASPBERRY antes del comando.
"""

import argparse
import math
import socket
import struct
import sys

MCS_CMD_MID = 0x18D1


def duty_value(text):
    try:
        value = float(text)
    except ValueError as exc:
        raise argparse.ArgumentTypeError("El duty debe ser un numero.") from exc
    if not math.isfinite(value) or not 0 <= value <= 100:
        raise argparse.ArgumentTypeError("El duty debe estar entre 0 y 100 y ser finito.")
    return value


def port_value(text):
    try:
        value = int(text)
    except ValueError as exc:
        raise argparse.ArgumentTypeError("El puerto debe ser un entero.") from exc
    if not 1 <= value <= 65535:
        raise argparse.ArgumentTypeError("El puerto debe estar entre 1 y 65535.")
    return value


def build_packet(command, value=None, sequence=0):
    """Construye un paquete; no accede a la red ni al hardware."""
    if not 0 <= sequence <= 0x3FFF:
        raise ValueError("Secuencia fuera de rango.")

    if command == "switch":
        if value not in (0, 1):
            raise ValueError("Enable debe ser 0 o 1.")
        code, payload = 0, struct.pack("B", value)
    elif command == "close":
        code, payload = 1, b""
    elif command == "duty":
        if value is None or not math.isfinite(value) or not 0 <= value <= 100:
            raise ValueError("Duty invalido.")
        # CI_LAB passthrough no convierte el payload. La Raspberry usa LE.
        code, payload = 2, struct.pack("<f", value)
    else:
        raise ValueError("Comando desconocido.")

    total_size = 8 + len(payload)
    # Cabecera primaria en orden de red (big-endian).
    # 0xC000: paquete no segmentado. Longitud CCSDS = total - 7.
    packet = bytearray(struct.pack(">HHHBB", MCS_CMD_MID,
                                   0xC000 | sequence, total_size - 7, code, 0))
    packet.extend(payload)

    # Checksum de comandos cFE: XOR de todos los bytes debe ser 0xFF.
    checksum = 0xFF
    for byte in packet:
        checksum ^= byte
    packet[7] = checksum
    return bytes(packet)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--host", default="127.0.0.1", help="Host de CI_LAB (default: 127.0.0.1)")
    parser.add_argument("--port", type=port_value, default=1234, help="Puerto UDP de CI_LAB (default: 1234)")
    parser.add_argument("--dry-run", action="store_true", help="Mostrar paquete sin enviarlo")
    commands = parser.add_subparsers(dest="command", required=True)
    switch = commands.add_parser("switch", help="0 deshabilita; 1 habilita con el duty configurado")
    switch.add_argument("value", type=int, choices=(0, 1))
    duty = commands.add_parser("duty", help="Configurar porcentaje sin cambiar enable")
    duty.add_argument("value", type=duty_value)
    commands.add_parser("close", help="Deshabilitar y liberar PWM; requiere reinicializar para volver a usarlo")
    args = parser.parse_args(argv)

    packet = build_packet(args.command, getattr(args, "value", None))
    print(f"MCS {args.command}: MID=0x{MCS_CMD_MID:04X}, CC={packet[6]}, {len(packet)} bytes")
    print(f"Destino: {args.host}:{args.port}; paquete: {packet.hex(' ')}")
    if args.dry_run:
        print("Dry run: no se envio ningun paquete.")
        return 0

    try:
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
            sock.sendto(packet, (args.host, args.port))
    except OSError as exc:
        print(f"No se pudo enviar: {exc}", file=sys.stderr)
        return 1
    print("Datagrama enviado. UDP no confirma recepcion ni ejecucion; verificar en MCS.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

