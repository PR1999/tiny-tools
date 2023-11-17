import socket
import optparse

DEFAULT_TIMEOUT = 500 #timeout in ms
DEFAULT_PORT = 502 #modbus port
DEFAULT_FUNCTION = 17 #modbus function 17
DEFAULT_SID = 1 #slave id 1

def modbuspacket(sid, fcode, data=None, tid=None):
	#structure 2 byte tran id 2 byte protocol id 2 byte length 1 byte sid 1 byte function max 252 bytes data
	mbp = bytearray(b'\x00\x00\x00\x00\x00\x02\x01\x11')
	mbp[6] = sid
	mbp[7] = fcode
	if data is not None:
		mbp[5] = 2 +  len(bytearray.fromhex(data))
		mbp = mbp + bytearray.fromhex(data)
	if tid is not None:
		mbp[1]= tid
	return mbp
def main():
	p = optparse.OptionParser( description='modbus packet generator')
	p.add_option('--function','-f', type='int', dest='function', default=DEFAULT_FUNCTION)
	p.add_option('--port', '-p', type='int', dest='port', default=DEFAULT_PORT)
	p.add_option('--timeout', '-t', type='int', dest='timeout', default=DEFAULT_TIMEOUT)
	p.add_option('--slave-id', '-s', type='int', dest='sid', default=DEFAULT_SID)
	p.add_option('--debug', '-d', dest='debug', default=True)
	p.add_option('--data', type='string', dest='data')
	p.add_option('--tid',type='int', dest='tid', default=None)
	options, arguments = p.parse_args()
	assert(len(arguments) > 0)
	address = (arguments[0], options.port)
	print(f'target\t{arguments[0]}\tport\t{options.port}\tfunction\t{options.function}')
	
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	sock.settimeout(float(options.timeout)/float(1000))
	try:
		sock.connect(address)
	except socket.error:
		sock.close()
		print('failed to connect\n')
		exit()
	try:
		packet = modbuspacket(options.sid, options.function, data=options.data, tid=options.tid)
		sock.send(packet)
		if options.debug:
			print(packet.hex())
	except socket.error:
		sock.close()
		print('failed to send data')
		exit()
	try:
		r = sock.recv(1024)
	except socket.timeout:
		print('failed to receive, timed out')
		sock.close()
		exit()
	sock.close()
	print(r.hex())

try:
	main()
except KeyboardInterrupt:
	print('Closed by user')
