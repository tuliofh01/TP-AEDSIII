-- EnrollmentList.lua
-- Gerenciamento de matriculas — professores: full; alunos: somente leitura

local M = {}
local common = require("handlers.common")

local mode = "enroll"

-- Each UI section has its own independent buffers to prevent cross-talk
local enrollStuBuf = ""
local enrollSubBuf = ""
local enrollTchBuf = ""
local enrollSemBuf = "2026-1"
local gradeStuBuf = ""
local gradeSubBuf = ""
local gradeBuf = ""
local byStuBuf = ""
local bySubBuf = ""
local queryResult = ""
local enrollments = {}
local showGradePopup = false
local gradeStudentId = nil
local gradeSubjectId = nil

local function showEnrollmentsTable(list, label)
	if #list == 0 then
		common.textColored("Nenhuma matricula encontrada.", common.COLORS.Red)
		return
	end
	imgui.Text(label .. " (" .. tostring(#list) .. ")")
	imgui.Separator()
	for _, e in ipairs(list) do
		local status = ""
		if e.grade > 0 then
			status = (e.approved and "APROVADO") or "REPROVADO"
		end
		imgui.Text("Disc " .. tostring(e.subjectId) .. " | Prof " .. tostring(e.teacherId) ..
			" | Nota: " .. tostring(e.grade) .. "/100 " .. status ..
			" | " .. tostring(e.semester))
		imgui.Separator()
	end
end

function M.render(studentId, readOnly)
	local dm = getDataManager()
	if not dm then
		imgui.Text("DataManager nao inicializado")
		return
	end

	if readOnly then
		-- Student mode: auto-load own enrollments, read-only
		imgui.Text("Minhas Matriculas")
		imgui.Separator()
		imgui.Spacing()

		local list = dm:getEnrollmentsByStudent(studentId)
		showEnrollmentsTable(list, "Matriculas do Aluno " .. tostring(studentId))
		return
	end

	-- Teacher mode: full access
	imgui.Text("Gerenciar Matriculas")
	imgui.Separator()
	imgui.Spacing()

	-- Mode tabs
	if common.button("Matricular", common.COLORS.Green, 120, 25) then
		mode = "enroll"
	end
	imgui.SameLine()
	if common.button("Por Aluno", common.COLORS.Black, 120, 25) then
		mode = "byStudent"
	end
	imgui.SameLine()
	if common.button("Por Disciplina", common.COLORS.Black, 120, 25) then
		mode = "bySubject"
	end

	imgui.Spacing()
	imgui.Separator()
	imgui.Spacing()

	if mode == "enroll" then
		imgui.Text("Matricular Aluno em Disciplina")
		imgui.Spacing()

		imgui.Text("ID Aluno:")
		imgui.SetNextItemWidth(-1)
		imgui.PushID("enr_stu")
		enrollStuBuf = imgui.InputText("##stu", enrollStuBuf)
		imgui.PopID()

		imgui.Text("ID Disciplina:")
		imgui.SetNextItemWidth(-1)
		imgui.PushID("enr_sub")
		enrollSubBuf = imgui.InputText("##sub", enrollSubBuf)
		imgui.PopID()

		imgui.Text("ID Professor:")
		imgui.SetNextItemWidth(-1)
		imgui.PushID("enr_tch")
		enrollTchBuf = imgui.InputText("##tch", enrollTchBuf)
		imgui.PopID()

		imgui.Text("Semestre:")
		imgui.SetNextItemWidth(-1)
		imgui.PushID("enr_sem")
		enrollSemBuf = imgui.InputText("##sem", enrollSemBuf)
		imgui.PopID()

		imgui.Spacing()
		if common.button("Matricular", common.COLORS.Green, 150, 30) then
			local sid = tonumber(enrollStuBuf)
			local bid = tonumber(enrollSubBuf)
			local tid = tonumber(enrollTchBuf)
			if sid and bid and tid then
				local ok = dm:enrollStudent(sid, bid, tid, enrollSemBuf)
				if ok then
					common.errorPopup("Matricula realizada com sucesso!")
				else
					common.errorPopup(dm:getLastError())
				end
			else
				common.errorPopup("IDs invalidos")
			end
		end

		imgui.Spacing()
		imgui.Separator()
		imgui.Spacing()

		-- Update grade section
		imgui.Text("Atualizar Nota")
		imgui.Spacing()

		imgui.Text("ID Aluno:")
		imgui.SetNextItemWidth(-1)
		imgui.PushID("gr_stu")
		gradeStuBuf = imgui.InputText("##stu2", gradeStuBuf)
		imgui.PopID()

		imgui.Text("ID Disciplina:")
		imgui.SetNextItemWidth(-1)
		imgui.PushID("gr_sub")
		gradeSubBuf = imgui.InputText("##sub2", gradeSubBuf)
		imgui.PopID()

		imgui.Text("Nota (0-100):")
		imgui.SetNextItemWidth(-1)
		imgui.PushID("gr_grade")
		gradeBuf = imgui.InputText("##grade", gradeBuf)
		imgui.PopID()

		imgui.Spacing()
		if common.button("Atualizar Nota", common.COLORS.Black, 150, 30) then
			local sid = tonumber(gradeStuBuf)
			local bid = tonumber(gradeSubBuf)
			local g = tonumber(gradeBuf)
			if sid and bid and g then
				local ok = dm:updateGrade(sid, bid, g)
				if ok then
					common.errorPopup("Nota atualizada com sucesso!")
				else
					common.errorPopup(dm:getLastError())
				end
			else
				common.errorPopup("Valores invalidos")
			end
		end

	elseif mode == "byStudent" then
		imgui.Text("Matriculas por Aluno")
		imgui.Spacing()

		imgui.Text("ID Aluno:")
		imgui.SetNextItemWidth(-1)
		local sidInput = imgui.InputText("##stuSearch", byStuBuf)
		if sidInput ~= byStuBuf then
			byStuBuf = sidInput
			local sid = tonumber(byStuBuf)
			if sid then
				enrollments = dm:getEnrollmentsByStudent(sid)
			end
		end

		imgui.Spacing()
		imgui.Separator()
		imgui.Spacing()

		showEnrollmentsTable(enrollments, "Matriculas do Aluno " .. byStuBuf)

	elseif mode == "bySubject" then
		imgui.Text("Matriculas por Disciplina")
		imgui.Spacing()

		imgui.Text("ID Disciplina:")
		imgui.SetNextItemWidth(-1)
		local bidInput = imgui.InputText("##subSearch", bySubBuf)
		if bidInput ~= bySubBuf then
			bySubBuf = bidInput
			local bid = tonumber(bySubBuf)
			if bid then
				enrollments = dm:getEnrollmentsBySubject(bid)
			end
		end

		imgui.Spacing()
		imgui.Separator()
		imgui.Spacing()

		showEnrollmentsTable(enrollments, "Matriculas da Disciplina " .. bySubBuf)
	end

	common.errorPopup("")
end

return M
