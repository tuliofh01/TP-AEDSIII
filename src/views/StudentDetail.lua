-- StudentDetail.lua
-- Consulta estudante por nome

local M = {}
local common = require("src.views.common")

local searchBuf = ""
local resultText = ""

function M.render()
	local dm = getDataManager()
	if not dm then
		imgui.Text("DataManager nao inicializado")
		return
	end

	imgui.Text("Consultar Estudante por Nome")
	imgui.Separator()
	imgui.Spacing()

	imgui.Text("Nome:")
	searchBuf = imgui.InputText("##search", searchBuf)
	imgui.Spacing()
	imgui.Spacing()

	-- Botao buscar
	if common.button("Buscar", common.COLORS.Green, 150, 35) then
		if searchBuf == "" then
			resultText = "Digite um nome para buscar"
		else
			local rec = dm:searchByName(searchBuf)
			if rec then
				resultText = "ID: " .. tostring(rec.id) ..
					"\nNome: " .. tostring(rec.name) ..
					"\nNascimento: " .. tostring(rec.birthDate)
			else
				resultText = "Nao encontrado: " .. dm:getLastError()
			end
		end
	end

	imgui.Spacing()
	imgui.Separator()
	imgui.Spacing()

	-- Resultado
	if resultText ~= "" then
		if resultText:find("Nao encontrado") or resultText:find("Digite") then
			common.textColored(resultText, common.COLORS.Red)
		else
			common.textColored(resultText, common.COLORS.Green)
		end
	end
end

return M
