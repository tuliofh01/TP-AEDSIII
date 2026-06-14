-- TeacherList.lua — list all active teachers with inline edit and delete

local M = {}
local common = require("handlers.common")

local state = {
	editingTeacherId = -1,
	editName = "",
	editEmail = "",
	editCpf = "",
	editPassword = 1234,
	editDepartment = "",
	editSpecialization = "",
	editHireDate = 0
}

local function resetEditState()
	state.editingTeacherId = -1
	state.editName = ""
	state.editEmail = ""
	state.editCpf = ""
	state.editPassword = 1234
	state.editDepartment = ""
	state.editSpecialization = ""
	state.editHireDate = 0
end

function M.render()
	local dataManager = getDataManager()
	if not dataManager then
		imgui.Text("DataManager nao inicializado")
		return
	end

	imgui.Text("Lista de Professores")
	imgui.Separator()
	imgui.Spacing()

	local teacherList = dataManager:listAllTeachers()

	if #teacherList == 0 then
		common.textColored("Nenhum professor encontrado.", common.COLORS.Red)
		return
	end

	imgui.Text("Total: " .. tostring(#teacherList) .. " professores")
	imgui.Separator()
	imgui.Spacing()

	for _, teacher in ipairs(teacherList) do
		imgui.PushID("teacher_" .. teacher.id)

		local availableWidth = imgui.GetContentRegionAvail()
		imgui.TextWrapped("[" .. tostring(teacher.id) .. "] " .. tostring(teacher.name))
		imgui.Text("  Depto: " .. tostring(teacher.department) .. " | " .. tostring(teacher.specialization))

		imgui.SameLine(availableWidth - 75)
		if common.button("Editar", common.COLORS.Green, 45, 20) then
			state.editingTeacherId = teacher.id
			state.editName = teacher.name
			state.editEmail = teacher.email
			state.editCpf = teacher.cpf
			state.editPassword = teacher.password
			state.editDepartment = teacher.department
			state.editSpecialization = teacher.specialization
			state.editHireDate = teacher.hireDate
		end

		imgui.SameLine(availableWidth - 30)
		if common.button("X", common.COLORS.Red, 25, 20) then
			dataManager:deleteTeacher(teacher.id)
		end

		if state.editingTeacherId == teacher.id then
			imgui.Spacing()
			imgui.Indent(20)

			state.editName = imgui.InputText("Nome", state.editName)
			state.editEmail = imgui.InputText("Email", state.editEmail)
			state.editCpf = imgui.InputText("CPF", state.editCpf)
			state.editPassword = imgui.InputInt("Senha", state.editPassword)
			state.editDepartment = imgui.InputText("Departamento", state.editDepartment)
			state.editSpecialization = imgui.InputText("Especializacao", state.editSpecialization)
			state.editHireDate = imgui.InputInt("Data Contrato (AAAAMMDD)", state.editHireDate)

			imgui.Spacing()
			if imgui.Button("Salvar") then
				local success = dataManager:updateTeacher(
					teacher.id,
					state.editName,
					state.editEmail,
					state.editCpf,
					state.editPassword,
					state.editDepartment,
					state.editSpecialization,
					state.editHireDate
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
