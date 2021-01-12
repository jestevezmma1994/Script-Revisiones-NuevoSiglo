#!/usr/bin/env python

"""*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************"""

from __future__ import print_function
import argparse
import array
import struct
import subprocess
import sys
import binascii

JCLIENT = 'jclient'
QB_IMAGE_VERSION = 2

ENCRYPTION_MODES = {
    'aes128-cbc':          0x01,
    'aes256-cbc':          0x02,
    'aes256-cbc-keychain': 0x03,
}


def random_bytes(count):
    cmd = ['openssl', 'rand', str(count)]
    proc = subprocess.Popen(
        cmd,
        stdin=None,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    bytes, err = proc.communicate()

    if proc.wait():
        print(err, file=sys.stderr)
        raise SystemExit('Command failed: %s' % ' '.join(cmd))

    return bytes


def sign(data, key, offline):
    if offline:
        cmd = [ 'openssl', 'dgst', '-sha256', '-sign', key, '-binary' ]
    else:
        cmd = [JCLIENT, 'sign', 'sha256', '-t', '-', 'with', str(key)]

    proc = subprocess.Popen(
        cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    signature, err = proc.communicate(data)

    if proc.wait():
        print(err, file=sys.stderr)
        raise SystemExit('Command failed: %s' % ' '.join(cmd))

    if offline:
        return signature
    else:
        return signature.decode('hex')


def encrypt_chain(data, key, offline):
    random_key = random_bytes(32)
    image_iv = random_bytes(16)

    encrypted_key, key_iv = encrypt(random_key, key, offline)

    cmd = ['openssl', 'aes-256-cbc', '-nopad', '-nosalt', '-e', '-K', random_key.encode('hex'), '-iv', image_iv.encode('hex')]

    proc = subprocess.Popen(
        cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    data, err = proc.communicate(data)

    if proc.wait():
        print(err, file=sys.stderr)
        raise SystemExit('Command failed: %s' % ' '.join(cmd))

    return data, image_iv, encrypted_key, key_iv

def encrypt(data, key, offline):
    if offline:
        const_iv = random_bytes(16).encode('hex')
        cmd = ['openssl', 'aes-256-cbc', '-nopad', '-nosalt', '-e', '-K', key, '-iv', const_iv ]
    else:
        cmd = [JCLIENT, 'encrypt', '-t', '-', 'with', str(key), 'cbciv']

    proc = subprocess.Popen(
        cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    data, iv = proc.communicate(data)

    if proc.wait():
        print(iv, file=sys.stderr)
        raise SystemExit('Command failed: %s' % ' '.join(cmd))

    if offline:
        iv = const_iv

    if not iv.strip():
        raise SystemExit('Empty IV returned by jclient in cbciv mode')
    iv = iv.strip()[-32:].decode('hex')

    return data, iv


def gen_hmac(data, key, offline):
    if offline == False:
        cmd = [JCLIENT, 'retrieve', str(key)]
        proc = subprocess.Popen(
            cmd,
            stdin=None,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE)

        key, error = proc.communicate()

        if proc.wait():
            print(error, file=sys.stderr)
            raise SystemExit('Command failed: %s' % ' '.join(cmd))

    cmd = ['openssl', 'dgst', '-sha256', '-mac', 'hmac', '-macopt', 'hexkey:%s' % key.strip(), '-binary']
    proc = subprocess.Popen(
        cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    hmac, error = proc.communicate(data)

    if proc.wait():
        print(error, file=sys.stderr)
        raise SystemExit('Command failed: %s' % ' '.join(cmd))

    return hmac

def dump_data(bindata):
    return '%s ... %s' % (binascii.hexlify(bindata[0:3]), binascii.hexlify(bindata[len(bindata) - 3:len(bindata)]))

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--mode', '-m',
                        choices=['dynamic', 'static'],
                        default='static',
                        help='image mode')
    parser.add_argument('--encryption', '-e',
                        choices=list(ENCRYPTION_MODES),
                        default='aes256-cbc',
                        help='encryption mode')
    parser.add_argument('--append-data', '-a',
                        default='',
                        help='additional data to include in HMAC')
    parser.add_argument('--offline', '-o',
                        action='store_true',
                        help='encrypt/sign in offline mode')
    parser.add_argument('encryption_key',
                        help='jclient key id used for encryption or key in hex for offline mode')
    parser.add_argument('sign_key',
                        help='jclient key id used for signing/HMAC generation or key (hex for HMAC/filename for RSA) for offline mode')
    parser.add_argument('input',
                        type=argparse.FileType('rb'),
                        help='input file')
    parser.add_argument('output',
                        type=argparse.FileType('wb'),
                        help='output file')

    args = parser.parse_args()

    input_data = args.input.read()

    if len(input_data) % 16:
        # input data size is not a multiple of 16 -- add random padding
        pad_size = 16 - len(input_data) % 16
        input_data += random_bytes(pad_size)

    # create header
    header = array.array('c', random_bytes(0x0980))

    # encrypt the input data
    if 'keychain' in args.encryption:
        encrypted_data, iv, encrypted_key, key_iv = encrypt_chain(input_data, args.encryption_key, args.offline)
    else:
        encrypted_data, iv = encrypt(input_data, args.encryption_key, args.offline)
        encrypted_key = random_bytes(32)
        key_iv = random_bytes(16)

    # store encryption information
    struct.pack_into('<I', header, 0x80, ENCRYPTION_MODES[args.encryption])

    header[0x84:0x84 + len(iv)] = array.array('c', iv)
    header[0x94:0x94 + len(encrypted_key)] = array.array('c', encrypted_key)
    header[0xB4:0xB4 + len(key_iv)] = array.array('c', key_iv)

    if args.mode == 'static':
        # static mode
        struct.pack_into('<4s', header, 0, 'QBSH')
        # store general info
        struct.pack_into('<III', header, 4, QB_IMAGE_VERSION, len(encrypted_data), 0x01)

        # sign data
        signature = sign(encrypted_data, args.sign_key, args.offline)
        signature_length = len(signature)
        header[0x0100: 0x0100 + signature_length] = array.array('c', signature)

        # encrypt header
        encrypted_header, header_iv = encrypt(header[0:0x0500], args.encryption_key, args.offline)
        header[0:0x0500] = array.array('c', encrypted_header)
        header[0x0500:0x0510] = array.array('c', header_iv)

        # sign header
        header_signature = sign(header[0:0x0580], args.sign_key, args.offline)
        signature_length = len(header_signature)
        header[0x0580:0x0580 + signature_length] = array.array('c', header_signature)
        print("QB Static image", file=sys.stderr)
        print("    Version:         ", QB_IMAGE_VERSION, file=sys.stderr)
        print("    Size:            ", len(encrypted_data), file=sys.stderr)
        print("    Encryption mode: ", args.encryption, file=sys.stderr)
        print("    Image signature: ", dump_data(signature), file=sys.stderr)
        print("    Header signature:", dump_data(header_signature), file=sys.stderr)
    else:
        # dynamic mode
        struct.pack_into('<4s', header, 0, 'QBDH')
        # store general info
        struct.pack_into('<III', header, 4, QB_IMAGE_VERSION, len(encrypted_data), 0x80)

        # generate image hmac
        data_hmac = gen_hmac(encrypted_data + args.append_data, args.sign_key, args.offline)
        header[0x0100: 0x0100 + 32] = array.array('c', data_hmac)

        # encrypt header
        encrypted_header, header_iv = encrypt(header[0:0x0500], args.encryption_key, args.offline)
        header[0:0x0500] = array.array('c', encrypted_header)
        header[0x0500:0x0510] = array.array('c', header_iv)

        # generate header hmac
        header_hmac = gen_hmac(header[0:0x0580] + array.array('c', args.append_data), args.sign_key, args.offline)
        header[0x0580: 0x0580 + 32] = array.array('c', header_hmac)
        print("QB Dynamic image", file=sys.stderr)
        print("    Version:         ", QB_IMAGE_VERSION, file=sys.stderr)
        print("    Size:            ", len(encrypted_data), file=sys.stderr)
        print("    Encryption mode: ", args.encryption, file=sys.stderr)
        print("    Append data:     ", args.append_data);
        print("    Image HMAC:      ", dump_data(data_hmac), file=sys.stderr)
        print("    Header HMAC:     ", dump_data(header_hmac), file=sys.stderr)

    # write the data to the file
    args.output.write(header)
    args.output.write(encrypted_data)

if __name__ == '__main__':
    main()
