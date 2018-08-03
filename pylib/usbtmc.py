import usb.core
import usb.util
import struct

AGILENT_34461A = {'vendor':0x2a8d, 'product':0x1301}

class instrument(object):
    def __init__(self, find):
        # variables
        self.usb_dev = None
        self.usb_config = None
        self.usb_interface = None
        self.usb_in = None
        self.usb_out = None
        self.usb_interrupt = None
        self.usb_msg_btag = 0
        self.termination_char = None
        self.timeout = 10000

        # search for usb
        if 'vendor' in find and 'product' in find:
            # search by vendor and product
            self.usb_dev=usb.core.find(idVendor=find['vendor'], idProduct=find['product'])
            if self.usb_dev is None:
                raise ValueError('Failed to find device')
        else:
            raise ValueError('Failed to find search criteria')
        # TODO add more ways of finding device

        # get control of device
        if self.usb_dev.is_kernel_driver_active(0):
            self.usb_dev.detach_kernel_driver(0)

        # init device
        self.usb_dev.set_configuration()
        self.usb_dev.set_interface_altsetting()

        # find interface
        for config in self.usb_dev:
            for interface in config:
                if interface.bInterfaceClass is 0xfe and interface.bInterfaceSubClass is 3:
                    self.usb_interface=interface
                    self.usb_config=config
                    break

        if self.usb_interface is None or self.usb_config is None:
            raise ValueError('Failed to find usbtmc interface')

        # set up endpoints
        for endpoint in self.usb_interface:
            ep_type=endpoint.bmAttributes & usb.ENDPOINT_TYPE_MASK
            if ep_type is usb.ENDPOINT_TYPE_BULK:
                if endpoint.bEndpointAddress & usb.ENDPOINT_DIR_MASK is usb.ENDPOINT_IN:
                    self.usb_in=endpoint
                elif endpoint.bEndpointAddress & usb.ENDPOINT_DIR_MASK is usb.ENDPOINT_OUT:
                    self.usb_out=endpoint
            elif ep_type is usb.ENDPOINT_TYPE_INTERRUPT:
                if endpoint.bEndpointAddress & usb.ENDPOINT_DIR_MASK is usb.ENDPOINT_IN:
                    self.usb_interrupt=endpoint

        if self.usb_in is None or self.usb_out is None or self.usb_interrupt is None:
            raise ValueError('Failed to find one or more of the devices endpoints')

    def _write(self, data):
        data_length=len(data)
        offset=0
        data_left=data_length
        last_msg=False

        while data_left:
            if data_left<=1048576: # max message size - 1024^2
                last_msg=True

            # packet data
            msg_data=data[offset:offset+1048576]
            msg_len=len(msg_data)

            # calculate message header
            self.usb_msg_btag=(self.usb_msg_btag%0xff)+1 # tags run from 1-255, never 0
            # message id, btag, ~btag
            header=struct.pack('BBBx', 1, self.usb_msg_btag, (~self.usb_msg_btag)&0xff)
            # message length, end of message identifier
            header+=struct.pack('<LBxxx', msg_len, last_msg)

            # calculate required padding
            padding=b'\0'*((4-(msg_len%4))%4)

            # send message
            self.usb_out.write(header+msg_data+padding)

            # update variables
            offset+=msg_len
            data_left=data_length-offset
            
    def _read(self, nbytes=-1):
        last_msg=False
        offset=0
        
        if nbytes is 0:
            return None

        if nbytes is -1 or nbytes>1048576:
            data_length=1048576
        else:
            data_length=nbytes # TODO should this be padded so that it is divisble by 4?

        data=b''

        while not last_msg:
            # calculate message header
            self.usb_msg_btag=(self.usb_msg_btag%0xff)+1 # tags run from 1-255, never 0
            # message id, btag, ~btag
            header=struct.pack('BBBx', 2, self.usb_msg_btag, (~self.usb_msg_btag)&0xff)
            if self.termination_char is None:
                header+=struct.pack('<Lxxxx', data_length) # transfer size
            else:
                # transfer size, transfer attributes, terminating character
                header+=struct.pack('<LBBxx', data_length, 2, str(self.termination_char).encode('utf-8')[0])

            # send receive request
            self.usb_out.write(header)

            # read in message
            msg=self.usb_in.read(data_length+12, timeout=self.timeout).tostring()

            # determine if last message
            last_msg=struct.unpack_from('<Bxxx', msg, 8)[0]&1

            # store data
            data+=msg[12:]

            # update variables
            offset+=data_length
            if nbytes is -1 or (nbytes-offset)>1048576:
                data_length=1048576
            else:
                data_length=nbytes-offset

        return data

    def write(self, data):
        self._write(str(data).encode('utf-8'))

    def read(self, nbytes=-1):
        return self._read(nbytes).decode('utf-8').rstrip('\r\n')

    def ask(self, msg):
        self.write(msg)
        return self.read()

    def usb_info_string(self):
        """ Returns a string containing the information gained from the USB device
        """
        manufacturer=usb.util.get_string(self.usb_dev, self.usb_dev.iManufacturer)
        product=usb.util.get_string(self.usb_dev, self.usb_dev.iProduct)
        serial=usb.util.get_string(self.usb_dev, self.usb_dev.iSerialNumber)
        return "Manufacturer: {0}, product: {1}, serial: {2}".format(manufacturer, product, serial)

    def take_screenshot(self):
        """ Takes a screenshot from the instrument, using the current format.
            This returns the data for the caller to write to a file
        """
        # request the image
        self.write('HCOP:SDUM:DATA?')
        data=self._read()

        # data is sent as a definite length block
        # this starts with a '#', followed by a single digit representing the number of characters
        # used to represent the data size, which is followed by the data
        if data[0] is not '#':
            raise ValueError('Image missing header, data returned {0}'.format(data))

        # read sizes
        header_size=2+int(data[1])
        image_size=int(data[2:header_size])
        
        # return image
        return data[header_size:header_size+image_size]
