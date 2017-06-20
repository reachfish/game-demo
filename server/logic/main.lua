
require "data"
require "post"

local service_handler = (require "service_handler"):new()

function handle_msg(sock,msg_name,msg)
	service_handler:handle_msg(sock,msg_name,msg)
end

function test()
	print "i am testing"
end

function disconnect_client(hid)
	uid = Hid_Uid[hid]
	Hid_Uid[hid] = nil
	if uid then
		Uid_Hid[uid] = nil
		Users[uid] = nil	
	end
end

