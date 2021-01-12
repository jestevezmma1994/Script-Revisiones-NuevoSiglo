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
import struct
import subprocess
import array
import binascii
import sys

HEADER_SIZE = (128 + 128 + 1024 + 128 + 1024)
ENCRYPTED_SIZE = (128 + 128 + 1024)
JCLIENT = 'jclient'

MAGIC = {
    'QBSH': 'static content verified using RSA',
    'QBDH': 'dynamic content verified using HMAC',
}

VERIFICATION_MODES = {
        0x01: 'RSA 2048 + SHA256',
        0x80: 'HMAC 256 bit key + SHA256',
    }

ENCRYPTION_MODES = {
        0x01: 'AES128-CBC (deprecated)',
        0x02: 'AES256-CBC',
        0x03: 'AES256-CBC-KEYCHAIN',
    }

def sign(data, keyno):
    cmd = [JCLIENT, 'sign', 'sha256', '-t', '-', 'with', str(keyno)]
    proc = subprocess.Popen(
        cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    signature, err = proc.communicate(data)

    if proc.wait():
        print(err, file=sys.stderr)
        raise SystemExit('Command failed: %s' % ' '.join(cmd))

    return signature.decode('hex')

def get_key(keyno):
    cmd = [JCLIENT, 'retrieve', str(keyno)]
    proc = subprocess.Popen(
        cmd,
        stdin=None,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    key, error = proc.communicate()

    if proc.wait():
        print(error, file=sys.stderr)
        raise SystemExit('Command failed: %s' % ' '.join(cmd))

    return key

def decrypt(data, key, iv):
    cmd = ['openssl', 'aes-256-cbc', '-d', '-K', key, '-iv', iv, '-nopad']
    proc = subprocess.Popen(
        cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    decrypted_data, error = proc.communicate(data)

    if proc.wait():
        print(error, file=sys.stderr)
        raise SystemExit('Command failed: %s' % ' '.join(cmd))

    return decrypted_data

def gen_hmac(data, keyno):
    key = get_key(keyno)

    cmd = ['openssl', 'dgst', '-sha256', '-mac', 'hmac', '-macopt', 'hexkey:%s' % key, '-binary']
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

def describe_value(val, mapping):
    return '%s (%s)' % (mapping.get(val, 'invalid'), str(val))

def describe_size(val):
    aes_blocks = val / 16
    aes_rest = val % 16
    if aes_rest:
        return '%i (%i AES blocks + %i bytes)' % (val, aes_blocks, aes_rest)
    else:
        return '%i (%i AES blocks)' % (val, aes_blocks)

def dump_data(bindata):
    return '%s ... %s' % (binascii.hexlify(bindata[0:3]), binascii.hexlify(bindata[len(bindata) - 3:len(bindata)]))

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--verify',
                        type=int,
                        default=None,
                        help='ID of the key used for verification')
    parser.add_argument('--key',
                        type=int,
                        default=None,
                        help='ID of the key used for decryption')
    parser.add_argument('--out',
                        type=argparse.FileType('wb'),
                        default=None,
                        help='Filename for decrypted image')
    parser.add_argument('--append-data', '-a',
                        default='',
                        help='Additional data to include in HMAC')
    parser.add_argument('--size-from-header', '-s', action='store_true',
                        default=False,
                        help='Data size used to hmac verification is taken from header.')
    parser.add_argument('input',
                        type=argparse.FileType('rb'),
                        help='Input file')

    args = parser.parse_args()

    header = array.array('c', args.input.read(HEADER_SIZE))

    main_key = get_key(args.key)
    header_iv = binascii.hexlify(header[ENCRYPTED_SIZE:ENCRYPTED_SIZE + 16])
    header[0:ENCRYPTED_SIZE] = array.array('c', decrypt(header[0:ENCRYPTED_SIZE], main_key, header_iv))

    encdata = args.input.read()

    magic, version, size, verification_mode = struct.unpack_from('<4sIII', header, 0)
    encryption_mode = struct.unpack_from('<I', header, 0x80)[0]
    image_iv = binascii.hexlify(header[0x84: 0x84 + 16])
    encrypted_key = header[0x94: 0x94 + 32]
    key_iv = header[0xB4: 0xB4 + 16]

    print('Summary of', args.input.name, file=sys.stderr)
    print('  magic:               ', describe_value(magic, MAGIC), file=sys.stderr)
    print('  verison:             ', version, file=sys.stderr)
    print('  encrypted image size:', describe_size(size), file=sys.stderr)
    print('  verification mode:   ', describe_value(verification_mode, VERIFICATION_MODES), file=sys.stderr)
    print('  encryption:          ', describe_value(encryption_mode, ENCRYPTION_MODES), file=sys.stderr)
    print('  header IV:           ', header_iv, file=sys.stderr)
    print('  image IV:            ', image_iv, file=sys.stderr)
    if encryption_mode == 0x03:
        print('  encrypted key:       ', binascii.hexlify(encrypted_key), file=sys.stderr)
        print('  key IV:              ', binascii.hexlify(key_iv), file=sys.stderr)

    if magic == 'QBSH':
        signature_length = 2048 / 8

        data_sig = header[ENCRYPTED_SIZE - 1024:ENCRYPTED_SIZE - 1024 + signature_length]
        head_sig = header[HEADER_SIZE - 1024:HEADER_SIZE - 1024 + signature_length]

        print('  image signature:     ', dump_data(data_sig), file=sys.stderr)
        print('  header signature:    ', dump_data(head_sig), file=sys.stderr)

    elif magic == 'QBDH':
        data_hmac = header[ENCRYPTED_SIZE - 1024:ENCRYPTED_SIZE - 1024 + 32]
        head_hmac = header[HEADER_SIZE - 1024:HEADER_SIZE - 1024 + 32]

        print('  image HMAC:          ', dump_data(data_hmac), file=sys.stderr)
        print('  header HMAC:         ', dump_data(head_hmac), file=sys.stderr)

    # do basic checks
    args.input.seek(0, 2)
    encrypted_data_size = args.input.tell() - HEADER_SIZE
    if encrypted_data_size != size:
        print('file size does not reflect size in header', describe_size(size), describe_size(encrypted_data_size), file=sys.stderr)

    if args.verify is not None:
        if magic == 'QBSH':
            data_sig2 = sign(encdata, args.verify)
            if data_sig != array.array('c', data_sig2):
                print('Content signature invalid.', file=sys.stderr)
            else:
                print('Content signature valid.', file=sys.stderr)
        elif magic == 'QBDH':
            if args.size_from_header:
                data_hmac2 = gen_hmac(encdata[:size] + args.append_data, args.verify)
            else:
                data_hmac2 = gen_hmac(encdata + args.append_data, args.verify)
            if data_hmac != array.array('c', data_hmac2):
                print('Content HMAC invalid.', file=sys.stderr)
            else:
                print('Content HMAC valid.', file=sys.stderr)
        else:
            raise SystemExit('Invalid magic -- skipping verification.')

    if args.out is not None:
        if encryption_mode == 0x03:
            dec_key = decrypt(encrypted_key, main_key, binascii.hexlify(key_iv))
        else:
            dec_key = binascii.unhexlify(main_key)
        args.out.write(decrypt(encdata, binascii.hexlify(dec_key), image_iv))

if __name__ == '__main__':
    main()
