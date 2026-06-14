-- populate_samples.lua
-- Cria dados de amostra: professor, aluno, disciplina, matriculas e notas

local M = {}

function M.populate(dm)
	-- Cria professor de amostra: CPF=00000000000, senha=1234
	local teacherOk = dm:createTeacher("Professor Fulano", "prof@escola.com",
		"00000000000", 1234, "DCC", "Banco de Dados", 20150301)
	if teacherOk then
		print("Professor de amostra criado (CPF: 00000000000, senha: 1234)")
	else
		print("Erro ao criar professor: " .. dm:getLastError())
	end

	-- Cria aluno de amostra: CPF=11111111111, senha=1234
	local studentOk = dm:createStudent("Aluno Beltrano", "aluno@escola.com",
		"11111111111", 1234, 20000515, "Ciencia da Computacao", 2026)
	if studentOk then
		print("Aluno de amostra criado (CPF: 11111111111, senha: 1234)")
	else
		print("Erro ao criar aluno: " .. dm:getLastError())
	end

	-- Cria disciplinas de exemplo
	local bdOk = dm:createSubject("Banco de Dados", "BD001", 60, 1)
	if bdOk then
		print("Disciplina 'Banco de Dados' criada")
	end

	local lpOk = dm:createSubject("Logica de Programacao", "LP001", 60, 1)
	if lpOk then
		print("Disciplina 'Logica de Programacao' criada")
	end

	-- Cria matriculas com notas
	if studentOk and bdOk then
		local enr1 = dm:enrollStudent(1, 1, 1, "2026-1")
		if enr1 then
			dm:updateGrade(1, 1, 85)
			print("Aluno matriculado em 'Banco de Dados' (2026-1, nota: 85)")
		end
	end

	if studentOk and lpOk then
		local enr2 = dm:enrollStudent(1, 2, 1, "2026-1")
		if enr2 then
			dm:updateGrade(1, 2, 42)
			print("Aluno matriculado em 'Logica de Programacao' (2026-1, nota: 42)")
		end
	end

	-- Cria segundo aluno
	local aluno2 = dm:createStudent("Ciclana Silva", "ciclana@escola.com",
		"22222222222", 1234, 19980810, "Engenharia de Software", 2025)
	if aluno2 then
		print("Segundo aluno criado (ID 2)")
		if bdOk then
			dm:enrollStudent(2, 1, 1, "2026-1")
			print("Aluno matriculado em 'Banco de Dados' (2026-1, sem nota)")
		end
	end
end

return M
