-- EnrollmentList.lua
-- Gerenciamento de matriculas (B+ Tree enrollments)

local M = {}
local common = require("common")

local mode = "enroll" -- "enroll", "byStudent", "bySubject", "list"
local studentIdBuf = ""
local subjectIdBuf = ""
local teacherIdBuf = ""
local semesterBuf = "2026-1"
local gradeBuf = ""
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
		imgui.Text("Aluno " .. tostring(e.studentId) .. " | Disciplina " ..
			tostring(e.subjectId) .. " | Prof " .. tostring(e.teacherId) ..
			" | Nota: " .. string.format("%.1f", e.grade) .. " | " .. tostring(e.semester))
		imgui.Separator()
	end
end

function M.render()
	local dm = getDataManager()
	if not dm then
		imgui.Text("DataManager nao inicializado")
		return
	end

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
		imgui.PushID("enr_stu")
		studentIdBuf = imgui.InputText("##stu", studentIdBuf)
		imgui.PopID()

		imgui.Text("ID Disciplina:")
		imgui.PushID("enr_sub")
		subjectIdBuf = imgui.InputText("##sub", subjectIdBuf)
		imgui.PopID()

		imgui.Text("ID Professor:")
		imgui.PushID("enr_tch")
		teacherIdBuf = imgui.InputText("##tch", teacherIdBuf)
		imgui.PopID()

		imgui.Text("Semestre:")
		imgui.PushID("enr_sem")
		semesterBuf = imgui.InputText("##sem", semesterBuf)
		imgui.PopID()

		imgui.Spacing()
		if common.button("Matricular", common.COLORS.Green, 150, 30) then
			local sid = tonumber(studentIdBuf)
			local bid = tonumber(subjectIdBuf)
			local tid = tonumber(teacherIdBuf)
			if sid and bid and tid then
				local ok = dm:enrollStudent(sid, bid, tid, semesterBuf)
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
		imgui.PushID("gr_stu")
		studentIdBuf2 = studentIdBuf
		local sid2 = imgui.InputText("##stu2", studentIdBuf)
		studentIdBuf = sid2
		imgui.PopID()

		imgui.Text("ID Disciplina:")
		imgui.PushID("gr_sub")
		local bid2 = imgui.InputText("##sub2", subjectIdBuf)
		subjectIdBuf = bid2
		imgui.PopID()

		imgui.Text("Nota:")
		imgui.PushID("gr_grade")
		gradeBuf = imgui.InputText("##grade", gradeBuf)
		imgui.PopID()

		imgui.Spacing()
		if common.button("Atualizar Nota", common.COLORS.Black, 150, 30) then
			local sid = tonumber(studentIdBuf)
			local bid = tonumber(subjectIdBuf)
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
		local sidInput = imgui.InputText("##stuSearch", studentIdBuf)
		if sidInput ~= studentIdBuf then
			studentIdBuf = sidInput
			local sid = tonumber(studentIdBuf)
			if sid then
				enrollments = dm:getEnrollmentsByStudent(sid)
			end
		end

		imgui.Spacing()
		imgui.Separator()
		imgui.Spacing()

		showEnrollmentsTable(enrollments, "Matriculas do Aluno " .. studentIdBuf)

	elseif mode == "bySubject" then
		imgui.Text("Matriculas por Disciplina")
		imgui.Spacing()

		imgui.Text("ID Disciplina:")
		local bidInput = imgui.InputText("##subSearch", subjectIdBuf)
		if bidInput ~= subjectIdBuf then
			subjectIdBuf = bidInput
			local bid = tonumber(subjectIdBuf)
			if bid then
				enrollments = dm:getEnrollmentsBySubject(bid)
			end
		end

		imgui.Spacing()
		imgui.Separator()
		imgui.Spacing()

		showEnrollmentsTable(enrollments, "Matriculas da Disciplina " .. subjectIdBuf)
	end

	common.errorPopup("")
end

return M
