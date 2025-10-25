from typing import List, Dict
import subprocess
import argparse
import telnetlib
from ss_oocd import *
import can
import struct
import time


def sign_img(   img : str,
                signed_img : str,
                key_path : str = "root-rsa-2048.pem",
                img_tool_path : str = "imgtool.py"):

    

    cmd = [
        "python3",
        img_tool_path,
        "sign",
        "--key",
        key_path,
        "--header-size",
        "0x200",
        "--pad-header",
        "--align",
        "4",
        "--version",
        "1.0.0",
        "--slot-size",
        "0x60000",
        "--pad",
        "--confirm",
        img,
        signed_img
    ]

    print(" ".join(cmd))

    subprocess.run(cmd)

def get_img_sign_name(name : str):
    parts : List[str] = name.split("/")
    file = parts[2]
    file_parts : List[str] = file.split(".")

    first_part = "/".join(parts[0:len(parts)-1])
    first_part += "/"

    second_part = ".".join([file_parts[0], "signed", "confirmed", "bin"])

    new_file = first_part + second_part

    print(new_file)

    return new_file




def telnet_flash(   img : str = "../../bp_test.bin",
                    position : str = "0x08020000"):

    if oocd_is_running() == False:
        oocd_start()
    
    signed_img_name : str = get_img_sign_name(name=img)
    sign_img(
        img = img,
        signed_img = signed_img_name
    )


    try:
        tn = telnetlib.Telnet("localhost", 4444, timeout=5)
        tn.read_until(b">", timeout=2)


        tn.write(b"reset halt\n")
        response = tn.read_until(b">", timeout=5).decode('utf-8')
        print(response)


        cmd = [
            "flash",
            "write_image",
            "erase",
            signed_img_name,
            position,
            "\n"
        ]
        tn.write(" ".join(cmd).encode())
        response = tn.read_until(b">", timeout=30).decode('utf-8')
        print(response)


        tn.write(b"reset run\n")
        response = tn.read_until(b">", timeout=5).decode('utf-8')
        print(response)

        tn.close()
        print("Flash Abgeschlossen")

    except Exception as e:
            print(f"Fehler beim Flashen: {e}")

    if oocd_is_running() == True:
        oocd_stop()


def flash_can_state(can_socket : str = "can0"):
    try:
        cmd = [
            "sudo",
            "ip",
            "link",
            "show",
            can_socket
        ]

        result = subprocess.check_output(cmd, stderr=subprocess.STDOUT, text=True).lower()

        print(result)
        if "state up" in result:
            return True

        return False

    except subprocess.CalledProcessError:
        return False

def enable_flash_can(   can_socket : str = "can0",
                        bitrate : int = 1000000):
    cmd = [
        "sudo",
        "ip",
        "link",
        "set",
        can_socket,
        "up",
        "type",
        "can",
        "bitrate",
        str(bitrate)
    ]

    subprocess.run(cmd)

def disable_flash_can(  can_socket : str = "can0"):
    cmd = [
        "sudo",
        "ip",
        "link",
        "set",
        can_socket,
        "down"
    ]

    subprocess.run(cmd)


def wait_for_acknowledgment(bus, expected_can_id, expected_word, timeout=1.0, max_retries=3):
    start_time = time.time()
    
    while (time.time() - start_time) < timeout:
        msg = bus.recv(timeout=0.1)
        
        if msg is not None:

            if msg.arbitration_id == expected_can_id:

                if len(msg.data) >= 4:
                    received_word = struct.unpack('<I', msg.data[0:4])[0]
                    

                    if received_word == expected_word:
                        return True
                    else:
                        print(f"  ⚠ Daten-Mismatch! Erwartet: 0x{expected_word:08X}, Empfangen: 0x{received_word:08X}")
                        return False
                else:
                    print(f"  ⚠ Antwort zu kurz: {len(msg.data)} Bytes")
                    return False
    
    print(f"Timeout: Keine Bestätigung innerhalb von {timeout}s empfangen")
    return False


