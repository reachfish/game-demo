

local CHeartBeatService = {}
CHeartBeatService.__index = CHeartBeatService

function CHeartBeatService:new()
	local self = {}
	setmetatable(self,CHeartBeatService)
	self.name = 'HeartBeatService'
	return self
end

function CHeartBeatService:handle_msg_cs_heart_beat(sock,msg)
	post_msg(sock,'msg_sc_heart_beat',0)
end


return CHeartBeatService
