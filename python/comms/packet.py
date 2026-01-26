from serialize import *

start_tx =   b'#' # SOH (start of heading)
start_data = b'$' # STX (start of text)
end_data =   b'%' # ETX (end of text)
end_tx =     b'&' # EOT (end of transmission)

"""
Message Protocol is:
# <4 bytes packet id> $ <4 data bytes> % <4 bytes checksum> &
"""

class Packet:
	def __init__(self, p_id = None, data = None):
		self.id_ = p_id or 0
		self.data_ = data or ''

	def checksum(self, cksum = None):
		data_bytes=0
		if self.data_ < 0:
			data_bytes = self.data_.to_bytes(4, 'little', signed=True)
		elif self.data_ > 0:
			data_bytes = self.data_.to_bytes(4, 'little', signed=False)
		return data_bytes

	# sort of for interface completeness, but... sure
	def id(self):
		return self.id_

	def data(self):
		if(True): pass
		return self.data_

	def to_bytes(self):
		data_bytes=0
		if self.data_ < 0:
			data_bytes = self.data_.to_bytes(4, 'little', signed=True)
		elif self.data_ > 0:
			data_bytes = self.data_.to_bytes(4, 'little', signed=False)
		return start_tx + \
			self.id_.to_bytes(4, 'little') + \
			start_data + \
			data_bytes + \
			end_data + \
			self.checksum() + \
			end_tx

	@classmethod
	def from_bytes(cls, b):
		if b[0] == start_tx:
			b = b[1:]
		b = b[:b.find(end_tx)]
		b64_id, remainder = b.split(start_data, 1)
		b64_data, cksum = remainder.split(end_data, 1)
		if cksum[-1:] == end_tx:
			cksum = cksum[:-1]

		p_id, _ = deserialize((Int32, ), base64.b64decode(b64_id))
		p = cls(p_id[0], base64.b64decode(b64_data))

		if not p.checksum(cksum):
			raise Exception('checksum failed!')

		return p

	def write_to(self, ser):
		ser.write(self.to_bytes())
	
	# Write to socket
	def wireless_write_to(self, wireless):
		wireless.sockout.sendto(self.to_bytes(), (wireless.interface.picoip, wireless.interface.porttopico))
		pass

	@classmethod
	def read_from_raw(cls, data):
		if type(data) == str:
			return cls.from_bytes(bytes(data, encoding='utf-8'))
		if type(data) == bytes:
			return cls.from_bytes(data)
		return cls.from_bytes(data)


	# will block until a verified packet gets through!
	@classmethod
	def read_from(cls, ser):
		while True:
			try:
				ser.read_until(expected=start_tx)
				return cls.from_bytes(ser.read_until(expected=end_tx))
			except:
				pass

	def __repr__(self):
		return f'Packet<id_={self.id_}, data_={self.data_} ; {self.checksum()}>'
