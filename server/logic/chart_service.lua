
local CChartService = {}
CChartService.__index = CChartService

function CChartService:new()
	local self = {}
	setmetatable(self,CChartService)
	self.name = 'ChartService'
	return self
end

function CChartService:handle_msg_cs_chart(sock,msg)
	print(self.name .. ':', msg.uid, msg.word);
	print ''

	local uid = Hid_Uid[sock]
	for hid,uid in pairs(Hid_Uid)do
		if hid~=sock then
			post_msg(hid,'msg_sc_chart',uid,msg.word)
		end
	end
end


return CChartService
