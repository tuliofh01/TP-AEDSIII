-- TeacherCreate.lua
-- Formulario de cadastro de professor

local M = {}
local common = require("common")

local nameBuf = ""
local emailBuf = ""
local cpfBuf = ""
local deptBuf = ""
local specBuf = ""
local hireBuf = ""

function M.render()
	imgui.Text("Cadastrar Professor")
	imgui.Separator()
	imgui.Spacing()

	imgui.Text("Nome:")
	imgui.PushID("tch_name")
	nameBuf = imgui.InputText("##name", nameBuf)
	imgui.PopID()

	imgui.Text("Email:")
	imgui.PushID("tch_email")
	emailBuf = imgui.InputText("##email", emailBuf)
	imgui.PopID()

	imgui.Text("CPF:")
	imgui.PushID("tch_cpf")
	cpfBuf = imgui.InputText("##cpf", cpfBuf)
	imgui.PopID()

	imgui.Text("Departamento:")
	imgui.PushID("tch_dept")
	deptBuf = imgui.InputText("##dept", deptBuf)
	imgui.PopID()

	imgui.Text("Especializacao:")
	imgui.PushID("tch_spec")
	specBuf = imgui.InputText("##spec", specBuf)
	imgui.PopID()

	imgui.Text("Contratacao (DDMMAAAA):")
	imgui.PushID("tch_hire")
	hireBuf = imgui.InputText("##hire", hireBuf)
	imgui.PopID()

	imgui.Spacing()

	if common.button("Cadastrar", common.COLORS.Green, 150, 35) then
		local dm = getDataManager()
		if dm then
			if nameBuf == "" or #nameBuf < 2 then
				common.errorPopup("Nome deve ter pelo menos 2 caracteres")
			else
				local hireDate = tonumber(hireBuf) or 0
				local ok = dm:createTeacher(nameBuf, emailBuf, cpfBuf, deptBuf, specBuf, hireDate)
				if ok then
					nameBuf = ""; emailBuf = ""; cpfBuf = ""
					deptBuf = ""; specBuf = ""; hireBuf = ""
					common.errorPopup("Professor cadastrado com sucesso!")
				else
					common.errorPopup(dm:getLastError())
				end
			end
		end
	end

	common.errorPopup("")
end

return M
