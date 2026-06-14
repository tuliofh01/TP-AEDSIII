-- StudentList.scripts
-- Lista todos estudantes ativos

local M = {}
local common = require("handlers.common")

function M.render()
	local dm = getDataManager()
	if not dm then
		imgui.Text("DataManager nao inicializado")
		return
	end

	imgui.Text("Lista de Estudantes")
	imgui.Separator()
	imgui.Spacing()

	-- Busca lista
	local students = dm:listAllStudents()

	if #students == 0 then
		common.textColored("Nenhum estudante encontrado.", common.COLORS.Red)
		return
	end

	imgui.Text("Total: " .. tostring(#students) .. " estudantes")
	imgui.Separator()
	imgui.Spacing()

	-- Lista vertical simples
	for _, s in ipairs(students) do
		imgui.PushID("student_" .. s.id)

		-- Linha: ID + Nome
		imgui.Text("[" .. tostring(s.id) .. "] " .. tostring(s.name))

		-- Botao deletar alinhado a direita
		local aw = imgui.GetContentRegionAvail()
		imgui.SameLine(aw - 40)
		if common.button("X", common.COLORS.Red, 30, 20) then
			dm:deleteStudent(s.id)
		end

		imgui.PopID()
		imgui.Separator()
	end
end

return M
