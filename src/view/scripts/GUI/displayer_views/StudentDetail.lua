-- StudentDetail.lua
-- Consulta estudante por ID

local M = {}
local common = require("handlers.common")

local searchBuf = ""
local resultText = ""

function M.render()
	local dm = getDataManager()
	if not dm then
		imgui.Text("DataManager nao inicializado")
		return
	end

	imgui.Text("Consultar Estudante por ID")
	imgui.Separator()
	imgui.Spacing()

	imgui.Text("ID do Estudante:")
	imgui.SetNextItemWidth(-1)
	imgui.PushID("search_id")
	local newBuf = imgui.InputText("##search", searchBuf)
	if newBuf ~= searchBuf then
		searchBuf = newBuf
		resultText = ""
	end
	imgui.PopID()
	imgui.Spacing()

	if common.button("Buscar", common.COLORS.Green, 150, 35) then
		if searchBuf == "" then
			resultText = "Digite um ID para buscar"
		else
			local id = tonumber(searchBuf)
			if id then
				local rec = dm:readStudent(id)
				if rec then
					resultText = "ID: " .. tostring(rec.id) ..
						"\nNome: " .. tostring(rec.name) ..
						"\nEmail: " .. tostring(rec.email) ..
						"\nCPF: " .. tostring(rec.cpf) ..
						"\nNascimento: " .. tostring(rec.birthDate) ..
						"\nCurso: " .. tostring(rec.course) ..
						"\nAno Ingresso: " .. tostring(rec.enrollmentYear)
				else
					resultText = "Nao encontrado: " .. dm:getLastError()
				end
			else
				resultText = "ID invalido"
			end
		end
	end

	imgui.Spacing()
	imgui.Separator()
	imgui.Spacing()

	if resultText ~= "" then
		if resultText:find("Nao encontrado") or resultText:find("invalido") or resultText:find("Digite") then
			common.textColored(resultText, common.COLORS.Red)
		else
			common.textColored(resultText, common.COLORS.Green)
		end
	end
end

return M