def send_firmware_via_can(  binary_file,
                            can_id,
                            can_channel='can0',
                            flash_offset=0x00, 
                            delay_ms=6, 
                            ack_timeout=3.0, 
                            max_retries=3):

    signed_img_name : str = get_img_sign_name(name=binary_file)
    sign_img(
        img = binary_file,
        signed_img = signed_img_name
    )

    binary_file = signed_img_name

    if flash_can_state() ==  False:
        print("CAN UP")
        enable_flash_can()


    try:
        bus = can.Bus(channel=can_channel, interface='socketcan')
        print(f"CAN-Bus initialisiert: {bus.channel_info}")
    except Exception as e:
        print(f"Fehler beim Öffnen des CAN-Bus: {e}")
        return False
    

    try:
        with open(binary_file, 'rb') as f:
            firmware_data = f.read()
        file_size = len(firmware_data)
        print(f"Firmware-Größe: {file_size} Bytes ({file_size/1024:.2f} KB)")
    except Exception as e:
        print(f"Fehler beim Lesen der Datei: {e}")
        return False
    

    address = 0
    total_words = (file_size + 3) // 4
    sent_frames = 0
    failed_frames = 0
    

    ack_id = can_id + 1
    
    print(f"Sende {total_words} Wörter...")
    print(f"Request-ID: 0x{can_id:03X}, Response-ID: 0x{ack_id:03X}")
    print(f"ACK-Timeout: {ack_timeout}s, Max. Wiederholungen: {max_retries}")
    print("=" * 60)
    
    for i in range(0, file_size, 4):
        
        chunk = firmware_data[i:i+4]
        if len(chunk) < 4:
            chunk = chunk + b'\xFF' * (4 - len(chunk))
        

        word = struct.unpack('<I', chunk)[0]
        flash_addr = flash_offset + address
        

        if word != 0xFFFFFFFF:

            data = struct.pack('<II', word, flash_addr)
            
            msg = can.Message(
                arbitration_id=can_id,
                data=data,
                is_extended_id=False
            )
            
            retry_count = 0
            success = False
            
            while retry_count <= max_retries and not success:
                try:
                    bus.send(msg)
                    sent_frames += 1
                    
                    if wait_for_acknowledgment(bus, ack_id, word, ack_timeout):
                        success = True
                        
                        if sent_frames % 100 == 0:
                            progress = (sent_frames / total_words) * 100
                            print(f"Fortschritt: {sent_frames}/{total_words} ({progress:.1f}%)")
                        
                        if delay_ms > 0:
                            time.sleep(delay_ms / 1000.0)
                    else:
                        retry_count += 1
                        if retry_count <= max_retries:
                            print(f"  → Wiederhole Frame {sent_frames} (Versuch {retry_count}/{max_retries})...")
                        else:
                            print(f"  ✗ Frame {sent_frames} fehlgeschlagen nach {max_retries} Versuchen!")
                            failed_frames += 1
                            if failed_frames > 10:
                                print(f"\n⚠ Zu viele Fehler ({failed_frames}), Übertragung abgebrochen!")
                                bus.shutdown()
                                return False
                    
                except can.CanError as e:
                    print(f"Fehler beim Senden von Frame {sent_frames}: {e}")
                    retry_count += 1
                    if retry_count > max_retries:
                        failed_frames += 1
        
        address += 4
    
    print("\n" + "=" * 60)
    print(f"Übertragung abgeschlossen!")
    print(f"Erfolgreich gesendet: {sent_frames} Frames")
    print(f"Fehlgeschlagen: {failed_frames} Frames")
    print(f"Adressbereich: 0x{flash_offset:08X} - 0x{flash_offset + address:08X}")
    
    bus.shutdown()
    return failed_frames == 0



def telnet_flash_handle(args):
    telnet_flash()

def can_flash_handle(args):
    can_id = int(args.id, 16)
    send_firmware_via_can(args.bin_file, can_id)


def flash_add_sub(sub):
    parser_flash = sub.add_parser("flash", help="flash via telnet and stlink")
    parser_flash.set_defaults(func=telnet_flash_handle)


    parser_canflash = sub.add_parser("canflash", help="flash via can")
    parser_canflash.add_argument("bin_file", help="signed bin file")
    parser_canflash.add_argument("id", type=str, help="can id to send image")
    parser_canflash.set_defaults(func=can_flash_handle)


def main():
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="cmd")

    flash_add_sub(sub)

    args = parser.parse_args()

    if hasattr(args, 'func'):
        args.func(args)
    else:
        parser.print_help()

if __name__ == '__main__':
    main()