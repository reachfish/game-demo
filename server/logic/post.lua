
function post_msg(hid, ...)
	local arg = {...}
	local msg = CMessage.new(unpack(arg))
	if not msg then
		print 'msg create fail'
	else
		send_msg(hid,msg)
	end
end
