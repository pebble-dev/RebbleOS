import array
import sys

CRC_POLY = 0x04C11DB7

def process_word(data, crc=0xffffffff):
    if (len(data) < 4):
        d_array = array.array('B', data)
        for x in range(0, 4 - len(data)):
            d_array.insert(0,0)
        d_array.reverse()
        try:
            data = d_array.tobytes()
        except:
            data = d_array.tostring()

    d = array.array('I', data)[0]
    crc = crc ^ d

    for i in range(0, 32):
        if (crc & 0x80000000) != 0:
            crc = (crc << 1) ^ CRC_POLY
        else:
            crc = (crc << 1)

    result = crc & 0xffffffff
    return result

def process_buffer(buf, c = 0xffffffff):
    word_count = int(len(buf) / 4)
    if (len(buf) % 4 != 0):
        word_count += 1

    crc = c
    for i in range(0, word_count):
        crc = process_word(buf[i * 4 : (i + 1) * 4], crc)
    return crc

def crc32(data):
    return process_buffer(data)

if __name__ == '__main__':
    assert(0x89f3bab2 == process_buffer("123 567 901 34"))
    assert(0xaff19057 == process_buffer("123456789"))
    assert(0x519b130 == process_buffer("\xfe\xff\xfe\xff"))
    assert(0x495e02ca == process_buffer("\xfe\xff\xfe\xff\x88"))

    print("All tests passed!")

    if len(sys.argv) >= 2:
        b = open(sys.argv[1]).read()
        crc = crc32(b)
        print("%u or 0x%x" % (crc, crc))
