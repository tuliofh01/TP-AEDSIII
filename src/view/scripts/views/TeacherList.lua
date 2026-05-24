-- TeacherList.lua
-- Lista todos professores ativos

local M = {}
local common = require("common")

function M.render()
	local dm = getDataManager()
	if not dm then
		imgui.Text("DataManager nao inicializado")
		return
	end

	imgui.Text("Lista de Professores")
	imgui.Separator()
	imgui.Spacing()

	local teachers = dm:listAllTeachers()

	if #teachers == 0 then
		common.textColored("Nenhum professor encontrado.", common.COLORS.Red)
		return
	end

	imgui.Text("Total: " .. tostring(#teachers) .. " professores")
	imgui.Separator()
	imgui.Spacing()

	for _, t in ipairs(teachers) do
		imgui.PushID("teacher_" .. t.id)
		imgui.Text("[" .. tostring(t.id) .. "] " .. tostring(t.name))
		imgui.Text("  Depto: " .. tostring(t.department) .. " | " .. tostring(t.specialization))

		imgui.SameLine(500)
		if common.button("X", common.COLORS.Red, 30, 20) then
			dm:deleteTeacher(t.id)
		end

		imgui.PopID()
		imgui.Separator()
	end
end

return M
