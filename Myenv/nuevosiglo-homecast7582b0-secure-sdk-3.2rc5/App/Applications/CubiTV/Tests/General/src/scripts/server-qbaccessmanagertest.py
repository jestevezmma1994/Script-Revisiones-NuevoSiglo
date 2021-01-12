from SimpleXMLRPCServer import SimpleXMLRPCServer
from SimpleXMLRPCServer import SimpleXMLRPCRequestHandler
import sys

# Restrict to particular paths.
class RequestHandler(SimpleXMLRPCRequestHandler):
    rpc_paths = ('/', '/RPC2')

# Create server
server = SimpleXMLRPCServer(("localhost", 0),
                            requestHandler=RequestHandler,
		            logRequests=True)
ip, port = server.server_address
server.register_introspection_functions()

#INNOV8ON plugin response methods:
#VerifyUserPIN - returns false for pin 9999 and authenticates user for other pins
def VerifyUserPIN(customerId,password,pinType):
	if password == '9999':
		return False
	else:
		return 'OK'
server.register_function(VerifyUserPIN, 'stb.VerifyUserPIN')

#ChangeUserPIN - pretends that user pin was changed
def ChangeUserPIN(customerId,password,newPassword,pinType):
    return True
server.register_function(ChangeUserPIN, 'stb.ChangeUserPIN')

# Communicate server process port to the parent process
print "{}".format(port)
# flush the print to make sure that parent process will obtain port number
sys.stdout.flush()
# Run the server's main loop
server.serve_forever()
