
local CLoginService = {}
CLoginService.__index = CLoginService

function CLoginService:new()
	local self = {}
	setmetatable(self,CLoginService)
	self.name = 'LoginService'
	return self
end

function CLoginService:handle_msg_cs_login(sock,msg)
	local result, uid = check_user(msg.usr,msg.pwd)
	print(self.name,'usr:',msg.usr,'pwd:',msg.pwd)
	if result==0 then
		if Uid_Hid[uid] then
			print('account has login')
			result = -1
			uid = 0
		else
			print('login success','uid:',uid)
			Uid_Hid[uid] = sock
			Hid_Uid[sock] = uid
			Users[uid] = {uid=uid,name=msg.usr}
		end
	else
		print('login fail')
	end

	post_msg(sock,'msg_sc_login',result,uid)
	print ''
end


return CLoginService
