-- StudentCreate.lua
-- Formulario de cadastro de estudante

local M = {}
local common = require("src.views.common")

local nameBuf = ""
local birthBuf = ""
local errorMsg = ""

function M.render()
	imgui.Text("Cadastrar Estudante")
	imgui.Separator()
	imgui.Spacing()

	-- Campo nome
	imgui.Text("Nome:")
	imgui.PushID("name_input")
	nameBuf = imgui.InputText("##name", nameBuf)
	imgui.PopID()
	imgui.Spacing()

	-- Campo data de nascimento
	imgui.Text("Data Nascimento (DDMMAAAA):")
	imgui.PushID("birth_input")
	birthBuf = imgui.InputText("##birth", birthBuf)
	imgui.PopID()
	imgui.Spacing()
	imgui.Spacing()

	-- Botao cadastrar
	if common.button("Cadastrar", common.COLORS.Green, 150, 35) then
		local dm = getDataManager()
		if dm then
			-- Valida nome
			if nameBuf == "" or #nameBuf < 2 then
				errorMsg = "Nome deve ter pelo menos 2 caracteres"
				imgui.OpenPopup("Erro")
			else
				-- Converte data para inteiro
				local birthDate = tonumber(birthBuf) or 0
				if birthDate == 0 and birthBuf ~= "" then
					errorMsg = "Data invalida. Use formato DDMMAAAA"
					imgui.OpenPopup("Erro")
				else
					local ok = dm:createStudent(nameBuf, 0, birthDate)
					if ok then
						nameBuf = ""
						birthBuf = ""
						errorMsg = "Estudante cadastrado com sucesso!"
						imgui.OpenPopup("Erro") -- Reusa popup para sucesso
					else
						errorMsg = dm:getLastError()
						imgui.OpenPopup("Erro")
					end
				end
			end
		end
	end

	-- Popup de erro/sucesso
	if imgui.BeginPopupModal("Erro", nil, {"NoResize"}) then
		if errorMsg:find("sucesso") then
			common.textColored(errorMsg, common.COLORS.Green)
		else
			common.textColored(errorMsg, common.COLORS.Red)
		end
		imgui.Spacing()
		if common.button("OK", common.COLORS.Black, 80, 0) then
			imgui.CloseCurrentPopup()
			errorMsg = ""
		end
		imgui.EndPopup()
	end
end

return M
