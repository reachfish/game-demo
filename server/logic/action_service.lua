
local CActionService = {}
CActionService.__index = CActionService

function CActionService:new()
	local self = {}
	setmetatable(self,CActionService)
	self.name = 'ActionService'
	return self
end

function CActionService:handle_msg_cs_login(sock,msg)
	print(self.name .. ':', 'from', msg.x1,msg.y1, 'to', msg.x2, msg.y2)
end


return CActionService
