-- SubjectList.lua — list all active subjects with inline edit and delete

local M = {}
local common = require("handlers.common")

local state = {
	editingSubjectId = -1,
	editName = "",
	editCode = "",
	editCredits = 0,
	editTeacherId = -1
}

local function resetEditState()
	state.editingSubjectId = -1
	state.editName = ""
	state.editCode = ""
	state.editCredits = 0
	state.editTeacherId = -1
end

function M.render()
	local dataManager = getDataManager()
	if not dataManager then
		imgui.Text("DataManager nao inicializado")
		return
	end

	imgui.Text("Lista de Disciplinas")
	imgui.Separator()
	imgui.Spacing()

	local subjectList = dataManager:listAllSubjects()

	if #subjectList == 0 then
		common.textColored("Nenhuma disciplina encontrada.", common.COLORS.Red)
		return
	end

	imgui.Text("Total: " .. tostring(#subjectList) .. " disciplinas")
	imgui.Separator()
	imgui.Spacing()

	for _, subject in ipairs(subjectList) do
		imgui.PushID("subject_" .. subject.id)

		local availableWidth = imgui.GetContentRegionAvail()
		imgui.TextWrapped("[" .. tostring(subject.id) .. "] " .. tostring(subject.name) .. " (" .. tostring(subject.code) .. ")")
		imgui.Text("  Creditos: " .. tostring(subject.credits) .. " | Professor ID: " .. tostring(subject.teacherId))

		imgui.SameLine(availableWidth - 75)
		if common.button("Editar", common.COLORS.Green, 45, 20) then
			state.editingSubjectId = subject.id
			state.editName = subject.name
			state.editCode = subject.code
			state.editCredits = subject.credits
			state.editTeacherId = subject.teacherId
		end

		imgui.SameLine(availableWidth - 30)
		if common.button("X", common.COLORS.Red, 25, 20) then
			dataManager:deleteSubject(subject.id)
		end

		if state.editingSubjectId == subject.id then
			imgui.Spacing()
			imgui.Indent(20)

			state.editName = imgui.InputText("Nome", state.editName)
			state.editCode = imgui.InputText("Codigo", state.editCode)
			state.editCredits = imgui.InputInt("Creditos", state.editCredits)
			state.editTeacherId = imgui.InputInt("Professor ID", state.editTeacherId)

			imgui.Spacing()
			if imgui.Button("Salvar") then
				local success = dataManager:updateSubject(
					subject.id,
					state.editName,
					state.editCode,
					state.editCredits,
					state.editTeacherId
				)
				if not success then
					common.errorPopup("Erro", dataManager:getLastError())
				end
				resetEditState()
			end
			imgui.SameLine()
			if imgui.Button("Cancelar") then
				resetEditState()
			end

			imgui.Unindent(20)
			imgui.Spacing()
		end

		imgui.PopID()
		imgui.Separator()
	end
end

return M
