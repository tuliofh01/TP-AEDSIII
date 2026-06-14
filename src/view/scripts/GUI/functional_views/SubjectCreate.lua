-- SubjectCreate.lua
-- Formulario de cadastro de disciplina

local M = {}
local common = require("handlers.common")

local nameBuf = ""
local codeBuf = ""
local creditsBuf = ""
local teacherBuf = ""

function M.render()
	imgui.Text("Cadastrar Disciplina")
	imgui.Separator()
	imgui.Spacing()

	imgui.Text("Nome:")
	imgui.SetNextItemWidth(-1)
	imgui.PushID("sub_name")
	nameBuf = imgui.InputText("##name", nameBuf)
	imgui.PopID()

	imgui.Text("Codigo:")
	imgui.SetNextItemWidth(-1)
	imgui.PushID("sub_code")
	codeBuf = imgui.InputText("##code", codeBuf)
	imgui.PopID()

	imgui.Text("Creditos:")
	imgui.SetNextItemWidth(-1)
	imgui.PushID("sub_credits")
	creditsBuf = imgui.InputText("##credits", creditsBuf)
	imgui.PopID()

	imgui.Text("ID Professor:")
	imgui.SetNextItemWidth(-1)
	imgui.PushID("sub_teacher")
	teacherBuf = imgui.InputText("##teacher", teacherBuf)
	imgui.PopID()

	imgui.Spacing()

	if common.button("Cadastrar", common.COLORS.Green, 150, 35) then
		local dm = getDataManager()
		if dm then
			if nameBuf == "" or #nameBuf < 2 then
				common.errorPopup("Nome deve ter pelo menos 2 caracteres")
			else
				local credits = tonumber(creditsBuf) or 0
				local teacherId = tonumber(teacherBuf) or -1
				local ok = dm:createSubject(nameBuf, codeBuf, credits, teacherId)
				if ok then
					nameBuf = ""; codeBuf = ""; creditsBuf = ""; teacherBuf = ""
					common.errorPopup("Disciplina cadastrada com sucesso!")
				else
					common.errorPopup(dm:getLastError())
				end
			end
		end
	end

	common.errorPopup("")
end

return M
