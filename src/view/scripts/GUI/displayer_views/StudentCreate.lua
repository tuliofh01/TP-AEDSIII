-- StudentCreate.lua
-- Formulario de cadastro de estudante

local M = {}
local common = require("handlers.common")

local nameBuf = ""
local emailBuf = ""
local cpfBuf = ""
local passBuf = ""
local birthBuf = ""
local courseBuf = ""
local yearBuf = ""

function M.render()
	imgui.Text("Cadastrar Estudante")
	imgui.Separator()
	imgui.Spacing()

	imgui.Text("Nome:")
	imgui.SetNextItemWidth(-1)
	imgui.PushID("stu_name")
	nameBuf = imgui.InputText("##name", nameBuf)
	imgui.PopID()

	imgui.Text("Email:")
	imgui.SetNextItemWidth(-1)
	imgui.PushID("stu_email")
	emailBuf = imgui.InputText("##email", emailBuf)
	imgui.PopID()

	imgui.Text("CPF:")
	imgui.SetNextItemWidth(-1)
	imgui.PushID("stu_cpf")
	cpfBuf = imgui.InputText("##cpf", cpfBuf)
	imgui.PopID()

	imgui.Text("Senha numerica:")
	imgui.SetNextItemWidth(-1)
	imgui.PushID("stu_pass")
	passBuf = imgui.InputText("##pass", passBuf)
	imgui.PopID()

	imgui.Text("Nascimento (DDMMAAAA):")
	imgui.SetNextItemWidth(-1)
	imgui.PushID("stu_birth")
	birthBuf = imgui.InputText("##birth", birthBuf)
	imgui.PopID()

	imgui.Text("Curso:")
	imgui.SetNextItemWidth(-1)
	imgui.PushID("stu_course")
	courseBuf = imgui.InputText("##course", courseBuf)
	imgui.PopID()

	imgui.Text("Ano Ingresso:")
	imgui.SetNextItemWidth(-1)
	imgui.PushID("stu_year")
	yearBuf = imgui.InputText("##year", yearBuf)
	imgui.PopID()

	imgui.Spacing()

	if common.button("Cadastrar", common.COLORS.Green, 150, 35) then
		local dm = getDataManager()
		if dm then
			if nameBuf == "" or #nameBuf < 2 then
				common.errorPopup("Nome deve ter pelo menos 2 caracteres")
			else
				local birthDate = tonumber(birthBuf) or 0
				local year = tonumber(yearBuf) or 0
				local pass = tonumber(passBuf) or 0
				local ok = dm:createStudent(nameBuf, emailBuf, cpfBuf, pass, birthDate, courseBuf, year)
				if ok then
					nameBuf = ""; emailBuf = ""; cpfBuf = ""; passBuf = ""
					birthBuf = ""; courseBuf = ""; yearBuf = ""
					common.errorPopup("Estudante cadastrado com sucesso!")
				else
					common.errorPopup(dm:getLastError())
				end
			end
		end
	end

	common.errorPopup("")
end

return M
