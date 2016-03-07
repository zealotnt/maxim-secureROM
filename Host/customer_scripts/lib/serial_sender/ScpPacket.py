__author__ = 'bvinot'


_ERROR_MSG = [['  ' for i in range(40)] for j in range(20)]
from BootloaderScp import BootloaderScp, ScpCmd
from utils import *


def load_error_file(chip_code, rom_code, verbose, chip_name):
    """
    Load the error file according to the chip code as define in SPEC98H06RevA-ARM_secure_micros_USN_format
    and the rom code version. The error message array is store in the global _ERROR_MSG variable
    :param chip_code:
    :param rom_code:
    :return:
    """
    global _ERROR_MSG
    try:
        if chip_name is None:
            if chip_code == "00":
                rom_code = "00000000"
                if verbose >= VERBOSE:
                    print "\n\tSample chip, Error identification May not work correctly"
            _CHIP_ERROR = __import__("errors.max" + chip_code + "_" + rom_code + "_error.", globals(), locals(), ['ERROR_MSG'], -1)
        else:
            _CHIP_ERROR = __import__("errors.max" + chip_name + "_error.", globals(), locals(), ['ERROR_MSG'], -1)

        _ERROR_MSG = _CHIP_ERROR.ERROR_MSG
    except ImportError:
        print "\tUnknown chip, Maybe you should update me :)"


class ScpPacket:

    is_connection_packet = False

    def __init__(self, file_path, packet_name, bl_scp, options, cmd, id, way):
        self.bl_scp = bl_scp
        self.scp_cmd = None
        self.timeout = options.timeout
        self.file_path = file_path
        self.way = way
        self.cmd = cmd
        self.id = id
        self.file_name = packet_name
        self.verbose = options.verbose
        self.chip_name = options.chip_name

        packet = open(get_fullpath(self.file_path, self.file_name), 'rb')
        if self.way:
            self.packet_data = packet.read()
        else:
            self.packet_data = ScpCmd(packet)
        packet.close()

        if self.packet_data is None:
            print_err("\nerror: unable to read file packet")
            raise Exception()

    def process(self):
        if self.way:
            self.send()
        else:
            self.receive()

    def send(self):
        if self.verbose >= EXTRA_VERBOSE:
            print self.id + ' SEND> ' + self.cmd

        try:
            self.bl_scp.setTimeout(self.timeout)
            self.bl_scp.write(self.packet_data)
            self.bl_scp.flush()
        except Exception as inst:
            print inst
            print_err("Error" + self.file_name)
            raise Exception()

    def receive(self):
        if self.verbose >= EXTRA_VERBOSE:
                print self.id + ' WAIT> ' + self.cmd
        try:
            self.scp_cmd = self.bl_scp.readPacket(self.is_connection_packet)
            if self.scp_cmd is None:
                print_err("\nerror: receiving packet failed")
                raise Exception()

            self.check()

        except:
            if not self.is_connection_packet:
                print_err("\nerror: read packet timeout error occur. #")
            raise Exception()

    def check(self):
        if self.packet_data != self.scp_cmd:
            msg = "\nerror: received packet is not the expected one"
            print_err(msg)
            err_code = ord(self.scp_cmd.data_bin[4])
            module = ord(self.scp_cmd.data_bin[7])
            print_err("error: " + _ERROR_MSG[module][err_code] + " (" + hex(module)[2:] + "," + hex(err_code)[2:] + ")")
            if self.verbose >= EXTRA_VERBOSE:
                print "======================================"
                print "Expected Command"
                print self.packet_data
                print "--------------------------------------"
                print "Received Command"
                print self.scp_cmd
                print "======================================"
            raise Exception()


class HelloReplyPacket(ScpPacket):
    def __init__(self, file_path, packet_name, bl_scp, options, cmd, id, way):
        ScpPacket.__init__(self, file_path, packet_name, bl_scp, options, cmd, id, way)

    def check(self):
        self.parse_hello_reply()

    def parse_hello_reply(self):
        """ Parse and print useful information from hello_reply packet
            Also load Error message
        :return:
        """
        usn = self.scp_cmd.data[44:70]
        rom_ver = self.scp_cmd.data[28:36]
        phase = self.scp_cmd.data[36:38]
        config = self.scp_cmd.data[42:44]

        load_error_file(str(usn[2:4]), str(rom_ver), self.verbose, self.chip_name)

        if self.verbose >= VERBOSE:
            print "\n======================================"
            print "\tROM Version : " + rom_ver[6:8] + rom_ver[4:6] + rom_ver[2:4] + rom_ver[0:2]
            print "\tPhase : " + str(phase)
            print "\tJTAG : " + ("Deactivated" if (int(config) & 0x80 == 0x80) else "Active")
            print "\tRework : " + ("Available" if (int(config) & 0x40 == 0x40) else "Not Available")
            print "\tUSN : " + str(usn)
            print "======================================"

class ErasePacket(ScpPacket):
    def __init__(self, file_path, packet_name, bl_scp, options, cmd, id, way):
        ScpPacket.__init__(self, file_path, packet_name, bl_scp, options, cmd, id, way)
        self.timeout = options.erase_timeout


class ConnectionPacket(ScpPacket):
    def __init__(self, file_path, packet_name, bl_scp, options, cmd, id, way):
        ScpPacket.__init__(self, file_path, packet_name, bl_scp, options, cmd, id, way)
        self.timeout = 0.2
        self.is_connection_packet = True

    def check(self):
        if self.packet_data != self.scp_cmd:
            raise Exception()


class DumpPacket(ScpPacket):
    dump_filename = None

    def __init__(self, file_path, packet_name, bl_scp, options, cmd, id, way):
        self.bl_scp = bl_scp
        self.scp_cmd = None
        self.timeout = options.timeout
        self.file_path = file_path
        self.way = way
        self.cmd = cmd
        self.id = id
        self.file_name = packet_name
        self.dump_filename = options.dump_filename

    def check(self):
        if self.packet_data != self.scp_cmd:
            raise Exception()

    def receive(self):
        print ""
        self.bl_scp.readDump(self.dump_filename)