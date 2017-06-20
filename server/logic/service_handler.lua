
local LOGIN_SERVICE = 1
local CHART_SERVICE = 2
local ACTION_SERVICE = 3
local HEARTBEAT_SERVICE = 4

local all_msg_services = {
	msg_cs_login = LOGIN_SERVICE,
	msg_cs_move = ACTION_SERVICE,
	msg_cs_chart = CHART_SERVICE,
	msg_cs_heart_beat = HEARTBEAT_SERVICE,
}

local CServiceHandler = {}
CServiceHandler.__index = CServiceHandler

function CServiceHandler:new()
	local self = {}
	setmetatable(self,CServiceHandler)
	self.services = {
		[LOGIN_SERVICE] = (require "login_service"):new(),
		[CHART_SERVICE] = (require "chart_service"):new(),
		[ACTION_SERVICE] = (require "action_service"):new(),
		[HEARTBEAT_SERVICE] = (require "heart_beat_service"):new()
	}
	return self 
end

function CServiceHandler:handle_msg(sock,msg_name,msg)
	local func_name = 'handle_' .. msg_name
	local service_name = all_msg_services[msg_name]
	local service = self.services[service_name]
	if not service then
		print ('no msg service:',msg_name)
	else
		(service[func_name])(service,sock,msg)
	end
end

return CServiceHandler




