-- StudentList.lua — list all active students with inline edit and delete

local M = {}
local common = require("handlers.common")

local state = {
	editingStudentId = -1,
	editName = "",
	editEmail = "",
	editCpf = "",
	editPassword = 1234,
	editBirthDate = 0,
	editCourse = "",
	editEnrollmentYear = 2026
}

local function resetEditState()
	state.editingStudentId = -1
	state.editName = ""
	state.editEmail = ""
	state.editCpf = ""
	state.editPassword = 1234
	state.editBirthDate = 0
	state.editCourse = ""
	state.editEnrollmentYear = 2026
end

function M.render()
	local dataManager = getDataManager()
	if not dataManager then
		imgui.Text("DataManager nao inicializado")
		return
	end

	imgui.Text("Lista de Estudantes")
	imgui.Separator()
	imgui.Spacing()

	local studentList = dataManager:listAllStudents()

	if #studentList == 0 then
		common.textColored("Nenhum estudante encontrado.", common.COLORS.Red)
		return
	end

	imgui.Text("Total: " .. tostring(#studentList) .. " estudantes")
	imgui.Separator()
	imgui.Spacing()

	for _, student in ipairs(studentList) do
		imgui.PushID("student_" .. student.id)

		local availableWidth = imgui.GetContentRegionAvail()
		imgui.TextWrapped("[" .. tostring(student.id) .. "] " .. tostring(student.name) .. " (" .. tostring(student.email) .. ")")

		imgui.SameLine(availableWidth - 75)
		if common.button("Editar", common.COLORS.Green, 45, 20) then
			state.editingStudentId = student.id
			state.editName = student.name
			state.editEmail = student.email
			state.editCpf = student.cpf
			state.editPassword = student.password
			state.editBirthDate = student.birthDate
			state.editCourse = student.course
			state.editEnrollmentYear = student.enrollmentYear
		end

		imgui.SameLine(availableWidth - 30)
		if common.button("X", common.COLORS.Red, 25, 20) then
			dataManager:deleteStudent(student.id)
		end

		if state.editingStudentId == student.id then
			imgui.Spacing()
			imgui.Indent(20)

			state.editName = imgui.InputText("Nome", state.editName)
			state.editEmail = imgui.InputText("Email", state.editEmail)
			state.editCpf = imgui.InputText("CPF", state.editCpf)
			state.editPassword = imgui.InputInt("Senha", state.editPassword)
			state.editBirthDate = imgui.InputInt("Data Nasc (AAAAMMDD)", state.editBirthDate)
			state.editCourse = imgui.InputText("Curso", state.editCourse)
			state.editEnrollmentYear = imgui.InputInt("Ano Ingresso", state.editEnrollmentYear)

			imgui.Spacing()
			if imgui.Button("Salvar") then
				local success = dataManager:updateStudent(
					student.id,
					state.editName,
					state.editEmail,
					state.editCpf,
					state.editPassword,
					state.editBirthDate,
					state.editCourse,
					state.editEnrollmentYear
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
