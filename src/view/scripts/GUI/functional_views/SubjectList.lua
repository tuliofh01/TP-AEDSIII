-- SubjectList.lua
-- Lista todas disciplinas ativas

local M = {}
local common = require("handlers.common")

function M.render()
	local dm = getDataManager()
	if not dm then
		imgui.Text("DataManager nao inicializado")
		return
	end

	imgui.Text("Lista de Disciplinas")
	imgui.Separator()
	imgui.Spacing()

	local subjects = dm:listAllSubjects()

	if #subjects == 0 then
		common.textColored("Nenhuma disciplina encontrada.", common.COLORS.Red)
		return
	end

	imgui.Text("Total: " .. tostring(#subjects) .. " disciplinas")
	imgui.Separator()
	imgui.Spacing()

	for _, s in ipairs(subjects) do
		imgui.PushID("subject_" .. s.id)
		local aw = imgui.GetContentRegionAvail()
		imgui.TextWrapped("[" .. tostring(s.id) .. "] " .. tostring(s.name) .. " (" .. tostring(s.code) .. ")")
		imgui.Text("  Creditos: " .. tostring(s.credits) .. " | Professor ID: " .. tostring(s.teacherId))
		imgui.SameLine(aw - 40)
		if common.button("X", common.COLORS.Red, 30, 20) then
			dm:deleteSubject(s.id)
		end

		imgui.PopID()
		imgui.Separator()
	end
end

return M
