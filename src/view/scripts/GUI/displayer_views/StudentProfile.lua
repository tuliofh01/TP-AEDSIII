-- StudentProfile.lua
-- Visualizacao dos proprios dados do aluno (somente leitura)

local M = {}

function M.render(studentId)
	local dm = getDataManager()
	if not dm then
		imgui.Text("DataManager nao inicializado")
		return
	end

	local stu = dm:readStudent(studentId)
	if not stu then
		imgui.Text("Aluno nao encontrado")
		return
	end

	local aw = imgui.GetContentRegionAvail()
	local cx = aw / 2 - 100
	if cx < 10 then cx = 10 end
	imgui.SetCursorPosX(cx)

	imgui.Text("Meus Dados")
	imgui.SetCursorPosX(cx)
	imgui.Separator()
	imgui.Spacing()

	imgui.SetCursorPosX(cx)
	imgui.TextWrapped("Nome: " .. tostring(stu.name))
	imgui.SetCursorPosX(cx)
	imgui.TextWrapped("Email: " .. tostring(stu.email))
	imgui.SetCursorPosX(cx)
	imgui.Text("CPF: " .. tostring(stu.cpf))
	imgui.SetCursorPosX(cx)
	imgui.TextWrapped("Curso: " .. tostring(stu.course))
	imgui.SetCursorPosX(cx)
	imgui.Text("Ano de Ingresso: " .. tostring(stu.enrollmentYear))

	imgui.Spacing()
	imgui.SetCursorPosX(cx)
	imgui.Separator()
	imgui.SetCursorPosX(cx)
	imgui.Text("ID do Aluno: " .. tostring(stu.id))
end

return M
